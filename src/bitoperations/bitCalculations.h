#ifndef BITCALCULATIONS_H_
#define BITCALCULATIONS_H_
#include <stdlib.h>
#include <math.h>

unsigned char* calculateBits(unsigned char *bitmapData, int* b, int k);
int calculateB(unsigned char *bitmapData, unsigned char *calculatedA, int k);
int* calculateBArray(int k);
int calculateLinealIndependency(unsigned char *arr1, unsigned char *arr2, unsigned char *arr3);
void untweakBits(unsigned char *bitmapData, int action, int* b, int pos);
unsigned char tweakBits(unsigned char *bitmapData, int pos, int* b);

#endif
