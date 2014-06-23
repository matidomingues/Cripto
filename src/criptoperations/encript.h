#ifndef ENCRIPT_H_
#define ENCRIPT_H_

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "../fileoperations/fileOp.h"
#include "../includes/commons.h"
#include "../bitoperations/bitCalculations.h"
#include "../matrix/matrix.h"
#ifdef __APPLE__
	#include <CommonCrypto/CommonDigest.h>
	#define MY_MD5_DIGEST_LENGTH CC_MD5_DIGEST_LENGTH
	#define MY_MD5(X, Y, Z) CC_MD5((X), (Y), (Z))
#else
	#include <openssl/md5.h>
	#include <openssl/crypto.h>
	#define MY_MD5_DIGEST_LENGTH MD5_DIGEST_LENGTH
	#define MY_MD5(X,Y,Z) MD5((X), (Y), (Z))
#endif

void getBitsTweaked(int numb, unsigned char* bitmapData, unsigned char* alist, int* b, int k);
void encrypt_images(byte * secret_bitmap_data, byte ** shadows_bitmap_data, int shadows_qty, int image_size, int k);
void decrypt_images(bitmap * secret_bitmap, bitmap ** shadows_bitmaps, int image_size, int k);
void init_crypto();

#endif
