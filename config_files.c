#include <stdio.h>
#include <string.h>
#include "config_files.h"

#define ENCRYPTION_KEY 5.89804 

const parameters zero = {.k = 0, .n = 0, .t = 0};
const parameters low = {.k = 524, .n = 1024, .t = 50};
const parameters middle = {.k = 1751, .n = 2048, .t = 27};
const parameters hight = {.k = 5413, .n = 6960, .t = 119};

static config_file config;
static config_file *cf_ptr = &config;


config_file GetDefaultConfigFile(void)
{
    config.low = low;
    config.middle = middle;
    config.hight = hight;

    return config;
}

parameters ReadConfigFile(int level, config_file cf)
{
    switch (level)
    {
    case 1:
        printf("%i\t %i\t %i\n", cf.low.n, cf.low.k, cf.low.t);
        return cf.low;
        break;
    case 2:
        printf("%i\t %i\t %i\n", cf.middle.n, cf.middle.k, cf.middle.t);
        return cf.middle;
        break;
    case 3:
        printf("%i\t %i\t %i\n", cf.hight.n, cf.hight.k, cf.hight.t);
        return cf.hight;
        break;
    default:
        return zero;
        break;
    }
}

void WriteConfigFile(config_file cf, char* userName)
{
    char fileName[512] = "config_";
    char fileExt[10] = ".bin";
    strcat(fileName, userName);
    strcat(fileName, fileExt);
    WriteBinaryFile(fileName, cf);
}

void WriteBinaryFile(char* filename, config_file cf) 
{ 
    FILE* file = fopen(filename, "wb");
    if (file == NULL) 
    { 
        printf("Failed to open file for writing\n"); 
        return; 
    }

    fwrite(&cf, sizeof(config_file), 1, file); 
    fclose(file); 
}

bool ReadBinaryFile(char* filename, config_file *cf) 
{
    FILE* file = fopen(filename, "rb");
    if (file == NULL) 
    { 
        printf("Failed to open file for reading\n"); 
        return false;
    }

    fread(cf, sizeof(config_file), 1, file); 
    fclose(file);
    return true; 
} 

