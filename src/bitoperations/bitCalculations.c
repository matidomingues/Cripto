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
