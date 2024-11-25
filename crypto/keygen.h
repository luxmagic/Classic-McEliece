#ifndef KEYGEN_H_
#define KEYGEN_H_
#include <stdbool.h>
#include <matrix.h>
#include <config_files.h>

extern volatile double keygen_time;
extern volatile double keygen_cycle;
extern volatile int keygen_using_memory;
extern volatile int key_size;

typedef struct 
{
    int t;
    M *public_key;
} pubKey;

typedef struct
{
    int t;
    M *S;
    M *G;
    M *P;
} priKey;

void *GeneratePublicKey(void *arg);
int GetUsingProcessMemory(void);

#endif