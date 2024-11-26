#ifndef FILE_HANDLE_H
#define FILE_HANDLE_H

#include <stdio.h>
#include <stdlib.h>

// Загрузка текстового файла
char* LoadTextFile(const char* fileName)
{
    FILE* file = fopen(fileName, "r");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    int fileSize = ftell(file);
    rewind(file);

    char* fileContent = (char*)malloc(fileSize + 1);
    fread(fileContent, fileSize, 1, file);
    fileContent[fileSize] = '\0';

    fclose(file);
    return fileContent;
}

// Загрузка двоичного файла
unsigned char* LoadBinaryFile(const char* fileName, int* fileSize)
{
    FILE* file = fopen(fileName, "rb");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file);
    rewind(file);

    unsigned char* fileContent = malloc(*fileSize);
    fread(fileContent, *fileSize, 1, file);

    fclose(file);
    return fileContent;
}

void FileParsing(GuiWindowFileDialogState * const state, char * const fileName, int * const fileSize, char ** fileContent)
{
    strcpy(fileName, TextFormat("%s" PATH_SEPERATOR "%s", state->dirPathText, state->fileNameText));
    // strncpy(fileNameToLoad, fileDialogState.fileNameText, 512 - 1);
    // fileNameToLoad[MAX_FILE_NAME_LENGTH - 1] = '\0'; // Ensure null-termination
    *fileSize = GetFileLength(fileName);
    state->SelectFilePressed = false;
    if (strcmp(state->filterExt, ".txt") == 0)
    {
        *fileContent = LoadTextFile(fileName);
    }
    else if (strcmp(state->filterExt, ".bin") == 0)
    {
        *fileContent = (void *)LoadBinaryFile(fileName, fileSize);
    }
    else if (strcmp(state->filterExt, ".hex") == 0)
    {
        *fileContent = (void *)LoadBinaryFile(fileName, fileSize);
    }
}

#endif