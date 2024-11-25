#include <stdio.h>
#include <stdlib.h>

#include <Windows.h>
#include <psapi.h>

#include <time.h>
#include <math.h>

#include "keygen.h"
#include "matrix.h"

#define d ((2*t-1) / 1)
#define ENCRYPTION_KEY 5.89804 

#define GF2 2

volatile double keygen_time = 0;
volatile double keygen_cycle = 0;
volatile int keygen_using_memory = 0;
volatile int key_size = 0;


void *GeneratePublicKey(void *arg)
{
    cf_threads *config = (cf_threads*)arg;    
    int n = config->cf->n, k = config->cf->k, t = config->cf->t;

    clock_t start = clock();

    if (config == NULL) 
    {
        fprintf(stderr, "Ошибка: неверные данные в потоке!\n");
        return NULL;
    }

    pthread_mutex_lock(&config->mutex);
    srand(time(NULL));
    M *G = create_matrix(k, n);
    M *S = create_matrix(k, k);
    M *P = create_matrix(n, n);
    M *GS, *G_;
    
    config->KeyGenMethics.keygen_using_memory += get_matrix_size(G) + get_matrix_size(S) + get_matrix_size(P);

    if(G == NULL || S == NULL || P == NULL) 
    {
        errno = ENOMEM;
        return NULL;
    }
    printf("Create Matrix G, S, P\n");
    // Generate matrix G, S, P
    for (int i = 0; i < n; i++) 
    {
        for (int j = 0; j < n; j++) 
        {
            if (i < k && j < n) G->value[i][j] = rand() % GF2;    
            if (i < k && j < k) S->value[i][j] = rand() % GF2;
            P->value[i][j] = rand() % GF2;
        }
    }

    printf("Have Matrix G, S, P\n");
    config->KeyGenMethics.keygen_using_memory += sizeof(int) * 5;

    //check Matrix S is invertible
    int det = get_determinant(S);
    uint16_t rank = compute_rank(S);
    
    while (rank != k)
    {
        // destroy_matrix(S);
        // S = create_matrix(k, k);
        for (int i = 0; i < k; i++) 
        {
            for (int j = 0; j < k; j++) 
            { 
                S->value[i][j] = rand() % GF2;
            }
        }
        rank = compute_rank(S);
    }
    printf("Check Matrix S\n");
    config->KeyGenMethics.keygen_using_memory += sizeof(int) * 2;
    printf("%d\n", rank);

    // Generate matrix key
    multiply_matrices(S, G, &GS);
    multiply_matrices(GS, P, &G_);

    printf("Created Key\n");

    config->KeyGenMethics.keygen_using_memory += get_matrix_size(GS) + get_matrix_size(G_);

    // print_matrix(G, "G");
    // print_matrix(S, "S");
    // print_matrix(P, "P");
    // print_matrix(G_, "G_");

    destroy_matrix(S);
    destroy_matrix(P);
    destroy_matrix(GS);
    destroy_matrix(G_);

    clock_t end = clock();
    config->KeyGenMethics.keygen_time = (double)(end - start) / (double)(CLOCKS_PER_SEC);
    config->KeyGenMethics.keygen_cycle = (double)(end - start);
    config->KeyGenMethics.keygen_using_memory += sizeof(clock_t);
    config->KeyGenMethics.key_size = get_matrix_size(G_);

    config->is_done = true;
    pthread_cond_signal(&config->cond);
    pthread_mutex_unlock(&config->mutex);
    return NULL;
}

int GetUsingProcessMemory(void)
{
    HANDLE hProcess = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc));
    return pmc.WorkingSetSize;
}