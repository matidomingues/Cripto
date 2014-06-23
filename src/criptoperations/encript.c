#include "encript.h"

void init_crypto() {
	init_inverses();
}

void decode(byte * bitmap_data, int pos, unsigned int ** A, unsigned int * B, int k) {
	unsigned int * X = mult_vec(A, B, k);
	int i = 0;
	for (i = 0; i < k; i++) {
		*(bitmap_data + pos + i) = (byte)X[i];
	}
	free(X);
}

int calculate_parity_bit(byte *hash, int k){
	int i;
	byte resp = 0x00;
	int bit = 0, aux = 0;
	byte *md = (byte *)malloc(MY_MD5_DIGEST_LENGTH * sizeof(byte));
	MY_MD5(hash, k, md);
	for (i = 0; i< MY_MD5_DIGEST_LENGTH; i++){
		resp ^= md[i];
	}
	for (i = 0; i<sizeof(byte); i++){
		aux = resp & 1;
		resp = resp >> 1;
		bit ^= aux;
	}
	return bit;
}

boolean check_parity_bit(byte *shadows_bitmap_data, int * b_coeff, int k) {
	byte * hashables = (byte*)malloc(k * sizeof(byte));
	byte mask = 0x01 << (b_coeff[k-1] - 1);
	int i = 0;
	for (i = 0; i < k; i++) {
		hashables[i] = (i == k-1)? shadows_bitmap_data[i] ^ mask : shadows_bitmap_data[i];
	}
	byte p = calculate_parity_bit(hashables, k);
	return p == ((shadows_bitmap_data[k-1] & mask) >> b_coeff[k-1]);
}

void getBitsTweaked(int numb, byte *bitmapData, byte *alist, int* b_coeffs, int k){
	byte *calculateHash = (byte *)malloc(k);
	int i, new, b_size;

	for(i=k-1; i >= 0; i--){
		b_size = (i == k-1)? b_coeffs[i] -1:
					b_coeffs[i];
		bitmapData[i] = alist[i] << b_coeffs[i];
		new = numb & (1 << b_size) -1;

		calculateHash[i] = (i == k-1)? bitmapData[i] + (new << 1) :
							bitmapData[i] + new;

		bitmapData[i] += new;
		numb = numb >> b_size;
	}
	int bit = calculate_parity_bit(calculateHash, k);
	bit = bit << (b_coeffs[k-1]-1);
	*(bitmapData+(k-1)) ^= bit;
}

void printMemory(unsigned char *data){
	int i;

	for(i=0; i<3;i++){
		printf("%2x ",*(data+i));
	}
	printf("\n");
}

int tweaker(byte ** bitmapData, int k, int loc, int *b){
	byte **data = (byte **)malloc(k * sizeof(byte*));
	byte tweaked = 0;
	int i, end = 0, tweak = 0, pos = 0 , lastop = 0, lastpos = 0, lastweak = 0, retry = 0;
	do{
		for(i = 0; i < k; i++){
			data[i] = calculateBits((bitmapData[i]+loc), b, 3);
		}
		if(calculateLinealIndependency(data[0], data[1], data[2]) == 0){
			retry = 1;
			if(tweaked != 0){
				*(bitmapData[lastweak]+loc) = tweaked;
			}
			lastweak = tweak;
			lastpos = pos;
			tweaked = tweakBits(bitmapData[tweak++]+loc, pos, b);
			if(tweak == k){
				pos++;
				tweak = 0;
				if(pos == k){
					lastop = 0;
					pos = 0;
				}
			}
		}else{
			end = 1;
		}
		for(i=0; i<k; i++){
			free(data[i]);
		}
	}while(!end);
	return retry;
}

int sumValue(int* map, int loc, int k, int images){
	int size = images-((k-1)-loc);
	if(loc == 0){
		if(size == map[loc]){
			return 0;
		}else{
			map[loc] += 1;
			return map[loc]+1;
		}
	}
	if(map[loc] == size){
		int val = sumValue(map, loc-1, k, images);
		if(val == 0){
			return 0;
		}
		map[loc] = val;
		return val + 1;
	}
	map[loc] += 1;
	return map[loc]+1;

}

void check_independence(byte **shadows_bitmap_data, int images, int* b, int w, int k){
	int * loc = (int *)malloc(k * sizeof(int));
	int i, end = 0, restart = 0;
	unsigned char *elems[k];
	while(!end){
		for(i=0; i<k; i++){
			elems[i] = shadows_bitmap_data[loc[i]];
		}
		if((restart = tweaker(elems,k,w,b)) || !sumValue(loc,k-1,k,images-1)){
			end = 1;
		}
	}
	if(restart){
		check_independence(shadows_bitmap_data,images,b,w,k);
	}
}

void encrypt_images(byte * secret_bitmap_data, byte ** shadows_bitmap_data, int shadows_qty, int image_size, int k){
	int w, i, num;
	unsigned char* data;
	int* b = calculate_b_coeffs(k);
	for(w = 0 ; w< image_size; w += k) {
		check_independence(shadows_bitmap_data, shadows_qty, b, w, k);
		for(i=0; i<shadows_qty; i++) {
			data = calculateBits(shadows_bitmap_data[i] + w, b, k);
			num = calculateB(secret_bitmap_data + w, data, k);
			getBitsTweaked(num, shadows_bitmap_data[i] + w, data, b,k);
			free(data);
		}
	}
}

void decrypt_images(bitmap * secret_bitmap, bitmap ** shadows_bitmaps, int image_size, int k) {
	unsigned int i, num, w;
	int *b_sizes = calculate_b_coeffs(k);
	for (w = 0; w < image_size; w += k) {
		unsigned int **A = (byte **)malloc(k * sizeof(unsigned int **));
		unsigned int **inverse = NULL;
		unsigned int *B;
		for (i = 0; i < k; i++) {
			if (!check_parity_bit(shadows_bitmaps[i]->data + w, b_sizes, k)) {
				printf("Parity bit doesn't match. File might be corrupted. Terminating.\n");
				int j = 0;
				for (j = 0; j < i; j++) free(A[i]);
				free(A);
				free(b_sizes);
				return;
			}
			A[i] = get_A(shadows_bitmaps[i]->data + w, b_sizes, k);
		}
		inverse = inverse_matrix(A, k);
		if (inverse == NULL) {
			printf("Some error happened. Probably matrix wasn't invertible.\n");
			free(b_sizes);
			free_matrix(A, k);
			return;
		}
		B = get_B(shadows_bitmaps[0]->data, b_sizes, w, k);
		decode(secret_bitmap->data, w, inverse, B, k);
		free(B);
		free_matrix(inverse, k);
		free_matrix(A, k);
	}
	free(b_sizes);
}

