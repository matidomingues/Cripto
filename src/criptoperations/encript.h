#ifndef ENCRIPT_H_
#define ENCRIPT_H_

#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <CommonCrypto/CommonDigest.h>
#include "../bitoperations/bitCalculations.h"

void encript(unsigned char** bitmapData, int images, int size, int k);

#endif
