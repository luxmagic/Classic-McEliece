#include <stdio.h>
#include <stdlib.h>

#include "utility.h"
#include "error.h"
#include "matrix.h"

#define EPSILON 1e-40

M *create_matrix(int lines, int columns) 
{
	M *a = (M *)malloc(sizeof(M));
	if(a == NULL) {
		return NULL;
	}

	a->lines       = lines;
	a->columns     = columns;
    a->value       = (double *)malloc(lines * columns * sizeof(double));

	// for (int i = 0; i < lines; i++) {
	// 	for (int j = 0; j < columns; j++) {
	// 		a->value[i * columns + j] = 0.0;
	// 	}
	// }

	return a;
}

void destroy_matrix(M *a) 
{
	free(a->value);
    free(a);
}

void copy_matrix(M *a, M *destination) 
{
	for(int i = 0; i < a->lines; i++) {
		for(int j = 0; j < a->columns; j++) {
			destination->value[i * a->columns + j] = a->value[i * a->columns + j];
		}
	}
}

int print_matrix(M *a, char *name)
{
	if(a == NULL) {
		return MATRIX_NOT_EXISTS;
	}

	printf("---->%s<-----------------------------------------------------\n", name);
	
	for(int i = 0; i < a->lines; i++) {
		for(int j = 0; j < a->columns; j++) {
		    printf("%lf ", a->value[i * a->columns + j]);
		}
		printf("\n");
	}

	return NO_ERROR;
}

bool isEqual(double a, double b)
{
	return fabs(a - b) < EPSILON;
}

bool compare_matrices(M *a, M *b)
{
	if(a->lines != b->lines)
		return false;
	if(a->columns != b->columns)
		return false;

	for(int i = 0; i < a->lines; i++) {
		for(int j = 0; j < a->columns; j++) {
			if(!isEqual(a->value[i * a->columns + j], b->value[i * a->columns + j])) return false;
		}
	}
	return true;
}

int get_matrix_size(M *a)
{
	return a->lines * a->columns * sizeof(double);
}