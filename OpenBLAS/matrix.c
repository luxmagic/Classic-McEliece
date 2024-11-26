#include "matrix.h"
#include "utility.h"
#include "error.h"
#include <cblas.h>
#include <lapacke.h>

#define EPSILON 1e-40

double get_determinant(M *a)
{
    int n = a->columns;
    int m = a->lines;
    M *b = create_matrix(n, n);
    int* ipiv = (int*)malloc(n * sizeof(int)); // Массив для хранения индексов перестановок
    
    copy_matrix(a, b);

    // Выполняем LU-разложение
    int info = LAPACKE_dgetrf(LAPACK_COL_MAJOR, n, n, b->value, n, ipiv);
    if (info != 0) {
        printf("Error during LU, info = %d\n", info);
        free(ipiv);
        destroy_matrix(b);
        // free(temp_matrix);
        return 0;
    }

    // Вычисляем определитель
    double det = 1.0;
    for (int i = 0; i < n; i++) {
        det *= b->value[i * n + i];  // Умножаем диагональные элементы
        if (ipiv[i] != i + 1) {  // Если произошла перестановка
            det = -det;
        }
    }

    free(ipiv);
    destroy_matrix(b);
    return det;
}

M *get_inverse(M *a)
{
    int n = a->columns;
    int m = a->lines;
    M *b = create_matrix(n, n);
    int* ipiv = (int*)malloc(n * sizeof(int));
    if (ipiv == NULL) {
        printf("Error allocating memory\n");
        destroy_matrix(b);
        return NULL;
    }
    int info;
    copy_matrix(a, b);

    // LU разложение
    info = LAPACKE_dgetrf(LAPACK_ROW_MAJOR, n, n, b->value, n, ipiv);
    if (info != 0) {
        printf("Error during\n");
        free(ipiv);
        destroy_matrix(b);
        return NULL;
    }

    // Обратная матрица
    info = LAPACKE_dgetri(LAPACK_ROW_MAJOR, n, b->value, n, ipiv);
    if (info != 0) {
        printf("Error computing inverse matrix\n");
        free(ipiv);
        destroy_matrix(b);
        return NULL;
    }

    // printf("OK\n");
    free(ipiv);
    return b;
}

int get_rank(M *a) {
    // Создаем массив для хранения коэффициентов Q и R
    int m = a->lines;
    int n = a->columns;

    M *tau = create_matrix(1, n);
    M *R = create_matrix(n, n);
    
    // Выполняем QR-разложение
    LAPACKE_dgeqrf(LAPACK_ROW_MAJOR, m, n, a->value, n, tau->value);
    
    // Копируем элементы R из A
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (j < i) {
                R->value[i * n + j] = 0.0;
            } else {
                R->value[i * n + j] = a->value[i * n + j];
            }
        }
    }

    // Определяем ранг: количество ненулевых диагональных элементов R
    int rank = 0;
    for (int i = 0; i < n; i++) {
        if (fabs(R->value[i * n + i]) > EPSILON) {
            rank++;
        }
    }

    destroy_matrix(tau);
    destroy_matrix(R);
    
    return rank;
}

int matrix_multiply(M *a, M *b, M *c) 
{
    // Матрица C для хранения результата
    if (a->columns != b->lines) return -1;

    int m = a->lines;
    int k = a->columns;
    int n = b->columns;

    // Умножение матриц A и B, результат сохраняется в C
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                m, n, k,          // Размеры матриц
                1.0,              // Альфа
                a->value, k,     // Матрица A
                b->value, n,     // Матрица B
                0.0,              // Бета
                c->value, n);    // Матрица C

    return 0;
}

void scalar_multiply(M *a, double scalar) 
{
    for (int i = 0; i < a->lines; i++) {
        for (int j = 0; j < a->columns; j++) {
            a->value[i * a->columns + j] *= scalar;
        }
    }
}

void vector_mupliply(M *a, M *v) 
{
    int n = a->columns;
    int m = a->lines;

    M *v_out = create_matrix(m, 1);
    
    cblas_dgemv(CblasRowMajor, CblasNoTrans, 
                m, n, 
                1.0, 
                a->value, n, 
                v->value, 1, 
                0.0, 
                v_out->value, 1);

    destroy_matrix(v_out);
}