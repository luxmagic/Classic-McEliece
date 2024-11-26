#ifndef KEYGEN_H_
#define KEYGEN_H_
#include <stdbool.h>
#include <matrix.h>
#include <utility.h>
#include <config_files.h>

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