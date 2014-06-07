#include <stdlib.h>
#include <stdio.h>
#include <math.h>

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


unsigned char *LoadBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader){
	FILE *filePtr; //our file pointer
	BITMAPFILEHEADER bitmapFileHeader; //our bitmap file header
	unsigned char *bitmapImage;  //store image data
	int imageIdx=0;  //image index counter
	unsigned char tempRGB;  //our swap variable
	unsigned int test;
	unsigned short test2;
	filePtr = fopen(filename,"rb");
	    if (filePtr == NULL)
	        return NULL;
	//read the bitmap file header
	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1,filePtr);

	//verify that this is a bmp file by check bitmap id
	if (bitmapFileHeader.bfType !=0x4D42)
	{
		fclose(filePtr);
		return NULL;
	}

	//read the bitmap info header
	fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER),1,filePtr);

	//move file point to the begging of bitmap data
	fseek(filePtr, bitmapFileHeader.bOffBits , SEEK_SET);

	//allocate enough memory for the bitmap image data
	bitmapImage = (unsigned char*)malloc(bitmapInfoHeader->biSizeImage);

	//verify memory allocation
	if (!bitmapImage)
	{
		free(bitmapImage);
		fclose(filePtr);
		return NULL;
	}

	//read in the bitmap image data
	fread(bitmapImage,bitmapInfoHeader->biSizeImage, 1,filePtr);

	//make sure bitmap image data was read
	if (bitmapImage == NULL)
	{
		fclose(filePtr);
		return NULL;
	}

	//close file and return bitmap image data
	fclose(filePtr);
	return bitmapImage;

}

unsigned char* calculateBits(unsigned char *bitmapData, int k){
	unsigned char *arr, *aux;
	int i, l[k];
	int pot;
	int num;
	l[0] = 8;
	for(i = 1; i < k; i++){
		l[i] = l[i-1] - ceil(l[i-1]/(float)(k-(i-1)));
	}
	aux = arr = (unsigned char*)malloc(k);

	for(i = 0; i < k; i++){
		pot = (int)pow(2,ceil(l[i]/(float)(k-i)));
		num = 256-pot;
		*arr = (*bitmapData & num);
		*arr /= pot;
		bitmapData++;
		arr++;

	}
	return aux;
}

unsigned char* getA(unsigned char *bitmapData, int k){
	unsigned char *arr, *aux;
		int i, l[k];
		int pot;
		int num;
		l[0] = 8;
		for(i = 1; i < k; i++){
			l[i] = l[i-1] - ceil(l[i-1]/(float)(k-(i-1)));
		}
		aux = arr = (unsigned char*)malloc(k);

		for(i = 0; i < k; i++){
			pot = (int)pow(2,ceil(l[i]/(float)(k-i)));
			num = 256-pot;
			*arr = (*bitmapData & num);
			*arr /= pot;
			bitmapData++;
			arr++;

		}
		return aux;
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

void printBMPMatrix(unsigned char *bitmapData, BITMAPINFOHEADER infoHeader){
	int i,w;
	for(i=0; i<infoHeader.biHeight; i++){
		for(w=0; w<infoHeader.biWidth ; w++){
			printf("%2x ",*bitmapData);
			bitmapData++;
		}
		printf("\n");
	}
}

void getBitsTweaked(int b, unsigned char* bitmapData, unsigned char* alist, int k){
	int i, l[k], pot, new, num;
	l[0] = 8;
	for(i = 1; i < k; i++){
		l[i] = l[i-1] - ceil(l[i-1]/(float)(k-(i-1)));
	}
	for(i=2; i >= 0; i--){
		num = ceil(l[i]/(float)(k-i));
		*(bitmapData+i) = *(alist +i) << num;
		new = b & (1<<num) -1;
		*(bitmapData+i) += new;
		b = b >> num;
	}
}

int main(void){
	BITMAPINFOHEADER infoHeaders[6];
	unsigned char *bitmapData[6], *data;
	int i, num, w;
	bitmapData[0] = LoadBitmapFile("20x20.bmp",&infoHeaders[0]);
	bitmapData[1] = LoadBitmapFile("20x20-1.bmp",&infoHeaders[1]);
	bitmapData[2] = LoadBitmapFile("20x20-2.bmp",&infoHeaders[2]);
	bitmapData[3] = LoadBitmapFile("20x20-3.bmp",&infoHeaders[3]);
	bitmapData[4] = LoadBitmapFile("20x20-4.bmp",&infoHeaders[4]);
	bitmapData[5] = LoadBitmapFile("20x20-5.bmp",&infoHeaders[5]);
	for(w = 0 ; w< infoHeaders[0].biSizeImage; w += 3){
		for(i = 1; i < 2; i++){
			data = calculateBits(bitmapData[i], 3);
			num = calculateB(bitmapData[0], data, 3);
			getBitsTweaked(num,bitmapData[i], data,3);
			bitmapData[i] += 3;
		}
		bitmapData[0] += 3;
	}

	printBMPMatrix(bitmapData[0], infoHeaders[0]);
	printf("\n");
	printBMPMatrix(bitmapData[1]-402, infoHeaders[1]);

}




