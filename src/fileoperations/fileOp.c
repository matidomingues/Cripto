#include "fileOp.h"

unsigned char *LoadBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader){
	FILE *filePtr; //our file pointer
	BITMAPFILEHEADER bitmapFileHeader; //our bitmap file header
	unsigned char *bitmapImage;  //store image data
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

void saveBitmapFile(char *filename, unsigned char* image){
	BITMAPINFOHEADER *bitmapInfoHeader;
	FILE *filePtr;
	BITMAPFILEHEADER bitmapFileHeader;
	filePtr = fopen(filename,"r+wb");
		if (filePtr == NULL)
			return;
	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1,filePtr);

	if (bitmapFileHeader.bfType !=0x4D42)
	{
		fclose(filePtr);
		return;
	}

	fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER),1,filePtr);

	fseek(filePtr, bitmapFileHeader.bOffBits , SEEK_SET);

	fwrite(image,bitmapInfoHeader->biSizeImage, 1,filePtr);

	fclose(filePtr);
}
