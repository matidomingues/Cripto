/*
 * matrix.c
 */
#include "matrix.h"

unsigned int modular_inverse[P];

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
			result[i] += matrix[i][j] * vec[j];
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
	unsigned int ** inverse = make_eye(matrix_size);
	unsigned int ** aux_matrix = NULL;
	int i = 0, j = 0;
	do {
		if (aux_matrix != NULL) {
			free_matrix(aux_matrix, matrix_size);
		}
		aux_matrix = copy_matrix(matrix, matrix_size);
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
	} while (!is_eye(aux_matrix, matrix_size));

	if (!is_eye(aux_matrix, matrix_size)) {
		free_matrix(inverse, matrix_size);
		inverse = NULL;
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
