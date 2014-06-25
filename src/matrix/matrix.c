#include "matrix.h"

static unsigned int modular_inverse[P];
static long available_permutations = 0;
static long permutations_left = 0;
static int ** permutations = NULL;
static const unsigned int table[] = {1, 1, 2, 6, 24, 120, 720,
		5040, 40320, 362880, 3628800, 39916800, 479001600};

long factorial(unsigned int n) {
  if (n >= sizeof table / sizeof *table) // if appropriate, omit test if NDEBUG
	  return -1;
  return table[n];
}

void restart_permutations() {
	permutations_left = available_permutations;
}


//function to display the permutations.
void init_permutations(int N)
{
	//indicates sign
	byte sign = 1;
	//Tracks when to change sign.
	unsigned short change_sign = 0;
	//loop variables
	short i = 0,j = 0,k = 0, p = 0;
	//iterations
	long loops = available_permutations = factorial(N);
	//Array of pointers to hold the digits
	int **index_nos_ptr = (int **)malloc(N * sizeof(int *));
	//Repetition of each digit (Master copy)
	int *digit_rep_master = (int *)malloc(N * sizeof(int));
	//Repetition of each digit (Local copy)
	int *digit_rep_local = (int *)malloc(N * sizeof(int));
	//Index for Index_Nos_ptr
	int *element_num = (int *)malloc(N * sizeof(int));

	permutations = (int **)malloc(available_permutations * sizeof(int *));

	//Initialization
	for(i = 0;i < N;i++){
		//Allocate memory to hold the subsequent digits in the form of a LUT
	            //For N = N, memory required for LUT = N(N+1)/2
		index_nos_ptr[i] = (int *)malloc((N-i) * sizeof(int));
		//Initialise the repetition value of each digit (Master and Local)
		//Each digit repeats for (i-1)!, where 1 is the position of the digit
		digit_rep_local[i] = digit_rep_master[i] = factorial(N-i-1);
		//Initialise index values to access the arrays
		element_num[i] = N-i-1;
		//Initialise the arrays with the required digits
		for(j = 0;j < N-i;j++)
			*(index_nos_ptr[i] +j) = N-j-1;
	}

	while(loops-- > 0){
		permutations[p] = (int *)malloc(N * sizeof(int));
		for(i = 0;i < N;i++){
			//Print from MSD to LSD
			permutations[p][i] = *(index_nos_ptr[i] + element_num[i]);
			//Decrement the repetition count for each digit
			if(--digit_rep_local[i] <= 0){
				//Refill the repitition factor
				digit_rep_local[i] = digit_rep_master[i];
				//And the index to access the required digit is also 0...
				if(element_num[i] <= 0 && i != 0){
					//Reset the index
					element_num[i] = N-i-1;
					//Update the numbers held in Index_Nos_ptr[]
					for(j = 0,k = 0;j <= N-i;j++){
						//Exclude the preceeding digit (from the previous array) already printed.
						if(j != element_num[i-1]){
							*(index_nos_ptr[i]+k)= *(index_nos_ptr[i-1]+j);
							k++;
						}
					}
				}else
					//Decrement the index value so as to print the appropriate digit
					//in the same array
					element_num[i]--;
			}
		}

		if(!(change_sign-- > 0)){
			//Update the sign value.
			sign = -sign;
			change_sign = 1;
		}
		p++;
	}
	for (i = 0; i < N; i++) {
		free(index_nos_ptr[i]);
	}
	free(index_nos_ptr);
	free(digit_rep_master);
	free(digit_rep_local);
	//free(element_num);
}

void permutate(unsigned int ** matrix, int matrix_size) {
	if (permutations_left == 0) return;
	int i = 0;
	unsigned int ** aux = (unsigned int **)malloc(matrix_size * sizeof(unsigned int *));
	int p = available_permutations - permutations_left;
	for (i = 0; i < matrix_size; i++) {
		aux[i] = matrix[permutations[p][i]];
	}
	for (i = 0; i < matrix_size; i++) {
		matrix[i] = aux[i];
	}
	free(aux);
}

void next_permutation() {
	if (permutations_left == 0) return; else permutations_left--;
}

void reorder_matrix(unsigned int ** matrix, int matrix_size) {
	unsigned int ** aux = (unsigned int **)malloc(matrix_size * sizeof(unsigned int *));
	int p = available_permutations - permutations_left;
	int i = 0;
	for (i = 0; i < matrix_size; i++) {
		aux[permutations[p][i]] = matrix[i];
	}
	for (i = 0; i < matrix_size; i++) {
		matrix[i] = aux[i];
	}
	free(aux);
}
unsigned int ** make_eye(int k) {
	unsigned int ** eye_mat = (unsigned int **)malloc(k * sizeof(unsigned int *));
	int i = 0;
	for (i = 0; i < k; i++) {
		eye_mat[i] = (unsigned int *)calloc(k, sizeof(unsigned int));
		eye_mat[i][i] = 1;
	}
	return eye_mat;
}

boolean is_eye(unsigned int ** matrix, int k) {
	int i = 0, j = 0;
	for (i = 0; i < k ; i++) {
		for (j = 0; j < k ; j++) {
			if ((j == i && matrix[i][j] != 1) ||
					(j != i && matrix[i][j] != 0)) {
				return false;
			}
		}
	}
	return true;
}

void mult_k(unsigned int * vec, unsigned int k, int size) {
	int i = 0;
	for (i = 0; i < size; i++) {
		vec[i] *= k;
		vec[i] %= P;
	}
}

unsigned int * mult_vec(unsigned int ** matrix, unsigned int * vec, int size) {
	unsigned int * result = (unsigned int*)calloc(size, sizeof(unsigned int));
	int i = 0, j = 0;
	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			result[i] += (matrix[i][j] * vec[j]);
			result[i] %= P;
		}
	}
	return result;
}

void sub(unsigned int * vec1, unsigned int * vec2, unsigned int k, int size) {
	int i = 0;
	for (i = 0; i < size; i++) {
		int aux = vec1[i];
		aux -= ((vec2[i] * k) % P);
		if (aux < 0) {
			vec1[i] = P + aux;
		} else {
			vec1[i] = aux % P;
		}
	}
}

void free_matrix(unsigned int ** matrix, int matrix_size) {
	int i = 0;
	for (i = 0; i < matrix_size; i++) {
		free(matrix[i]);
	}
	free(matrix);
}

unsigned int ** copy_matrix(unsigned int ** matrix, int matrix_size) {
	unsigned int ** aux_matrix = (unsigned int **)malloc(matrix_size * sizeof(unsigned int *));
	int i = 0, j = 0;
	for (i = 0; i < matrix_size; i++) {
		aux_matrix[i] = (unsigned int *)malloc(matrix_size * sizeof(unsigned int));
		memcpy(aux_matrix[i], matrix[i], matrix_size * sizeof(unsigned int));
		for (j = 0; j < matrix_size; j++) {
			aux_matrix[i][j] = aux_matrix[i][j] % P;
		}
	}
	return aux_matrix;
}

unsigned int ** inverse_matrix(unsigned int ** matrix, int matrix_size) {
	unsigned int ** inverse = NULL;
	unsigned int ** aux_matrix = NULL;
	int i = 0, j = 0;
	restart_permutations();
	do {
		if (aux_matrix != NULL) {
			free_matrix(aux_matrix, matrix_size);
			free_matrix(inverse, matrix_size);
			next_permutation();
		}
		aux_matrix = copy_matrix(matrix, matrix_size);
		inverse = make_eye(matrix_size);
		permutate(inverse, matrix_size);
		permutate(aux_matrix, matrix_size);
		for (i = 0; i < matrix_size; i++) {
			int mod_inv = modular_inverse[aux_matrix[i][i]];
			mult_k(inverse[i], mod_inv, matrix_size);
			mult_k(aux_matrix[i], mod_inv, matrix_size);
			for (j = 0; j < matrix_size; j++) {
				if (j == i) continue;
				sub(inverse[j], inverse[i], aux_matrix[j][i], matrix_size);
				sub(aux_matrix[j], aux_matrix[i], aux_matrix[j][i], matrix_size);
			}
		}
	} while (!is_eye(aux_matrix, matrix_size) && permutations_left > 0);

	if (!is_eye(aux_matrix, matrix_size)) {
		free_matrix(inverse, matrix_size);
		inverse = NULL;
	} else {
		reorder_matrix(inverse, matrix_size);
	}

	free_matrix(aux_matrix, matrix_size);
	return inverse;
}


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

void print_matrix(unsigned int ** matrix, int size) {
	int i = 0, j = 0;
	for (i = 0; i < size; i++) {
		for (j = 0; j < size; j++) {
			printf("%d ", matrix[i][j]);
		}
		printf("\n");
	}
}

void print_vec(unsigned int * vec, int size) {
	int i =0;
	printf("[");
	for (i = 0; i < size; i++) {
		printf("%d ", vec[i]);
	}
	printf("]\n");
}
void init_inverses() {
	unsigned int i = 0;
	for (i = 0; i < P; i++) {
		modular_inverse[i] = modinv(i, P);
	}
}

boolean is_zeroes(unsigned int ** matrix, int matrix_size) {
	int i = 0, j = 0;
	for (i = 0; i < matrix_size; i++) {
		for (j = 0; j < matrix_size; j++) {
			if (matrix[i][j] != 0) return false;
		}
	}
	return true;
}

void init_matrix(int k) {
	init_inverses();
	init_permutations(k);
}
