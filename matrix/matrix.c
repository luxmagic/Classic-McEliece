#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <math.h>
#include "matrix.h"
#include "error.h"

M *get_minor(M *a, int line, int columns)
{
	M *minor = create_matrix(a->lines - 1, a->columns - 1);
	if(minor == NULL) {
		return NULL;
	}

	int x = 0, y = 0;
	for(int i = 0; i < a->lines; i++) {
		if(i == line)
			x--;
		for(int j = 0; j < a->columns; j++) {
			if(i != line && j != columns) {
				minor->value[x][y] = a->value[i][j];
				y++;
			}
		}
		x++;
		y = 0;
	}

	return minor;
}

void freeMatrix(M* a) {
    for (uint16_t i = 0; i < a->lines; i++) {
        free(a->value[i]);
    }
    free(a->value);
    free(a);
}

// int get_determinant(M *a)                                        
// {                                                                              
// 	int det;
// 	M *m;

// 	if (a->lines == 1 && a->columns == 1) {                                     
// 		return a->value[0][0];              
// 	}                                    
// 	else {                                                                      
// 	  	det = 0;                                                          
// 	  	for (int i = 0; i < a->columns; i++) {                                  
// 	    	m = get_minor(a, 0, i);                              
// 	      	det ^= (a->value[0][i]) & (i % 2 == 0 ? 1 : -1) & (get_determinant(m));    
// 	      	freeMatrix(m);                                                    
// 	  	}                                                                                                                           
// 	}
// 	return det;                                                                          
// }



int get_determinant(M *a)
{
   // Базовые случаи
    if (a->lines == 1) {
        return a->value[0][0];
    }
    if (a->lines == 2) {
        return a->value[0][0] * a->value[1][1] - a->value[0][1] * a->value[1][0];
    }

    uint16_t det = 0;
    for (uint16_t i = 0; i < a->lines; i++) {
        // Создание подтемпературы
        M* subMat = create_matrix(a->lines - 1, a->columns - 1);
        
        for (uint16_t j = 1; j < a->lines; j++) {
            for (uint16_t k = 0; k < a->columns; k++) {
                if (k < i) {
                    subMat->value[j - 1][k] = a->value[j][k];
                } else if (k > i) {
                    subMat->value[j - 1][k - 1] = a->value[j][k];
                }
            }
        }

        // Рекурсивно вычисляем определитель подтемпературы
        det ^= (i % 2 == 0 ? 1 : -1) & a->value[0][i] & get_determinant(subMat);
        freeMatrix(subMat);
    }

    return (int)det;
}



int get_transpose(M *a, M **transpose) 
{
	*transpose = create_matrix(a->lines, a->columns);
	if(*transpose == NULL) {
		return CANT_CREATE_MATRIX;
	}

	for(int i = 0; i < a->lines; i++) {
		for(int j = 0; j < a->columns; j++) {
			(*transpose)->value[i][j] = a->value[j][i];
		}
	}

	return NO_ERROR;
}


void multiply_matrix_with_scalar(M *a, double scalar) 
{
	for(int i = 0; i < a->lines; i++) {
		for(int j = 0; j < a->columns; j++) {
			a->value[i][j] *= scalar; 
		}
	}
}

int compute_inverse(M *a) 
{
	if(a->determinant == 0.0) {
		return NO_INVERSE;
	}

	M *transpose = create_matrix(a->lines, a->columns);
	M *adjugate = create_matrix(a->lines, a->columns);
	M *current_minor;

	get_transpose(a, &transpose);	
	for(int i = 0; i < transpose->lines; i++) {
		for(int j = 0; j < transpose->columns; j++) {
			current_minor         = get_minor(transpose, i, j);
			adjugate->value[i][j] = pow(-1.0, (double)i + (double)j) *
										get_determinant(current_minor);
			
			destroy_matrix(current_minor);
		}
	}
	destroy_matrix(transpose);
	
	a->inverse = adjugate;
	multiply_matrix_with_scalar(a->inverse, 1/a->determinant);
	
	return NO_ERROR;
}

int multiply_matrices(M *a, M *b, M **result)
{
	if(a->columns != b->lines) {
		printf("<--- A col not equal B lines --->\n");
		return SIZE_NOT_MATCH;
	}

	*result = create_matrix(a->lines, b->columns);
	if(*result == NULL) {
		printf("<--- Matrix can`t be created --->\n");
		return CANT_CREATE_MATRIX; 
	}

    int x  = 0, y = 0;
    for(int i = 0; i < a->lines; i++) {
        y = 0;
        for(int j = 0; j < b->columns; j++) {
            for(int k = 0; k < a->columns; k++) {
                (*result)->value[x][y] ^= (a->value[i][k] & b->value[k][j]);
            }
            y++; 
        }
        x++;
    }

	return NO_ERROR;
}

M *matrix_pow(M *a, int power) 
{
	M *result = create_matrix(a->lines, a->columns);
	copy_matrix(a, &result);

	for(int i = 0; i < power-1; i++) {
		multiply_matrices(result, a, &result);
	}

	return result;
}

int add_matrices(M *a, M *b, M **result)
{
	if(a->lines != b->lines && a->columns != b->columns) {
		return NOT_SAME_SIZE;
	}

	*result = create_matrix(a->lines, a->columns);
	
	for(int i = 0; i < a->lines; i++) {
		for(int j = 0; j < a->columns; j++) {
			(*result)->value[i][j] = a->value[i][j] + b->value[i][j];
		}
	}

	return NO_ERROR;
}

int compute_trace(M *a)
{
	if(a->columns != a->lines) {
		return NOT_SQUARE_MATRIX; //error
	}

	for(int i = 0; i < a->columns; i++) {
		a->trace += a->value[i][i];
	}

	return NO_ERROR;
}

int get_matrix_size(M *a)
{
	return a->lines * a->columns * sizeof(uint8_t);
}

// Функция для обмена двух строк в матрице
void swap_rows(uint8_t **matrix, uint16_t row1, uint16_t row2, uint16_t columns) {
    for (uint16_t j = 0; j < columns; j++) {
        uint8_t temp = matrix[row1][j];
        matrix[row1][j] = matrix[row2][j];
        matrix[row2][j] = temp;
    }
}

// Функция для подсчета ранга матрицы
uint16_t compute_rank(M *a) {
    uint16_t rank = 0;
    uint16_t lines = a->lines;
    uint16_t columns = a->columns;

    M *matrix = create_matrix(lines, columns);
	copy_matrix(a, &matrix);
	

    for (uint16_t col = 0; col < columns; col++) {
        // Находим строку с ненулевым элементом в текущем столбце
        uint16_t row = rank;
        while (row < lines && matrix->value[row][col] == 0) {
            row++;
        }

        if (row == lines) {
            // Если мы не нашли ненулевой элемент, переходим к следующему столбцу
            continue;
        }

        // Меняем текущую строку rank с найденной строкой row
        if (row != rank) {
            swap_rows(matrix->value, rank, row, columns);
        }

        // Теперь обнуляем другие строки
        for (uint16_t i = 0; i < lines; i++) {
            if (i != rank && matrix->value[i][col] == 1) {
                for (uint16_t j = 0; j < columns; j++) {
                    matrix->value[i][j] ^= matrix->value[rank][j]; // XOR для обнуления
                }
            }
        }

        // Увеличиваем ранг
        rank++;
    }

    // Освобождаем память
    for (uint16_t i = 0; i < lines; i++) {
        free(matrix->value[i]);
    }
    free(matrix);

    return rank;
}