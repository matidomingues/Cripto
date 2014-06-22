#include <stdlib.h>
#include <limits.h>
#include <dirent.h>
#include "includes/commons.h"
#include "fileoperations/fileOp.h"
#include "criptoperations/encript.h"

#define MAX_OPTS 6
#define D_OPT_POS 0
#define D_OPT_SHORT "d"
#define D_OPT_LONG NULL
#define R_OPT_POS 0
#define R_OPT_SHORT "r"
#define R_OPT_LONG NULL
#define K_OPT_POS 1
#define K_OPT_SHORT "k"
#define K_OPT_LONG NULL
#define MIN_K 2
#define MAX_K 3
#define N_OPT_FILES_COUNT UINT_MAX
#define N_OPT_POS 2
#define N_OPT_SHORT "n"
#define N_OPT_LONG NULL
#define SECRET_OPT_POS 3
#define SECRET_OPT_SHORT NULL 
#define SECRET_OPT_LONG "secret"
#define DIR_OPT_POS 4
#define DIR_OPT_SHORT NULL
#define DIR_OPT_LONG "dir"
#define DEFAULT_DIR_OPT "."
#define END_OPT_POS 5

#define BASE_ALLOC (MAX_K * 2)
#define REALLOC_COEF 3

/* Argument variables */
static void ** argtable_d = NULL;// = {command_d, k_d, n_d, secret_d, dir_d, end_d };
static void ** argtable_r = NULL;// = {command_r, k_r, n_r, secret_r, dir_r, end_r };

/* Main variables */
static const char *default_shadow_dir[] = { DEFAULT_DIR_OPT };
static const char *default_shadow_dir_extension[] = { "" };
static unsigned int k = 0, n = 0;
static string secret_filename = NULL;
static string secret_filepath = NULL;
static BITMAPINFOHEADER *secret_info_header = NULL;
static byte *secret_bitmap_data = NULL;
static string shadows_dir = NULL;
static unsigned int shadows_qty = 0;
static BITMAPINFOHEADER *shadows_info_headers = NULL;
static byte **shadows_bitmap_data = NULL;

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

void fetch_shadows() {
	DIR * dir_pt;
	struct dirent *ent;
	shadows_info_headers = (BITMAPINFOHEADER *)malloc(BASE_ALLOC * sizeof(BITMAPINFOHEADER));
	shadows_bitmap_data = (byte **)malloc(BASE_ALLOC * sizeof(byte));
	if ((dir_pt = opendir(shadows_dir)) != NULL) {
		int max_shadows = BASE_ALLOC;
		shadows_qty = 0;
		while ((ent = readdir(dir_pt)) != NULL && (n == N_OPT_FILES_COUNT || shadows_qty < n)) {
			if (ent->d_type != DT_REG) continue;
			if (n == N_OPT_FILES_COUNT && max_shadows == shadows_qty) {
				max_shadows = realloc_shadows(max_shadows);
			}
			string shadow_filename = (string)malloc((strlen(shadows_dir) + strlen(ent->d_name) + 1) * sizeof(string));
			strcpy(shadow_filename, shadows_dir);
			strcat(shadow_filename, PATH_SEPARATOR);
			strcat(shadow_filename, ent->d_name);
			shadows_bitmap_data[shadows_qty] = LoadBitmapFile(shadow_filename, &shadows_info_headers[shadows_qty]);
			free(shadow_filename);
			shadows_qty++;
		}
	}
	closedir(dir_pt);
}

void free_shadows() {
	unsigned int i = 0;
	for (i = 0; i < shadows_qty; i++) {
		free(shadows_bitmap_data[i]);
		free(&shadows_info_headers[i]);
	}
	free(shadows_bitmap_data);
	free(shadows_info_headers);
}

void init_files_d(void **argtable) {
	secret_filename = realpath(((arg_file *)argtable[SECRET_OPT_POS])->filename[0], NULL);
	secret_bitmap_data = LoadBitmapFile(secret_filename, &secret_info_header);

	shadows_dir = realpath(((arg_file *)argtable[DIR_OPT_POS])->filename[0], NULL);
	fetch_shadows();
}

void init_files_r(void **argtable) {
	shadows_dir = realpath(((arg_file *)argtable[DIR_OPT_POS])->filename[0], NULL);
	fetch_shadows();

	secret_filename = realpath(((arg_file *)argtable[SECRET_OPT_POS])->filename[0], NULL);
	secret_bitmap_data = (byte *)malloc(shadows_info_headers[0].biSizeImage);
}

void free_files() {
	free(secret_filename);
	free(secret_bitmap_data);
	free(shadows_dir);
	free_shadows();
}

void distribute(void **argtable) {
	k = *((arg_int *)argtable[K_OPT_POS])->ival;
	n = (*((arg_int *)argtable[N_OPT_POS])->ival == 0)? N_OPT_FILES_COUNT : *((arg_int *)argtable[N_OPT_POS])->ival;
	if (k < MIN_K || k > MAX_K) {
		printf("Invalid or unsupported minimum number of shadows. k value must be between %d and %d.\n", MIN_K, MAX_K);
		return;
	}
	if (((arg_int *)argtable[N_OPT_POS])->ival != NULL && n < k) {
		printf("Invalid value for the maximum number of shadows.\n");
		return;
	}
	init_files_d(argtable);
	if (shadows_qty < k) {
		printf("Insufficient images in directory.\n");
	} else {
		encrypt_images(secret_bitmap_data, shadows_bitmap_data, shadows_qty, secret_info_header->biSizeImage, k);
	}
	free_files();
}

void retrieve(void **argtable) {
	k = *((arg_int *)argtable[K_OPT_POS])->ival;
	if (k < MIN_K || k > MAX_K) {
		printf("Invalid or unsupported minimum number of shadows. k value must between %d and %d.\n", MIN_K, MAX_K);
		return;
	}
	n = k;
	init_files_r(argtable);
	if (shadows_qty < k) {
		printf("Insufficient images in directory.\n");
	} else {
		int i = 0;
		int image_size = 0;
		for (i = 0; i < k; i++) {
			if (i == 0) image_size = shadows_info_headers[i].biSizeImage;
			if (image_size != shadows_info_headers[i].biSizeImage) {
				image_size = 0;
			}
		}
		if (image_size == 0) {
			printf("Images size don't match.\n");
		} else {
			decrypt_images(&secret_bitmap_data, shadows_bitmap_data, image_size, k);
			if (secret_bitmap_data == NULL) {
				printf("Error in file decryption.\n");
			} else {
				saveBitmapFile(secret_filename, secret_bitmap_data);
			}
		}
	}
}

//int main(int argc, char *argv[]) {
//	int ret_value = EXIT_SUCCESS;
//	init_inverses();
//	init_argtable_d();
//	init_argtable_r();
//	int errors_d = arg_parse(argc, argv, argtable_d);
//	int errors_r = arg_parse(argc, argv, argtable_r);
//
//	if (errors_d == 0) {
//		if (arg_nullcheck(argtable_d) == 0) {
//			distribute(argtable_d);
//		} else {
//			perror("Not enough memory!\n");
//			ret_value = EXIT_FAILURE;
//		}
//	} else if (errors_r == 0) {
//		if (arg_nullcheck(argtable_r) == 0) {
//			retrieve(argtable_r);
//		} else {
//			perror("Not enough memory!\n");
//			ret_value = EXIT_FAILURE;
//		}
//	} else {
//		//TODO: Ensure to report errors for the most similar syntax.
//		arg_print_errors(stdout, argtable_d[MAX_OPTS-1], argv[0]);
//	}
//
//}

//int main(void) {
//	srand(time(NULL));
//	BITMAPINFOHEADER infoHeaders[6];
//	unsigned char *bitmapData[6];
//	bitmapData[1] = LoadBitmapFile("20x20-1.bmp",&infoHeaders[1]);
//	bitmapData[2] = LoadBitmapFile("20x20-2.bmp",&infoHeaders[2]);
//	bitmapData[3] = LoadBitmapFile("20x20-3.bmp",&infoHeaders[3]);
//	bitmapData[4] = LoadBitmapFile("20x20-4.bmp",&infoHeaders[4]);
//	bitmapData[5] = LoadBitmapFile("20x20-5.bmp",&infoHeaders[5]);
//	bitmapData[0] = (byte *)malloc(infoHeaders[1].biSizeImage);
//
//	decrypt_images(&bitmapData[0], bitmapData + 1, infoHeaders[1].biSizeImage, 3);
//	saveBitmapFile("secret-20x20.bmp", secret_bitmap_data);
//}

int main(void){
	srand(time(NULL));
	BITMAPINFOHEADER infoHeaders[6];
	unsigned char *bitmapData[6];
	bitmapData[0] = LoadBitmapFile("20x20.bmp",&infoHeaders[0]);
	bitmapData[1] = LoadBitmapFile("20x20-1.bmp",&infoHeaders[1]);
	bitmapData[2] = LoadBitmapFile("20x20-2.bmp",&infoHeaders[2]);
	bitmapData[3] = LoadBitmapFile("20x20-3.bmp",&infoHeaders[3]);
	bitmapData[4] = LoadBitmapFile("20x20-4.bmp",&infoHeaders[4]);
	bitmapData[5] = LoadBitmapFile("20x20-5.bmp",&infoHeaders[5]);
	printBMPMatrix(bitmapData[1], infoHeaders[1]);

	encript(bitmapData, 6, infoHeaders[0].biSizeImage,3);

	//printBMPMatrix(bitmapData[0], infoHeaders[0]);
	printf("\n");

	printBMPMatrix(bitmapData[1], infoHeaders[1]);
}
