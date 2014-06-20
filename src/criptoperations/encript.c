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

void calculateLinealIndependency(unsigned char *bitmapData[6]){
	unsigned char* data[5];
	int i;
	for(i=0; i<5; i++){
//		data[i] = calculateBits(bitmapData[i+1], 3);
	}

}
