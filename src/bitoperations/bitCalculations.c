#include "bitCalculations.h"

unsigned char* calculateBits(unsigned char *bitmapData, int* b, int k){
	unsigned char *arr;
	int i, pot;

	arr = (unsigned char*)malloc(k);

	for(i = 0; i < k; i++){
		pot = (int)pow(2,b[i]);
		arr[i] = (*bitmapData & (256-pot));
		arr[i] /= pot;
		bitmapData++;
	}
	return arr;
}

int calculateB(unsigned char *bitmapData, unsigned char *calculatedA, int k){
	int i;
	long sum = 0;
	for(i=0; i<k; i++){
		sum += (*bitmapData * *calculatedA);
		bitmapData++;
		calculatedA++;
	}
	return sum % 251;
}

int* calculateBArray(int k){
	int i, l[k], *b;
	b = malloc(k*sizeof(int));
	l[0] = 8;
	b[0] = ceil(l[0]/(float)(k));
	for(i = 1; i < k; i++){
		l[i] = l[i-1] - ceil(l[i-1]/(float)(k-(i-1)));
		b[i] = ceil(l[i]/(float)(k-i));
	}
	b[k-1] += 1;
	return b;
}


int calculateLinealIndependency(unsigned char *arr1, unsigned char *arr2, unsigned char *arr3){
	unsigned char *arr[3];
	arr[0] = arr1;
	arr[1] = arr2;
	arr[2] = arr3;
	int i, sum, res, total = 0;
	for(i=0; i<3; i++){
		sum = arr[i][0]*arr[(i+1)%3][1]*arr[(i+2)%3][2];
		res = arr[i][2]*arr[(i+1)%3][1]*arr[(i+2)%3][0];
		total += (sum-res);
	}
	return total;
}

void untweakBits(unsigned char *bitmapData, int action, int* b, int pos){
	int num = (1<<b[pos])*(-1*action);
	*(bitmapData+pos) +=num;
}

int tweakBits(unsigned char *bitmapData, pos, int* b){
	int num = 1<<b[pos];
	unsigned char aux = *(bitmapData+pos);
	*(bitmapData+pos) += num;
	if(*(bitmapData+pos) < aux){
		*(bitmapData+pos) = aux-num;
		return -1;
	}
	return 1;
}
