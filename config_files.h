#ifndef CONFIG_FILES_H
#define CONFIG_FILES_H
#include <pthread.h>
#include <stdbool.h>

typedef struct
{
    int n;
    int k;
    int t;
} parameters;

typedef struct
{
    parameters low;
    parameters middle;
    parameters hight;
} config_file;

typedef struct
{
    double keygen_time;
    double keygen_cycle;
    int keygen_using_memory;
    int key_size;
} methrics;

typedef struct
{
    parameters *cf;
    methrics KeyGenMethics;
    bool is_done;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} cf_threads;

config_file GetDefaultConfigFile(void);
parameters ReadConfigFile(int level, config_file cf);
void WriteConfigFile(config_file cf, char* userName);
void WriteBinaryFile(char* filename, config_file cf);
bool ReadBinaryFile(char* filename, config_file *cf); 
#endif