#include "encript.h"

int calculateParityBit(unsigned char* hash, int k){
	int i;
	unsigned char resp = 0x00;
	int bit = 0, aux = 0;
	unsigned char* md = malloc (CC_MD5_DIGEST_LENGTH);
	CC_MD5(hash, k, md);
	for(i=0; i<CC_MD5_DIGEST_LENGTH; i++){
		resp ^= md[i];
	}
	for(i = 0; i<8;i++){
		aux = resp & 1;
		resp = resp >> 1;
		bit ^= aux;
	}
	return bit;
}

void getBitsTweaked(int numb, unsigned char* bitmapData, unsigned char* alist, int* b, int k){
	unsigned char* calculateHash = malloc(k);
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
	int bit = calculateParityBit(calculateHash, k);
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

int Tweaker(unsigned char ** bitmapData, int k, int loc, int *b){
	unsigned char *data[k];
	int i, end = 0, tweak = 0, pos = 0 , lastop = 0, lastpos = 0, lastweak = 0;
	do{
		for(i = 0; i < k; i++){
			data[i] = calculateBits((bitmapData[i]+loc), b, 3);
		}
		if(calculateLinealIndependency(data[0], data[1], data[2]) == 0){
			untweakBits(bitmapData[lastweak]+loc, lastop, b, lastpos);
			lastweak = tweak;
			lastpos = pos;
			lastop = tweakBits(bitmapData[tweak++]+loc, pos, b);
			if(tweak == 4){
				pos++;
				tweak = 0;
				if(pos == 3){
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
}

int independenceNow(unsigned char **bitmapData, int images, int* b, int w){

	return -1;
}

void encript(unsigned char** bitmapData, int images, int size){
	int w, i, num;
	unsigned char* data;
	int* b = calculateBArray(3);
	for(w = 0 ; w< size; w += 3){
		if(independenceNow(bitmapData, images, b, w));
		for(i=1; i<4; i++){
			data = calculateBits(bitmapData[i]+w, b, 3);
			num = calculateB(bitmapData[0]+w, data, 3);
			getBitsTweaked(num,bitmapData[i]+w, data, b,3);
		}
	}
}

