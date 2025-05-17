#ifndef ARCHIVATOR_TYPES_H
#define ARCHIVATOR_TYPES_H 1

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#define ERROR_LENGTH 256

typedef enum
{
    null,
    archivateMode,
    dearchivateMode,
    infoMode,
    checkMode,
    undefinedMode,
} EArchivatorMode;

typedef struct
{
    EArchivatorMode mode;

    char *dirToArchivate;
    int filesCount;
    char **filesToArchivate;

    char *archivePath;
    char *destDir;

    bool withInfo; 

    bool isError;
    char *errorMessage;
} TSetupSettings;

typedef struct
{
    bool isError;
    char *errorMessage;
} TArchivatorResponse;

typedef struct
{
    char* path;
    bool _isFreePathNeeded;

    uint64_t baseSizeBytes;
    uint64_t compressSizeBytes;
    bool isValidCheckSum;
} TFileData;

#endif