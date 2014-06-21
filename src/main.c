#include <stdlib.h>
#include "fileoperations/fileOp.h"
#include "criptoperations/encript.h"


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

int main(void){
	BITMAPINFOHEADER infoHeaders[6];
	unsigned char *bitmapData[6];
	bitmapData[0] = LoadBitmapFile("20x20.bmp",&infoHeaders[0]);
	bitmapData[1] = LoadBitmapFile("20x20-1.bmp",&infoHeaders[1]);
	bitmapData[2] = LoadBitmapFile("20x20-2.bmp",&infoHeaders[2]);
	bitmapData[3] = LoadBitmapFile("20x20-3.bmp",&infoHeaders[3]);
	bitmapData[4] = LoadBitmapFile("20x20-4.bmp",&infoHeaders[4]);
	bitmapData[5] = LoadBitmapFile("20x20-5.bmp",&infoHeaders[5]);
	printBMPMatrix(bitmapData[1], infoHeaders[1]);

	encript(bitmapData, 6, infoHeaders[0].biSizeImage);



	//printBMPMatrix(bitmapData[0], infoHeaders[0]);
	printf("\n");

	printBMPMatrix(bitmapData[1], infoHeaders[1]);

}




