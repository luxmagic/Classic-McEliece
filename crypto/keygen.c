#include <Windows.h>
#include <psapi.h>

#include <cblas.h>
#include <lapacke.h>

#include "keygen.h"

#define d ((2*t-1) / 1)
#define ENCRYPTION_KEY 5.89804 

#define GF2 2

#define EPSILON 1e-40

void *GeneratePublicKey(void *arg)
{
    srand(time(NULL));
    cf_threads *config = (cf_threads*)arg;  
    int n = config->cf->n, k = config->cf->k, t = config->cf->t;
    
    M *G = create_matrix(k, n);
    M *S = create_matrix(k, k);
    M *P = create_matrix(n, n);
    M *G_ = create_matrix(k, n);
    M *KEY = create_matrix(k, n);

    clock_t start = clock();
    
    if (config == NULL) 
    {
        fprintf(stderr, "Ошибка: неверные данные в потоке!\n");
        return NULL;
    }

    pthread_mutex_lock(&config->mutex);

    config->KeyGenMethics.keygen_using_memory += get_matrix_size(G) + get_matrix_size(S) + get_matrix_size(P) + get_matrix_size(G_) + get_matrix_size(KEY);

    // Generate matrix G, S, P
    for (int i = 0; i < n; i++) 
    {
        for (int j = 0; j < n; j++) 
        {
            if (i < k && j < n) G->value[i * n + j] = (double)(rand() % GF2);    
            if (i < k && j < k) S->value[i * k + j] = (double)(rand() % GF2);
            P->value[i * n + j] = (double)(rand() % GF2);
        }
    }

    int rank = get_rank(S);
    static int cnt = 0;
    
    while (rank != k)
    {
        for (int i = 0; i < k; i++)
        {
            for (int j = 0; j < k; j++)
            { 
                S->value[i * k + j] = (double)(rand() % GF2);
            }
        }
        rank = get_rank(S);
        cnt++;
    }
    config->KeyGenMethics.keygen_using_memory += get_matrix_size(S) + (S->lines * sizeof(double));
    if (cnt != 0) config->KeyGenMethics.keygen_using_memory += cnt * (get_matrix_size(S) + (S->lines * sizeof(double)));
    printf("Check S matrix complete\n");

    // Generate key
    matrix_multiply(S, G, G_);
    matrix_multiply(G_, P, KEY);

    printf("Key generation complete\n");

    config->KeyGenMethics.keygen_using_memory += 6 * sizeof(int);

    destroy_matrix(S);
    destroy_matrix(G);
    destroy_matrix(P);
    destroy_matrix(G_);
    destroy_matrix(KEY);
    clock_t end = clock();

    config->KeyGenMethics.keygen_time = (double)(end - start) / (double)(CLOCKS_PER_SEC);
    config->KeyGenMethics.keygen_cycle = (double)(end - start);
    config->KeyGenMethics.keygen_using_memory += sizeof(clock_t);
    config->KeyGenMethics.key_size = get_matrix_size(KEY);

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