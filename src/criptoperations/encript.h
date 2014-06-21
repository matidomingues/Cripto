#ifndef ENCRIPT_H_
#define ENCRIPT_H_

#include <stdlib.h>
#include <math.h>
#include <openssl/md5.h>
#include "../bitoperations/bitCalculations.h"

void getBitsTweaked(int numb, unsigned char* bitmapData, unsigned char* alist, int* b, int k);
void encript(unsigned char** bitmapData, int images, int size);

#endif
