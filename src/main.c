#include <stdlib.h>
#include <stdio.h>

typedef int DWORD;
typedef short WORD;

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

int main(void){
	BITMAPINFOHEADER bitmapInfoHeader;
	unsigned char *bitmapData;
	bitmapData = LoadBitmapFile("20x20.bmp",&bitmapInfoHeader);
	int i = 0, w = 0;
	for(i=0; i<bitmapInfoHeader.biHeight; i++){
		for(w=0; w<bitmapInfoHeader.biWidth ; w++){
			printf("%2x ",*bitmapData);
			bitmapData++;
		}
		printf("\n");
	}
}




