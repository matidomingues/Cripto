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
	int image_size = (bitmapInfoHeader->biSizeImage == 0) ? bitmapInfoHeader->biHeight * bitmapInfoHeader->biWidth :
			bitmapInfoHeader->biSizeImage;
	bitmapInfoHeader->biSizeImage = image_size;
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

bitmap * load_bitmap_file(string filename) {
	FILE * fp;
	if ((fp = fopen(filename, "rb")) == NULL) {
		return NULL;
	}
	bitmap * bm = (bitmap *)malloc(sizeof(bitmap));
	fread(&(bm->f_hdr), sizeof(BITMAPFILEHEADER), 1,fp);

		//verify that this is a bmp file by check bitmap id
		if (bm->f_hdr.bfType !=0x4D42)
		{
			free(bm);
			fclose(fp);
			return NULL;
		}

		//read the bitmap info header
		fread(&(bm->i_hdr), sizeof(BITMAPINFOHEADER),1,fp);

		//move file point to the begging of bitmap data
		fseek(fp, bm->f_hdr.bOffBits , SEEK_SET);

		//allocate enough memory for the bitmap image data
		int image_size = (bm->i_hdr.biSizeImage == 0) ? bm->i_hdr.biHeight * bm->i_hdr.biWidth :
				bm->i_hdr.biSizeImage;
		bm->i_hdr.biSizeImage = image_size;
		bm->data = (byte *)malloc(bm->i_hdr.biSizeImage);

		//verify memory allocation
		if (!bm->data)
		{
			free(bm);
			fclose(fp);
			return NULL;
		}

		//read in the bitmap image data
		fread(bm->data,bm->i_hdr.biSizeImage, 1,fp);

		//make sure bitmap image data was read
		if (bm->data == NULL)
		{
			fclose(fp);
			return NULL;
		}

		//close file and return bitmap image data
		fclose(fp);
	return bm;
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

void create_bitmap_file(string filename, bitmap * bitmap) {
	FILE *fp;
	fp = fopen(filename, "w");
	if (fp == NULL) {
		printf("Error while creating file. Terminating.\n");
		return;
	}
	if (fwrite(&(bitmap->f_hdr), sizeof(BITMAPFILEHEADER), 1, fp) < 1) {
		printf("Error while writing file header. Terminating.\n");
		fclose(fp);
		return;
	}
	if (fwrite(&(bitmap->i_hdr), sizeof(BITMAPINFOHEADER), 1, fp) < 1) {
		printf("Error while writing info header. Terminating.\n");
		fclose(fp);
		return;
	}

	int i = 0, c = 0;
	while((c = fputc(0, fp)) != EOF && i++ < bitmap->f_hdr.bOffBits);
	if (c == EOF) {
		printf("Error while writing padding.\n");
		fclose(fp);
		return;
	}
	if (fwrite(bitmap->data, bitmap->i_hdr.biSizeImage, 1, fp) != 1) {
		printf("Error while writing data\n");
		fclose(fp);
		return;
	}
	fclose(fp);
}

void copy_headers(bitmap *to, bitmap *from) {
	to->f_hdr.bOffBits = from->f_hdr.bOffBits;
	to->f_hdr.bfReserved = from->f_hdr.bfReserved;
	to->f_hdr.bfSize = from->f_hdr.bfSize;
	to->f_hdr.bfType = from->f_hdr.bfType;
    to->i_hdr.biSize = from->i_hdr.biSize;
    to->i_hdr.biWidth = from->i_hdr.biWidth;
    to->i_hdr.biHeight = from->i_hdr.biHeight;
    to->i_hdr.biPlanes = from->i_hdr.biPlanes;
    to->i_hdr.biBitCount = from->i_hdr.biBitCount;
    to->i_hdr.biCompression = from->i_hdr.biCompression;
    to->i_hdr.biSizeImage = from->i_hdr.biSizeImage;
    to->i_hdr.biXPelsPerMeter = from->i_hdr.biXPelsPerMeter;
    to->i_hdr.biYPelsPerMeter = from->i_hdr.biYPelsPerMeter;
    to->i_hdr.biClrUsed = from->i_hdr.biClrUsed;
    to->i_hdr.biClrImportant = from->i_hdr.biClrImportant;
//	memcpy(&(to->f_hdr), &(from->f_hdr), sizeof(BITMAPFILEHEADER));
//	memcpy(&(to->i_hdr), &(from->i_hdr), sizeof(BITMAPINFOHEADER));
}
