#ifndef CONFIG_FILES_H
#define CONFIG_FILES_H
#include <pthread.h>
#include <stdbool.h>

typedef struct cf
{
    int n;
    int k;
    int t;
    int m;
} parameters;

typedef struct
{
    parameters low;
    parameters middle;
    parameters hight;
} config_file;


typedef struct
{
    double time;
    double cycle;
    int using_memory;
    int one_obj_size;
    int two_obj_size;
} methrics;


config_file GetDefaultConfigFile(void);
parameters ReadConfigFile(int level, config_file cf);
void WriteConfigFile(config_file cf, char* userName);
void WriteBinaryFile(char* filename, config_file cf);
bool ReadBinaryFile(char* filename, config_file *cf); 
#endif