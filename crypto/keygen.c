#include <stdio.h>
#include "keygen.h"
#include "matrix.h"



int run(void)
{
    M *foo = create_matrix(2, 2);
    foo->value[1][0] = 3;
    multiply_matrix_with_scalar(foo, 2);
    print_matrix(foo);   
    destroy_matrix(foo);
    printf("matrix!!!!\n");
    return 0;
}