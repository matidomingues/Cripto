#include <stdlib.h>
#include <string.h>
#include <argtable2.h>
#include <dirent.h>
#include <limits.h>
#include "fileoperations/fileOp.h"
#include "criptoperations/encript.h"

#define MAX_OPTS 6
#define P 251
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

#define PATH_SEPARATOR "/"

#define true 1
#define false 0

typedef struct arg_lit arg_lit;
typedef struct arg_int arg_int;
typedef struct arg_file arg_file;
typedef char *string;
typedef unsigned char boolean;
typedef unsigned char byte;

/* Argument variables */
void ** argtable_d;// = {command_d, k_d, n_d, secret_d, dir_d, end_d };
void ** argtable_r;// = {command_r, k_r, n_r, secret_r, dir_r, end_r };

/* Main variables */
const char *default_shadow_dir[] = { DEFAULT_DIR_OPT };
const char *default_shadow_dir_extension[] = { "" };
unsigned int k = 0, n = 0;
string secret_filename;
string secret_filepath;
BITMAPINFOHEADER secret_info_header;
byte *secret_bitmap_data;
string shadows_dir;
unsigned int shadows_qty;
BITMAPINFOHEADER *shadows_info_headers;
byte **shadows_bitmap_data;

unsigned int modular_inverse[P];

unsigned int modinv(unsigned int u, unsigned int v)
{
    unsigned int inv, u1, u3, v1, v3, t1, t3, q;
    int iter;
    /* Step X1. Initialize */
    u1 = 1;
    u3 = u;
    v1 = 0;
    v3 = v;
    /* Remember odd/even iterations */
    iter = 1;
    /* Step X2. Loop while v3 != 0 */
    while (v3 != 0)
    {
        /* Step X3. Divide and "Subtract" */
        q = u3 / v3;
        t3 = u3 % v3;
        t1 = u1 + q * v1;
        /* Swap */
        u1 = v1; v1 = t1; u3 = v3; v3 = t3;
        iter = -iter;
    }
    /* Make sure u3 = gcd(u,v) == 1 */
    if (u3 != 1)
        return 0;   /* Error: No inverse exists */
    /* Ensure a positive result */
    if (iter < 0)
        inv = v - u1;
    else
        inv = u1;
    return inv;
}

void init_inverses() {
	unsigned int i = 0;
	for (i = 0; i < P; i++) {
		modular_inverse[i] = modinv(i, P);
	}
}

void printBMPMatrix(unsigned char *bitmapData, BITMAPINFOHEADER infoHeader){
	unsigned int i,w;
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
	((arg_file *)argtable_r[DIR_OPT_POS])->filename = ((arg_file *)argtable_r[DIR_OPT_POS])->basename = default_shadow_dir;
	((arg_file *)argtable_r[DIR_OPT_POS])->extension = default_shadow_dir_extension;
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
	((arg_file *)argtable_d[DIR_OPT_POS])->extension = default_shadow_dir_extension;
}

int realloc_shadows(int max) {
	BITMAPINFOHEADER * aux_info_headers = (BITMAPINFOHEADER *)realloc(shadows_info_headers, (max += REALLOC_COEF) * sizeof(BITMAPINFOHEADER));
	if (aux_info_headers != shadows_info_headers) free(shadows_info_headers);
	shadows_info_headers = aux_info_headers;
	byte **aux_shadows_data = (byte **)realloc(shadows_bitmap_data, (shadows_qty + REALLOC_COEF) * sizeof(byte**));
	if (aux_shadows_data != shadows_bitmap_data) free(shadows_bitmap_data);
	shadows_bitmap_data = aux_shadows_data;
	return max;
}

void fetch_shadows() {
	DIR *dir_pt;
	struct dirent *ent;
	//TODO: make this happen!
	boolean secret_in_folder = false; //(strcmp(secret_filename, shadows_dir) == 0)? true : false;

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
		byte **data = (byte **)malloc(shadows_qty * k * sizeof(byte **));
		unsigned int i, num, w;
		int* b = calculateBArray(k);
		for(w = 0 ; w < secret_info_header.biSizeImage; w += k){
			for(i = 0; i < k-1; i++){
				data[i] = calculateBits(shadows_bitmap_data[i] + w, b, k);
				num = calculateB(secret_bitmap_data + w, data[i], k);
				getBitsTweaked(num, shadows_bitmap_data[i] + w, data[i], b, k);
				free(data[i]);
			}
		}
		free(data);
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
		byte **data = (byte **)malloc(shadows_qty * k * sizeof(byte **));
		unsigned int i, num, w;
		int *b = calculateBArray(k);
		for (w = 0; w < shadows_info_headers[0].biSizeImage; w += k) {
			for (i = 0; i < k; i++) {
				data[i] = calculateBits(shadows_bitmap_data[i] + w, b, k);
				if (i == k-1) {
				}
			}
		}
	}
}


int main(int argc, char *argv[]) {
	int ret_value = EXIT_SUCCESS;
	init_inverses();
	init_argtable_d();
	init_argtable_r();
	int errors_d = arg_parse(argc, argv, argtable_d);
	int errors_r = arg_parse(argc, argv, argtable_r);

	if (errors_d == 0) {
		if (arg_nullcheck(argtable_d) == 0) {
			distribute(argtable_d);
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

	arg_freetable(argtable_r, MAX_OPTS);
	arg_freetable(argtable_d, MAX_OPTS);
	return ret_value;

//	BITMAPINFOHEADER infoHeaders[6];
//	unsigned char *bitmapData[6], *data[5];
//	int i, num, w;
//	int* b = calculateBArray(3);
//	for(i = 0; i<3; i++){
//		printf("%d\n", b[i]);
//	}
//	bitmapData[0] = LoadBitmapFile("20x20.bmp",&infoHeaders[0]);
//	bitmapData[1] = LoadBitmapFile("20x20-1.bmp",&infoHeaders[1]);
//	bitmapData[2] = LoadBitmapFile("20x20-2.bmp",&infoHeaders[2]);
//	bitmapData[3] = LoadBitmapFile("20x20-3.bmp",&infoHeaders[3]);
//	bitmapData[4] = LoadBitmapFile("20x20-4.bmp",&infoHeaders[4]);
//	bitmapData[5] = LoadBitmapFile("20x20-5.bmp",&infoHeaders[5]);
//	for(w = 0 ; w< infoHeaders[0].biSizeImage; w += 3){
//		for(i = 1; i < 2; i++){
//			data[i-1] = calculateBits(bitmapData[i]+w, b, 3);
//			num = calculateB(bitmapData[0]+w, data[i-1], 3);
//			getBitsTweaked(num,bitmapData[i]+w, data[i-1], b,3);
//		}
//	}
//
//	printBMPMatrix(bitmapData[0], infoHeaders[0]);
//	printf("\n");
//	printBMPMatrix(bitmapData[1], infoHeaders[1]);

}




