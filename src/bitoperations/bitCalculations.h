#ifndef BITCALCULATIONS_H_
#define BITCALCULATIONS_H_

#include <math.h>

unsigned char* calculateBits(unsigned char *bitmapData, int* b, int k);
int calculateB(unsigned char *bitmapData, unsigned char *calculatedA, int k);
int* calculateBArray(int k);
int calculateLinealIndependency(unsigned char *arr1, unsigned char *arr2, unsigned char *arr3);
void untweakBits(unsigned char *bitmapData, int action, int* b, int pos);
int tweakBits(unsigned char *bitmapData, pos, int* b);

#endif
