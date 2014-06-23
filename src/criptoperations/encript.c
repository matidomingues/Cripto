#include "encript.h"

void printMemory(unsigned char *data){
	int i;

	for(i=0; i<3;i++){
		printf("%2x ",*(data+i));
	}
	printf("\n");
}

int calculateParityBit(unsigned char* hash, int k){
	int i;
	unsigned char resp = 0x00;
	int bit = 0, aux = 0;
	unsigned char* md = (unsigned char*)malloc(CC_MD5_DIGEST_LENGTH);
	CC_MD5(hash, k, md);
	for (i = 0; i< CC_MD5_DIGEST_LENGTH; i++){
		resp ^= md[i];
	}
	for (i = 0; i<8; i++){
		aux = resp & 1;
		resp = resp >> 1;
		bit ^= aux;
	}
	free(md);
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
		printf("hash ");
		printMemory(calculateHash);
		printf("actual ");
		printMemory(bitmapData);
	}

	int bit = calculateParityBit(calculateHash, k);
	bit = bit << (b[k-1]-1);
	*(bitmapData+(k-1)) ^= bit;
	free(calculateHash);
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

void independenceNow(unsigned char **bitmapData, int images, int* b, int w, int k){
	int loc[k], i, end = 0, restart = 0;
	unsigned char *elems[k];
	for(i=1; i<=k;i++){
		loc[i-1] = i;
	}
	while(!end){
		for(i=0; i<k; i++){
			elems[i] = bitmapData[loc[i]];
		}
		if((restart = tweaker(elems,k,w,b)) || !sumValue(loc,k-1,k,images-1)){
			end = 1;
		}
	}
	if(restart){
		independenceNow(bitmapData,images,b,w,k);
	}
}

void encript(unsigned char** bitmapData, int images, int size, int k){
	int w, i, num;
	unsigned char* data;
	int* b = calculateBArray(k);
	for(w = 0 ; w< size; w += k){
		independenceNow(bitmapData, images, b, w, k);
		for(i=1; i<images; i++){
			data = calculateBits(bitmapData[i]+w, b, k);
			num = calculateB(bitmapData[0]+w, data, k);
			getBitsTweaked(num,bitmapData[i]+w, data, b,k);
			free(data);
		}
	}
	free(b);
}

