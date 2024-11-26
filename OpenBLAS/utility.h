#ifndef UTILITY_H
#define UTILITY_H

#include "matrix.h"

M *create_matrix(int lines, int columns);
void destroy_matrix(M *a);
void copy_matrix(M *a, M *destination);
int print_matrix(M *a, char *name);
bool compare_matrices(M *a, M *b);
int get_matrix_size(M *a);
bool isEqual(double a, double b);


#endif