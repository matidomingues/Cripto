#include <stdlib.h>
#include <limits.h>
#include <dirent.h>
#include <string.h>
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
static unsigned int k = 0, n = 0;
static unsigned int shadows_count = 0;
static string secret_filename = NULL;
static string shadows_path = NULL;
static bitmap * secret = NULL;
static bitmap ** shadows = NULL;

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

void init_argtable_r() {
 	argtable_r = (void **)malloc(MAX_OPTS * sizeof(void*));
 	argtable_r[R_OPT_POS] = arg_lit1(R_OPT_SHORT, R_OPT_LONG, "Indicates to retrieve the secret in other images");
 	argtable_r[K_OPT_POS] = arg_int1(K_OPT_SHORT, K_OPT_LONG, "<n>", "Minimum shadows' quantity to unveil the secret");
 	argtable_r[N_OPT_POS] = arg_int0(N_OPT_SHORT, N_OPT_LONG, "<n>", "The total shadows' quantity among which the secret was distributed");
	argtable_r[SECRET_OPT_POS] = arg_file1(SECRET_OPT_SHORT, SECRET_OPT_LONG, "<file>", "Secret's output path");
 	argtable_r[DIR_OPT_POS] = arg_file0(DIR_OPT_SHORT, DIR_OPT_LONG, "<dir>", "The directory containing the images where the secret was distributed");
 	argtable_r[END_OPT_POS] = arg_end(10);
 	((arg_file *)argtable_r[DIR_OPT_POS])->filename = default_shadow_dir;
 	((arg_file *)argtable_r[DIR_OPT_POS])->basename = default_shadow_dir;
}

void init_argtable_d() {
 	argtable_d = (void **)malloc(MAX_OPTS * sizeof(void*));
 	argtable_d[D_OPT_POS] = arg_lit1(D_OPT_SHORT, D_OPT_LONG, "Indicates to distribute the secret in other images");
 	argtable_d[K_OPT_POS] = arg_int1(K_OPT_SHORT, K_OPT_LONG, "<n>", "Minimum shadows' quantity among which the secret will be distributed");
	argtable_d[N_OPT_POS] = arg_int0(N_OPT_SHORT, N_OPT_LONG, "<n>", "The total shadows' quantity among which the secret will be distributed");
 	argtable_d[SECRET_OPT_POS] = arg_file1(SECRET_OPT_SHORT, SECRET_OPT_LONG, "<file>", "Secret's path");
 	argtable_d[DIR_OPT_POS] = arg_file0(DIR_OPT_SHORT, DIR_OPT_LONG, "<dir>", "The directory containing the images where the secret was distributed");
	argtable_d[END_OPT_POS] = arg_end(10);
	((arg_file *)argtable_d[DIR_OPT_POS])->filename = ((arg_file *)argtable_d[DIR_OPT_POS])->basename = default_shadow_dir;
}

int realloc_shadows(int shadows_qty) {
	bitmap ** aux = (bitmap**)realloc(shadows, (shadows_qty + REALLOC_COEF) * sizeof(byte**));
	if (aux == NULL) {
		perror("Out of memory.\n");
		exit(EXIT_FAILURE);
	}
	shadows = aux;
	return shadows_qty + REALLOC_COEF;
}

void fetch_shadows() {
	DIR * dir_pt;
	struct dirent *ent;
	if ((dir_pt = opendir(shadows_path)) != NULL) {
		int max_shadows = BASE_ALLOC;
		shadows_count = 0;
		shadows = (bitmap **)malloc(BASE_ALLOC * sizeof(bitmap **));
		while ((ent = readdir(dir_pt)) != NULL && (n == N_OPT_FILES_COUNT || shadows_count < n)) {
			if (ent->d_type != DT_REG) continue;
			if (n == N_OPT_FILES_COUNT && max_shadows == shadows_count) {
				max_shadows = realloc_shadows(max_shadows);
			}
			string shadow_filename = (string)malloc((strlen(shadows_path) + strlen(ent->d_name) + 1) * sizeof(string));
			strcpy(shadow_filename, shadows_path);
			strcat(shadow_filename, PATH_SEPARATOR);
			strcat(shadow_filename, ent->d_name);
			if (strcmp(shadow_filename, secret_filename) == 0) {
				free(shadow_filename);
				continue;
			}
			shadows[shadows_count] = load_bitmap_file(shadow_filename);
			if (shadows[shadows_count] == NULL) {
				free(shadow_filename);
				continue;
			}
			free(shadow_filename);
			shadows_count++;
		}
	}
	closedir(dir_pt);
}

void free_shadows() {
	unsigned int i = 0;
	if (shadows != NULL) {
		for (i = 0; i < shadows_count; i++) {
			free(shadows[i]->data);
			free(shadows[i]);
		}
		free(shadows);
	}
}

//void init_files_d(void **argtable) {
//	secret_filename = realpath(((arg_file *)argtable[SECRET_OPT_POS])->filename[0], NULL);
//	secret_bitmap_data = LoadBitmapFile(secret_filename, &secret_info_header);
//
//	shadows_dir = realpath(((arg_file *)argtable[DIR_OPT_POS])->filename[0], NULL);
//	fetch_shadows();
//}

void init_files_r(void **argtable) {
	secret_filename = realpath(((arg_file *)argtable[SECRET_OPT_POS])->filename[0], NULL);
	if (secret_filename == NULL) {
		secret_filename = ((arg_file *)argtable[SECRET_OPT_POS])->filename[0];
	}
	const string path = (((arg_file *)argtable[DIR_OPT_POS])->count == 0) ? default_shadow_dir[0] :
			((arg_file *)argtable[DIR_OPT_POS])->filename[0];
	shadows_path = realpath(path, NULL);
	fetch_shadows();
	secret = (bitmap *)malloc(sizeof(bitmap));
	secret->data = (byte *)malloc(shadows[0]->i_hdr.biSize * sizeof(byte));
}

void free_files() {
	if (secret != NULL) {
		if (secret->data != NULL) {
			free(secret->data);
		}
		free(secret);
	}
	if (shadows_path != NULL) free(shadows_path);
	free_shadows();
}

//void distribute(void **argtable) {
//	k = *((arg_int *)argtable[K_OPT_POS])->ival;
//	n = (*((arg_int *)argtable[N_OPT_POS])->ival == 0)? N_OPT_FILES_COUNT : *((arg_int *)argtable[N_OPT_POS])->ival;
//	if (k < MIN_K || k > MAX_K) {
//		printf("Invalid or unsupported minimum number of shadows. k value must be between %d and %d.\n", MIN_K, MAX_K);
//		return;
//	}
//	if (((arg_int *)argtable[N_OPT_POS])->ival != NULL && n < k) {
//		printf("Invalid value for the maximum number of shadows.\n");
//		return;
//	}
//	init_files_d(argtable);
//	if (shadows_count < k) {
//		printf("Insufficient images in directory.\n");
//	} else {
//		encrypt_images(secret_bitmap_data, shadows_bitmap_data, shadows_count, secret_info_header.biSizeImage, k);
//	}
//	free_files();
//}

void retrieve(void **argtable) {
	k = *((arg_int *)argtable[K_OPT_POS])->ival;
	if (k < MIN_K || k > MAX_K) {
		printf("Invalid or unsupported minimum number of shadows. k value must between %d and %d.\n", MIN_K, MAX_K);
		return;
	}
	n = k;
	init_files_r(argtable);
	init_crypto(k);
	if (shadows_count < k) {
		printf("Insufficient images in directory.\n");
	} else {
		int i = 0;
		int image_size = 0;
		for (i = 0; i < k; i++) {
			if (i == 0) image_size = shadows[i]->i_hdr.biSizeImage;
			if (image_size != shadows[i]->i_hdr.biSizeImage) {
				image_size = 0;
			}
		}
		if (image_size == 0) {
			printf("Images size don't match.\n");
		} else {
			switch (decrypt_images(secret, shadows, image_size, k)) {
			case EXIT_OK:
				copy_headers(secret, shadows[0]);
				create_bitmap_file(secret_filename, secret);
				break;
			case EXIT_NON_INVERTIBLE_MATRIX:
				printf("An error ocurred while trying to decode.\n");
				break;
			case EXIT_PARITY_CHECK_ERR:
				printf("Parity check failed. File might be corrupted.\n");
				break;
			default:
				printf("An Unknown error ocurred. Abortin.\n");
				break;
			}
		}
	}
	free_files();
}

int main(int argc, char *argv[]) {
	int ret_value = EXIT_SUCCESS;
	init_argtable_d();
	init_argtable_r();
	int errors_d = arg_parse(argc, argv, argtable_d);
	int errors_r = arg_parse(argc, argv, argtable_r);

	if (errors_d == 0) {
		if (arg_nullcheck(argtable_d) == 0) {
//			distribute(argtable_d);
		} else {
			perror("Not enough memory!\n");
			ret_value = EXIT_FAILURE;
		}
	} else if (errors_r == 0) {
		if (arg_nullcheck(argtable_r) == 0) {
			retrieve(argtable_r);
		} else {
			perror("Not enough memory!\n");
			ret_value = EXIT_FAILURE;
		}
	} else {
		//TODO: Ensure to report errors for the most similar syntax.
		arg_print_errors(stdout, argtable_d[MAX_OPTS-1], argv[0]);
	}
	return ret_value;
}

//int main(void) {
//	init_crypto();
//	srand(time(NULL));
//	BITMAPINFOHEADER infoHeaders[6];
//	unsigned char *bitmapData[6];
//	bitmap * secret;
//	bitmap * shadow_bitmaps[2];
//	bitmapData[1] = LoadBitmapFile("20x20-1.bmp",&infoHeaders[1]);
//	bitmapData[2] = LoadBitmapFile("20x20-2.bmp",&infoHeaders[2]);
//	bitmapData[3] = LoadBitmapFile("20x20-3.bmp",&infoHeaders[3]);
//	bitmapData[4] = LoadBitmapFile("20x20-4.bmp",&infoHeaders[4]);
//	bitmapData[5] = LoadBitmapFile("20x20-5.bmp",&infoHeaders[5]);
//	bitmapData[1] = LoadBitmapFile("cachavachassd.bmp",&infoHeaders[1]);
//	bitmapData[2] = LoadBitmapFile("neuruspuchossd.bmp",&infoHeaders[2]);
//	bitmapData[3] = LoadBitmapFile("raimundossd.bmp",&infoHeaders[3]);
//	bitmapData[4] = LoadBitmapFile("20x20-4.bmp",&infoHeaders[4]);
//	bitmapData[5] = LoadBitmapFile("20x20-5.bmp",&infoHeaders[5]);
//	shadow_bitmaps[0] = load_bitmap_file("smartphones156x156ssd.bmp");
//	shadow_bitmaps[1] = load_bitmap_file("obelisco156x156ssd.bmp");
//	secret = (bitmap *)malloc(sizeof(bitmap));
//	secret->data = (byte*)malloc(shadow_bitmaps[0]->i_hdr.biSizeImage * sizeof(byte));
//	k = 2;
//	decrypt_images(secret, shadow_bitmaps, shadow_bitmaps[0]->i_hdr.biSizeImage, k);
//	copy_headers(secret, shadow_bitmaps[0]);
//	create_bitmap_file("secret-test1.bmp", secret);
//}

//int main(void){
//	srand(time(NULL));
//	BITMAPINFOHEADER infoHeaders[6];
//	unsigned char *bitmapData[6];
//	bitmapData[0] = LoadBitmapFile("20x20.bmp",&infoHeaders[0]);
//	bitmapData[1] = LoadBitmapFile("20x20-1.bmp",&infoHeaders[1]);
//	bitmapData[2] = LoadBitmapFile("20x20-2.bmp",&infoHeaders[2]);
//	bitmapData[3] = LoadBitmapFile("20x20-3.bmp",&infoHeaders[3]);
//	bitmapData[4] = LoadBitmapFile("20x20-4.bmp",&infoHeaders[4]);
//	bitmapData[5] = LoadBitmapFile("20x20-5.bmp",&infoHeaders[5]);
//	printBMPMatrix(bitmapData[1], infoHeaders[1]);
//
//	encrypt_images(bitmapData[0], &bitmapData[1], 5, infoHeaders[0].biSizeImage,3);
//
//	//printBMPMatrix(bitmapData[0], infoHeaders[0]);
//	printf("\n");
//
//	printBMPMatrix(bitmapData[1], infoHeaders[1]);
//}
