#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../include/types.h"
#include "../include/pathlib.h"

#include "buffers.h"
#include "haffman_tree.h"
#include "settings.h"

#define uchar unsigned char

typedef struct
{
    uint64_t fileLength;
    int codesCount;
    uint64_t *frequencies;
} TFrequenciesData;

typedef struct
{
    bool isExist;
    int length;
    char *codeValue;
} TCode;

typedef struct
{
    TTreePoint *codeTree;
    TCode *codes;

    TFrequenciesData freqsData;
} TArchivatorData;

typedef struct
{
    uint64_t checkSum;
    uint64_t compressSizeBytes;
    uint64_t baseSizeBytes;

    int isValid;
} THeaderData;

static TFrequenciesData count_file_frequencies(FILE *file)
{
    TFrequenciesData freqsData;
    freqsData.fileLength = 0;
    freqsData.codesCount = 0;

    freqsData.frequencies = (uint64_t *)malloc(MAX_CODES_COUNT * sizeof(uint64_t));
    for (int i = 0; i < BYTE_SIZE; i++)
        freqsData.frequencies[i] = 0;

    uchar value;
    while (fread(&value, sizeof(uchar), 1, file) != 0)
    {
        freqsData.frequencies[value]++;
        freqsData.fileLength++;
    }

    for (int i = 0; i < MAX_CODES_COUNT; i++)
    {
        if (freqsData.frequencies[i] != 0)
            freqsData.codesCount++;
    }
    return freqsData;
}

static void read_codes_recursion(TTreePoint *item, char *codeBuffer, int deep, TCode *dest)
{
    if (item == NULL)
        return;
    if (item->type == leafType)
    {
        int value = item->value;
        char *code = (char *)malloc((deep + 1) * sizeof(char));
        codeBuffer[deep] = '\0';
        strcpy(code, codeBuffer);

        dest[value].length = deep;
        dest[value].codeValue = code;
        dest[value].isExist = 1;
        return;
    }
    codeBuffer[deep] = '0';
    read_codes_recursion(item->left, codeBuffer, deep + 1, dest);
    codeBuffer[deep] = '1';
    read_codes_recursion(item->right, codeBuffer, deep + 1, dest);
}

static TCode *read_codes_from_tree(TTreePoint *tree)
{
    TCode *codes = (TCode *)malloc(MAX_CODES_COUNT * sizeof(TCode));
    for (int i = 0; i < MAX_CODES_COUNT; i++)
        codes[i].isExist = 0;

    char codeBuffer[MAX_CODE_LENGTH + 1];
    read_codes_recursion(tree, codeBuffer, 0, codes);
    return codes;
}

void delete_file_data(TFileData data)
{
    if (data._isFreePathNeeded)
    {
        free(data.path);
    }
}

static void write_tree_recursion(TTreePoint *tree, TBinWriteBuffer *buffer)
{
    if (tree->type == leafType)
    {
        write_buffer_push(buffer, 1);
        uchar value = tree->value;
        for (int i = BYTE_LENGTH - 1; i >= 0; i--)
        {
            if ((value >> i) & 1)
            {
                write_buffer_push(buffer, 1);
            }
            else
            {
                write_buffer_push(buffer, 0);
            }
        }
        return;
    }

    write_buffer_push(buffer, 0);
    if (tree->left != NULL)
    {
        write_tree_recursion(tree->left, buffer);
    }
    if (tree->right != NULL)
    {
        write_tree_recursion(tree->right, buffer);
    }
}

static void write_tree_to_file(TBinWriteBuffer *buffer, TTreePoint *tree, int pointCount)
{
    uchar bytePointCount = (uchar)pointCount;
    buffer_write_arg(buffer, &bytePointCount, sizeof(uchar));

    write_tree_recursion(tree, buffer);
    flash_write_buffer(buffer);
}

static void read_tree_recursion(TBinReadBuffer *readBuffer, TTreePoint *curPoint, int deep, int *codesCount)
{
    if (*codesCount == 0)
        return;

    int curBit = pop_bit_from_read_buffer(readBuffer);
    if (curBit == 1)
    {
        int pointValue = 0;
        for (int i = 0; i < 8; i++)
        {
            int bit = pop_bit_from_read_buffer(readBuffer);
            pointValue = (pointValue << 1) + bit;
        }
        curPoint->type = leafType;
        curPoint->value = pointValue;

        (*codesCount)--;
        return;
    }
    curPoint->left = get_tree_point();
    curPoint->left->type = pointType;
    read_tree_recursion(readBuffer, curPoint->left, deep + 1, codesCount);

    curPoint->right = get_tree_point();
    curPoint->right->type = pointType;
    read_tree_recursion(readBuffer, curPoint->right, deep + 1, codesCount);
}

static TTreePoint *read_tree_from_file(TBinReadBuffer *buffer)
{
    uchar codesCountByte;
    size_t res = buffer_read_arg(buffer, &codesCountByte, sizeof(uchar));
    if (res == 0)
    {
        return NULL;
    }
    int codesCount = (int)codesCountByte;
    if (codesCount == 0)
        codesCount = MAX_CODES_COUNT;

    TTreePoint *tree = get_tree_point();
    tree->type = pointType;
    read_tree_recursion(buffer, tree, 0, &codesCount);

    delete_read_buffer(buffer);
    return tree;
}

static TArchivatorData extract_file_data(FILE *srcFile)
{
    TFrequenciesData freqsData = count_file_frequencies(srcFile);
    TTreeArr *treeArr = get_tree_arr_by_freq(freqsData.frequencies, freqsData.codesCount);

    while (treeArr->length != 1)
    {
        TTreePoint *minItem1 = tree_arr_pop(treeArr);
        TTreePoint *minItem2 = tree_arr_pop(treeArr);

        TTreePoint *merged = (TTreePoint *)malloc(sizeof(TTreePoint));
        merged->freq = minItem1->freq + minItem2->freq;
        merged->left = minItem2;
        merged->right = minItem1;
        merged->type = treeType;

        tree_arr_min_append(treeArr, merged);
    }

    TCode *codes = read_codes_from_tree(treeArr->items[0]);

    TArchivatorData data;
    data.codes = codes;
    data.codeTree = treeArr->items[0];
    data.freqsData = freqsData;
    free(treeArr->items);
    free(treeArr);
    fseek(srcFile, 0, SEEK_SET);

    return data;
}

void archivate_file_with_codes(FILE *ifile, TBinWriteBuffer *writeBuffer, TCode *codes)
{
    uchar byte;
    while (fread(&byte, sizeof(uchar), 1, ifile) != 0)
    {
        TCode code = codes[(int)byte];
        for (int i = 0; i < code.length; i++)
        {
            if (code.codeValue[i] == '1')
            {
                write_buffer_push(writeBuffer, 1);
            }
            else
            {
                write_buffer_push(writeBuffer, 0);
            }
        }
    }
    flash_write_buffer(writeBuffer);
}

static THeaderData read_header_data(FILE *archive)
{
    THeaderData result;
    uint64_t *fields[] = {&result.checkSum, &result.compressSizeBytes, &result.baseSizeBytes};

    for (int i = 0; i < 3; ++i)
    {
        if (fread(fields[i], sizeof(uint64_t), 1, archive) != 1)
        {
            result.isValid = false;
            return result;
        }
    }

    result.isValid = true;
    return result;
}

static bool dearchivate_file_with_tree(TBinReadBuffer *readBuffer, TTreePoint *tree, FILE *destFile, uint64_t fileLength)
{
    TTreePoint *curPoint = tree;

    uint64_t charsCount = 0;
    while (charsCount != fileLength)
    {
        int bit = pop_bit_from_read_buffer(readBuffer);
        if (bit == -1)
        {
            return false;
        }
        if (curPoint->type != leafType)
        {
            if (bit == 0)
            {
                curPoint = curPoint->left;
            }
            else
            {
                curPoint = curPoint->right;
            }
        }
        if (curPoint->type == leafType)
        {
            char code = curPoint->value;
            fwrite(&code, sizeof(char), 1, destFile);

            charsCount++;
            curPoint = tree;
        }
    }
    return true;
}

TFileData dearchivate_file(FILE *archiveFile, char* absDestDir, TArchivatorResponse *errorDest)
{
    TFileData result;
    int destDirLength = strlen(absDestDir);

    THeaderData headerData = read_header_data(archiveFile);
    if (!headerData.isValid)
    {
        goto invalid_archive_error;
    }
    TBinReadBuffer *readBuffer = get_read_buffer(archiveFile);

    char *destRelPath = buffer_read_string(readBuffer);
    if (destRelPath == NULL)
    {
        goto invalid_archive_error;
    }
    char* destPath = path_concat(absDestDir, destRelPath, SERIALIZE_SEP);
    printf("%s\n\n", destPath);
    TTreePoint *tree = read_tree_from_file(readBuffer);
    if (tree == NULL)
    {
        goto invalid_archive_error;
    }

    int res = create_dirs_for_file(destPath);
    if (res == -1)
    {
        delete_tree(tree);
        goto invalid_file_path_error;
    }
    char *freePath = get_free_file_path(destPath);
    if (freePath == NULL)
    {
        delete_tree(tree);
        goto invalid_file_path_error;
    }

    FILE *destFile = fopen(freePath, "wb");
    if (destFile == NULL){
        delete_tree(tree);
        goto invalid_file_path_error;
    }
    bool isSuccess = dearchivate_file_with_tree(readBuffer, tree, destFile, headerData.compressSizeBytes);
    if (!isSuccess)
    {
        remove(destRelPath);
        delete_tree(tree);
        fclose(destFile);
        goto invalid_archive_error;
    }
    fclose(destFile);

    result.path = freePath;
    result.baseSizeBytes = headerData.baseSizeBytes;
    result.compressSizeBytes = headerData.compressSizeBytes;
    return result;

invalid_archive_error:
    strcpy(errorDest->errorMessage, "Invalid archive");
    errorDest->isError = true;
    return result;

invalid_file_path_error:
    snprintf(errorDest->errorMessage, ERROR_LENGTH, "Can't save file: %s", destRelPath);
    errorDest->isError = true;
    return result;
}

static void write_header_data(TBinWriteBuffer *buffer, THeaderData data, long headerPos)
{
    long endPos = write_buffer_ftell(buffer);
    write_buffer_fseek_cur(buffer, headerPos - endPos);
    buffer_write_arg(buffer, &data.checkSum, sizeof(uint64_t));
    buffer_write_arg(buffer, &data.compressSizeBytes, sizeof(uint64_t));
    buffer_write_arg(buffer, &data.baseSizeBytes, sizeof(uint64_t));

    long curPos = write_buffer_ftell(buffer);
    write_buffer_fseek_cur(buffer, endPos - curPos);
}

TFileData archivate_file(char *sourcePath, char *serializedPath, FILE *archiveFile, TArchivatorResponse *errorDest)
{
    TFileData result;
    result.path = sourcePath;
    result._isFreePathNeeded = false;
    result.compressSizeBytes = 0;

    FILE *srcFile = fopen(sourcePath, "rb");
    if (srcFile == NULL)
    {
        snprintf(errorDest->errorMessage, ERROR_LENGTH, "Can't open file to read: %s", sourcePath);
        errorDest->isError = true;
        return result;
    }

    TArchivatorData data = extract_file_data(srcFile);
    uint64_t fileLength = data.freqsData.fileLength;

    TBinWriteBuffer *writeBuffer = get_write_buffer(archiveFile);

    long headerDataPos = write_buffer_ftell(writeBuffer);
    size_t headerDataOffset = 3 * sizeof(uint64_t);
    write_buffer_fseek_cur(writeBuffer, headerDataOffset);

    buffer_write_string(writeBuffer, serializedPath);

    write_tree_to_file(writeBuffer, data.codeTree, data.freqsData.codesCount);
    archivate_file_with_codes(srcFile, writeBuffer, data.codes);

    THeaderData headerData;
    headerData.checkSum = writeBuffer->checkSum;
    headerData.compressSizeBytes = writeBuffer->bytesCount;
    headerData.baseSizeBytes = fileLength;
    write_header_data(writeBuffer, headerData, headerDataPos);

    result.compressSizeBytes = writeBuffer->bytesCount;
    result.baseSizeBytes = fileLength;

    free(data.codes);
    free(data.freqsData.frequencies);
    delete_tree(data.codeTree);
    fclose(srcFile);

    return result;
}
