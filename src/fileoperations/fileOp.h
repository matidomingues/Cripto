#ifndef FILEOP_H_
#define FILEOP_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../includes/commons.h"

typedef unsigned int DWORD;
typedef unsigned short WORD;

#pragma pack(push, 1)

typedef struct tagBITMAPFILEHEADER
{
	unsigned short bfType;  //specifies the file type
    unsigned int bfSize;  //specifies the size in bytes of the bitmap file
    unsigned int bfReserved;  //reserved; must be 0
    unsigned int bOffBits;  //species the offset in bytes from the bitmapfileheader to the bitmap bits
}BITMAPFILEHEADER;

#pragma pack(pop)
#pragma pack(push, 1)

typedef struct tagBITMAPINFOHEADER
{
    DWORD biSize;  //specifies the number of bytes required by the struct
    DWORD biWidth;  //specifies width in pixels
    DWORD biHeight;  //species height in pixels
    WORD biPlanes; //specifies the number of color planes, must be 1
    WORD biBitCount; //specifies the number of bit per pixel
    DWORD biCompression;//spcifies the type of compression
    DWORD biSizeImage;  //size of image in bytes
    DWORD biXPelsPerMeter;  //number of pixels per meter in x axis
    DWORD biYPelsPerMeter;  //number of pixels per meter in y axis
    DWORD biClrUsed;  //number of colors used by th ebitmap
    DWORD biClrImportant;  //number of colors that are important
}BITMAPINFOHEADER;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct bitmap {
	BITMAPFILEHEADER f_hdr;
	BITMAPINFOHEADER i_hdr;
	byte * data;
} bitmap;
#pragma pack(pop)

byte *LoadBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader);
bitmap * load_bitmap_file(string filename);
void saveBitmapFile(char *filename, unsigned char* image);
void create_bitmap_file(string filename, bitmap * bitmap);
void copy_headers(bitmap *to, bitmap *from);

#endif
