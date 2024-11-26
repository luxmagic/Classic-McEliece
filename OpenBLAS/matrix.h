#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

typedef struct __attribute__ ((packed)) M
{
	int lines;
	int columns;
	double *value;
} M;

double get_determinant(M *a);
M *get_inverse(M *a);
int get_rank(M *a);

int matrix_multiply(M *a, M *b, M *c);
void scalar_multiply(M *a, double scalar);
void vector_mupliply(M *a, M *v);

#endif