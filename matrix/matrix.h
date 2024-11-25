#ifndef MATRICI_H
#define MATRICI_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @M 
 * 
 * @brief The basic data structure used by this library to represent a matrix.
 *
 * @var M::line The number of lines of a matrix.
 * @var M::columns The number of columns of a matrix.
 * @var M::determinant The determinant of a matrix.
 * @var M::trace The trace of a matrix.
 * @var M::value Pointer to a dynamically alocated 2 dimensional array.
 * @var M::inverse Pointer to a M which represents the inverse
 *      of the current matrix.
 */
typedef struct M {
	uint16_t lines;
	uint16_t columns;

	uint16_t determinant;
	uint8_t trace;
			
	uint8_t **value;
	struct M *inverse;
} M;

/**
 * @brief Returns a pointer to a M with `rows` rows and `columns`
 *        columns. Note that the new matrix is empty, but it's elemnts are, by
 *        default 0.
 *
 * @param rows The number of rows of the matrix.
 * @param columns The number of columns of the matrix.
 *
 * @return M A pointer to an empty M.
 */
M *create_matrix(int rows, int columns);

/**
 * @brief Utility function used to read a matrix from the standard input.
 *
 * @param a The matrix in which the readed elements will be stored.
 *
 * @retrun void It doesn't return anything.
 */
void read_matrix(M *a);

/**
 * @brief utility function used to print a matrix to the standar output.
 *
 * @param a The matrix that should be printed.
 *
 * @return int If the matrix doesn't exists it will return MATRIX_NOT_EXISTS
 *         and NO_ERROR if the matrix was successfully printed.
 */
int print_matrix(M *a, char *name);

/**
 * @brief Returns the minorant of a matrix by a specific row and column.
 *
 * @param a The matrix that we want to compute the minorant of.
 * @param line The line That should be elimined when selecting the minorant.
 * @param colum The column that should be elimined when computing the minorant.
 *
 * @return M * It returns NULL or a pointer o to a M
 *         that represents the minorant of the matrix `a`. 
 */
M *get_minor(M *a, int line, int column);

/**
 * @brief Multiplies a matrix `a` with an scalar. Note that the new value of 
 *        the matrix will be the result of this operation.
 *
 * @param a The matrix that should be multiplied with an scalar.
 * @param scalar The scalar that should be multiplied withe the matrix `a`.
 *
 * @return void It doesn't return anything.
 */
void multiply_matrix_with_scalar(M *a, double scalar); 

/**
 * @brief Stores the transpose of the matrix `a` in the matrix `transpose`.
 *
 * @param a The matrix that the transpose of we will compute.
 * @param transpose This matrix will hold the value of the matrix `a`. Note
 *        that you should pass a reference to the matrix transpose. That's
 *        the way a call to this function would look like:
 *        `get_transpose(some_matrix, &transpose_of_a)`. Also, keep in mind
 *        that the matrix `transpose` doesn't need to be initialized, the
 *        function will do that for you;
 *
 * @return int CANT_CREATE_MATRIX if the `transpose` can't be created and 
 *         NO_ERROR if all wen ok.
 */
int get_transpose(M *a, M **transpose);

/**
 * @bried Dealocates the memory allocated for a matrix, after calling this
 *        function on a matrix all memory used for that matrix will be 
 *        dealocated, this includes, also, the memory alocate for it's inverse.
 *
 * @param a The matrix that should be destroyed.
 *
 * @return void It doesn't return anything.
 */
void destroy_matrix(M *a); 

/**
 * @brief Computes the inverse of the matrix `a`. Note that the inverse of
 *        matrix `a` will be stored in the `inverse` member of `a`. So, after
 *        calling this function you will access the inverse of `a` like this:
 *        `a->inverse`.
 *
 * @param a The matrix that we want to compute inverse of.
 *
 * @return int If the matrix doesn't have an inverse the NO_INVERSE error will
 *         be returned, otherwise NO_ERROR.
 */
int compute_inverse(M *a);

/**
 * @brief Returns the determinant of a matrix.
 *
 * @param a The matrix whose determinant will be computed.
 *
 * @return int The determinant of matrix `a`.
 */
int get_determinant(M *a);

/**
 * @brief Multiplies the matrices `a` and `b` and stores their results in the
 *        matrix `result`. Note that the function will alocate memory for the
 *        matrix `result`.
 *
 * @param a The matrix who will be multiplied with matrix `b`.
 * @param b The matrix wo will be multiplied with matrix `a`.
 *
 * @return int SIZE_NOT_MATCH if matrix `a` doesn't have the same number of 
 *         lines as the matrix `b`, CANT_CREATE_MATRIX if the function can't
 *         allocate memory for the matrix `result` and NO_ERROR if the matrices
 *         successully multiplied.
 */
int multiply_matrices(M *a, M *b, 
					  M **result);



/**
 * @brief Use this function to add 2 matrices. Note that the result will
 *        be stored in a third matrix.
 *
 * @param a An instance of M.
 * @param b An instance of M.
 * @param c An instance of M.
 *
 * @return int Returns and error status.
 *
 */
int add_matrices(M *a, M *b, M **result);

/**
 * @brief Copies the contents of the matrix `a` to the matrix `destinaation`.
 *
 * @param a The matrix whose contents we will copy.
 * @param destination The matrix where the contents of `a` will be stored. Note
 *        that the functio doesn't allocate memory for `destination` you      
 *        should create this matrix by yourself.
 *
 * @return void It doesn't return anything.
 */
void copy_matrix(M *a, M **destination); 

/**
 * @brief raise the matrix `a` to the power `power`.
 *
 * @param a The matrix that will be raised to the power `power`.
 * @param power The power that the matrix `a` will be raised to.
 *
 * @return M * Returns a matrix that will contain the result of
 *         raising `a` to `power`.
 */
M *matrix_pow(M *a, int power); 

/**
 * @brief Use this function to compare to matrices. The criteria
 *        used to compare the matrices are the dimensions and the
 *        actual values of the 2 matrices.
 *
 * @param a An instance of struct matrix.
 * @param b An instance of sruct martix.
 *
 * @return bool 1 if the matrices are equal and 0 otherwise.
 *
 */
bool compare_matrices(M *a, M *b);

/**
 * @brief Use this function to get the size of a matrix
 *
 * @param a An instance of struct matrix.
 *
 * @return int The size of the matrix
 *
 */
int get_matrix_size(M *a);

/**
 * @brief Use this function to get the rank of a matrix
 *
 * @param a An instance of struct matrix.
 *
 * @return uint16 The rank of the matrix
 *
 */
uint16_t compute_rank(M *a);

#endif 
