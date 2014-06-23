/*
 * matrix.h
 */

#ifndef MATRIX_H_
#define MATRIX_H_

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../includes/commons.h"

#define P 251

void init_inverses();
unsigned int ** make_eye(int k);
boolean is_eye(unsigned int ** matrix, int k);
void mult_k(unsigned int * vec, unsigned int k, int size);
unsigned int * mult_vec(unsigned int ** matrix, unsigned int * vec, int size);
void sub(unsigned int * vec1, unsigned int * vec2, unsigned int k, int size);
void free_matrix(unsigned int ** matrix, int matrix_size);
unsigned int ** copy_matrix(unsigned int ** matrix, int matrix_size);
unsigned int ** inverse_matrix(unsigned int ** matrix, int matrix_size);
void print_matrix(unsigned int ** matrix, int matrix_size);
boolean is_zeroes(unsigned int ** matrix, int matrix_size);

#endif /* MATRIX_H_ */
