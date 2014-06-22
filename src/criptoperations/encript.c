#include "encript.h"

unsigned int modular_inverse[P];

unsigned int modinv(unsigned int u, unsigned int v)
{
    unsigned int inv, u1, u3, v1, v3, t1, t3, q;
    int iter;
    /* Step X1. Initialize */
    u1 = 1;
    u3 = u;
    v1 = 0;
    v3 = v;
    /* Remember odd/even iterations */
    iter = 1;
    /* Step X2. Loop while v3 != 0 */
    while (v3 != 0)
    {
        /* Step X3. Divide and "Subtract" */
        q = u3 / v3;
        t3 = u3 % v3;
        t1 = u1 + q * v1;
        /* Swap */
        u1 = v1; v1 = t1; u3 = v3; v3 = t3;
        iter = -iter;
    }
    /* Make sure u3 = gcd(u,v) == 1 */
    if (u3 != 1)
        return 0;   /* Error: No inverse exists */
    /* Ensure a positive result */
    if (iter < 0)
        inv = v - u1;
    else
        inv = u1;
    return inv;
}

void init_inverses() {
	unsigned int i = 0;
	for (i = 0; i < P; i++) {
		modular_inverse[i] = modinv(i, P);
	}
}

unsigned int ** make_eye(int k) {
	unsigned int ** eye_mat = (unsigned int **)malloc(k * sizeof(unsigned int *));
	int i = 0;
	for (i = 0; i < k; i++) {
		eye_mat[i] = (unsigned int *)calloc(k, sizeof(unsigned int));
		eye_mat[i][i] = (unsigned int)1;
	}
	return eye_mat;
}

boolean is_eye(unsigned int ** matrix, int k) {
	boolean is_eye = true;
	int i = 0, j = 0;
	for (i = 0; i < k && is_eye; i++) {
		for (j = 0; j < k && is_eye; i++) {
			if ((j == i && matrix[i][j] != 1) ||
					(j != i && matrix[i][j] != 0)) {
				is_eye = false;
				break;
			}
		}
	}
	return is_eye;
}

void mult_k(unsigned int * vec, unsigned int k, int size) {
	int i = 0;
	for (i = 0; i < size; i++) {
		vec[i] *= k;
		vec[i] %= P;
	}
}

void mult_vec(unsigned int ** matrix, unsigned int * vec, int size) {
	unsigned int * result = (unsigned int*)calloc(size, sizeof(unsigned int));
	int i = 0, j = 0;
	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			result[i] += matrix[i][j] * vec[j];
		}
		result[i] %= result[i] % P;
	}
}

void sub(unsigned int * vec1, unsigned int * vec2, unsigned int k, int size) {
	int i = 0;
	for (i = 0; i < size; i++) {
		vec1[i] -= ((vec2[i] * k) % P);
		vec1[i] %= P;
	}
}

void free_matrix(unsigned int ** matrix, int matrix_size) {
	int i = 0;
	for (i = 0; i < matrix_size; i++) {
		free(matrix[i]);
	}
	free(matrix);
}

unsigned int ** copy_matrix(unsigned int ** matrix, int matrix_size) {
	unsigned int ** aux_matrix = (unsigned int **)malloc(matrix_size * sizeof(unsigned int *));
	int i = 0, j = 0;
	for (i = 0; i < matrix_size; i++) {
		aux_matrix[i] = (unsigned int *)malloc(matrix_size * sizeof(unsigned int));
		memcpy(aux_matrix[i], matrix[i], matrix_size * sizeof(unsigned int));
		for (j = 0; j < matrix_size; j++) {
			aux_matrix[i][j] = aux_matrix[i][j] % P;
		}
	}
	return aux_matrix;
}

unsigned int ** inverse_matrix(unsigned int ** matrix, int matrix_size) {
	unsigned int ** inverse = make_eye(matrix_size);
	unsigned int ** aux_matrix = (unsigned int **)malloc(matrix_size * sizeof(unsigned int *));
	int i = 0, j = 0;
	for (i = 0; i < matrix_size; i++) {
		int modinv = modular_inverse[aux_matrix[i][i]];
		mult_k(inverse[i], modinv, matrix_size);
		mult_k(aux_matrix[i], modinv, matrix_size);
		for (j = 0; j < matrix_size; j++) {
			if (j == i) continue;
			sub(inverse[j], inverse[i], inverse[j][i], matrix_size);
			sub(aux_matrix[j], aux_matrix[i], aux_matrix[j][i], matrix_size);
		}
	}
	if (!is_eye(aux_matrix, matrix_size)) {
		free_matrix(inverse, matrix_size);
		inverse = NULL;
	}
	free_matrix(aux_matrix, matrix_size);
	return inverse;
}

void decode(byte ** bitmap_data, int pos, unsigned int ** A, unsigned int * B, int k) {
	unsigned int * X = mult_vec(A, B, k);
	int i = 0;
	for (i = 0; i < k; i++) {
		*(bitmap_data + pos)[i] = (byte)X[i];
	}
}

int calculate_parity_bit(unsigned char* hash, int k){
	int i;
	unsigned char resp = 0x00;
	int bit = 0, aux = 0;
	unsigned char* md = (unsigned char*)malloc(MY_MD5_DIGEST_LENGTH);
	MY_MD5(hash, k, md);
	for (i = 0; i< MY_MD5_DIGEST_LENGTH; i++){
		resp ^= md[i];
	}
	for (i = 0; i<8; i++){
		aux = resp & 1;
		resp = resp >> 1;
		bit ^= aux;
	}
	return bit;
}

void getBitsTweaked(int numb, unsigned char* bitmapData, unsigned char* alist, int* b, int k){
	unsigned char *calculateHash = (unsigned char*)malloc(k);
	int i, new, num;

	for(i=k-1; i >= 0; i--){

		if(i==k-1){
			num = b[i] -1;
		}else{
			num = b[i];
		}

		*(bitmapData+i) = *(alist +i) << b[i];
		new = numb & (1<<num) -1;

		if(i==k-1){
			*(calculateHash+i) = *(bitmapData+i) + (new<<1);
		}else{
			*(calculateHash+i) = *(bitmapData+i) + new;
		}

		*(bitmapData+i) += new;
		numb = numb >> num;
	}
	int bit = calculate_parity_bit(calculateHash, k);
	bit = bit << (b[k-1]-1);
	*(bitmapData+(k-1)) ^= bit;
}

void printMemory(unsigned char *data){
	int i;

	for(i=0; i<3;i++){
		printf("%2x ",*(data+i));
	}
	printf("\n");
}

int tweaker(unsigned char ** bitmapData, int k, int loc, int *b){
	unsigned char *data[k];
	unsigned char tweaked = 0;
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
	int loc[k], i, end = 0, restart = 0;
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

void decrypt_images(byte ** secret_bitmap_data_pt, byte **shadows_bitmap_data, int image_size, int k) {
	unsigned int i, num, w;
	int *b_coeff = calculate_b_coeffs(k);
	byte * secret_bitmap_data = *secret_bitmap_data_pt;
	if (secret_bitmap_data == NULL) {
		*secret_bitmap_data_pt = (byte *)malloc(image_size * sizeof(byte *));
		secret_bitmap_data = *secret_bitmap_data_pt;
	}
	for (w = 0; w < image_size; w += k) {
		unsigned int **data = (byte **)malloc(k * sizeof(unsigned int **));
		unsigned int **inverse = NULL;
		unsigned int * b;
		for (i = 0; i < k; i++) {
			data[i] = get_A(shadows_bitmap_data[i] + w, b_coeff, k);
		}
		b = get_B(shadows_bitmap_data[0], w, k);
		inverse = inverse_matrix(data, k);
		if (inverse == NULL) {
			printf("Some error happened. Probably matrix wasn't invertible.\n");
			return;
		}
		decode(secret_bitmap_data, w, inverse, b, k);
		free_matrix(inverse, k);
		free(b);
		free(inverse);
		free(data);
	}
}

