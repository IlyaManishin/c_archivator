#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "../include/types.h"
#include "../include/core_api.h"
#include "../include/pathlib.h"
#include "eta_progress.h"

#define MAX_PATH_PRINT_LENGTH 40
#define MAX_COMPRESSED_SIZE_LENGTH 20
#define MAX_BASE_SIZE_LENGTH 30
#define COLUMN_OFFSET 5
#define COLUMN_COUNT 3
#define DOTS_COUNT 3

typedef enum
{
    left,
    right
} EPrintDirection;

static int maxTerminalColumnSize;

static void print_string_column(char *s, int maxSize, bool withOffset, EPrintDirection dir)
{
    if (maxSize > maxTerminalColumnSize)
    {
        maxSize = maxTerminalColumnSize;
    }
    int length = strlen(s);
    if (withOffset)
    {
        printf("%*s", COLUMN_OFFSET, " ");
    }
    if (maxSize <= DOTS_COUNT)
    {
        printf("%*s", maxSize, ".");
    }
    else if (length > maxSize)
    {
        int startInd = length - maxSize + DOTS_COUNT;
        printf("...%s", s + startInd);
    }
    else
    {
        if (dir == right)
        {
            printf("%*s", maxSize, s);
        }
        else if (dir == left)
        {
            int whiteSpaceCount = maxSize - length;
            printf("%s%*s", s, whiteSpaceCount, " ");
        }
    }
}

static void print_size_column(uint64_t sizeBytes, int maxSize, bool withOffset, EPrintDirection dir)
{
    char s[100];
    long double sizeMB = ((long double)sizeBytes) / 1024 / 1024;
    snprintf(s, 100, "%" PRId64 " B (%.2Lf MB)", sizeBytes, sizeMB);
    print_string_column(s, maxSize, withOffset, dir);
}

static void print_header()
{
    char pathColumn[] = "Path";
    char comprSizeColumn[] = "Compressed size";
    char baseSizeColumn[] = "Base size";

    print_string_column(pathColumn, MAX_PATH_PRINT_LENGTH, false, left);
    print_string_column(comprSizeColumn, MAX_COMPRESSED_SIZE_LENGTH, true, right);
    print_string_column(baseSizeColumn, MAX_BASE_SIZE_LENGTH, true, right);
    printf("\n");
}

static void print_file_data(TFileData data, int index)
{
    if (index == 0)
    {
        print_header();
    }
    print_string_column(data.path, MAX_PATH_PRINT_LENGTH, false, left);
    print_size_column(data.compressSizeBytes, MAX_COMPRESSED_SIZE_LENGTH, true, right);
    print_size_column(data.baseSizeBytes, MAX_BASE_SIZE_LENGTH, true, right);
    printf("\n");
}

void get_archive_info(TSetupSettings *settings, TArchivatorResponse *respDest)
{
    char *archivePath = settings->archivePath;
    FILE *archiveFile = fopen(archivePath, "rb");
    if (archiveFile == NULL)
    {
        snprintf(respDest->errorMessage, ERROR_LENGTH, "Can't open archive: %s\n", archivePath);
        respDest->isError = true;
        return;
    }
    uint32_t filesCount;
    int res = fread(&filesCount, sizeof(uint32_t), 1, archiveFile);
    if (res == 0)
    {
        strcpy(respDest->errorMessage, "Invalid archive\n");
        respDest->isError = true;
        return;
    }
    TFileData sumData;
    sumData.path = "TOTAL:";
    sumData.baseSizeBytes = 0;
    sumData.compressSizeBytes = 0;

    int terminalWidth = get_terminal_width();
    maxTerminalColumnSize = (terminalWidth - COLUMN_OFFSET * (COLUMN_COUNT - 1)) / COLUMN_COUNT;

    for (int i = 0; i < filesCount; i++)
    {
        TFileData result = read_file_info(archiveFile, respDest);
        if (respDest->isError)
        {
            delete_file_data(result);
            goto exit;
        }
        if (!result.isValidCheckSum)
        {
            snprintf(respDest->errorMessage, ERROR_LENGTH, "Archive is spoiled: invalid check sum for file \"%s\"\n", result.path);
            respDest->isError = true;

            delete_file_data(result);
            goto exit;
        }
        if (settings->mode == infoMode)
        {
            sumData.baseSizeBytes += result.baseSizeBytes;
            sumData.compressSizeBytes += result.compressSizeBytes;
            print_file_data(result, i);
        }
    }
    if (settings->mode == checkMode)
    {
        printf("Archive status: OK\n");
    }
    if (settings->mode == infoMode)
    {
        print_file_data(sumData, filesCount);
    }

exit:
    fclose(archiveFile);
}