#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../include/types.h"

#include "buffers.h"
#include "haffman_tree.h"
#include "settings.h"
#include "utils.h"

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

TFrequenciesData count_file_frequencies(FILE *file)
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

void read_codes_recursion(TTreePoint *item, char *codeBuffer, int deep, TCode *dest)
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

TCode *read_codes_from_tree(TTreePoint *tree)
{
    TCode *codes = (TCode *)malloc(MAX_CODES_COUNT * sizeof(TCode));
    for (int i = 0; i < MAX_CODES_COUNT; i++)
        codes[i].isExist = 0;

    char codeBuffer[MAX_CODE_LENGTH + 1];
    read_codes_recursion(tree, codeBuffer, 0, codes);
    return codes;
}

void write_tree_recursion(FILE *ofile, TTreePoint *tree, TBinWriteBuffer *buffer)
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
        write_tree_recursion(ofile, tree->left, buffer);
    }
    if (tree->right != NULL)
    {
        write_tree_recursion(ofile, tree->right, buffer);
    }
}

void write_tree_to_file(FILE *ofile, TTreePoint *tree, int pointCount)
{
    uchar bytePointCount = (uchar)pointCount;
    write_byte_as_bin_to_file(ofile, bytePointCount);

    TBinWriteBuffer *writeBuffer = get_write_buffer(ofile, 1);
    write_tree_recursion(ofile, tree, writeBuffer);
    delete_write_buffer(writeBuffer);
}

void read_tree_recursion(TBinReadBuffer *readBuffer, TTreePoint *curPoint, int deep, int *codesCount)
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

TTreePoint *read_tree_from_file(FILE *ifile)
{
    uchar codesCountByte;
    fread(&codesCountByte, sizeof(uchar), 1, ifile);
    int codesCount = (int)codesCountByte;
    if (codesCount == 0)
        codesCount = MAX_CODES_COUNT;

    TBinReadBuffer *buffer = get_read_buffer(ifile);

    TTreePoint *tree = get_tree_point();
    tree->type = pointType;
    read_tree_recursion(buffer, tree, 0, &codesCount);

    delete_read_buffer(buffer);
    return tree;
}

TArchivatorData extract_file_data(FILE *srcFile)
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

void archivate_file_with_codes(FILE *ifile, FILE *ofile, TCode *codes)
{
    TBinWriteBuffer *write_buffer = get_write_buffer(ofile, 1);

    uchar byte;
    while (fread(&byte, sizeof(uchar), 1, ifile) != 0)
    {
        TCode code = codes[(int)byte];
        for (int i = 0; i < code.length; i++)
        {
            if (code.codeValue[i] == '1')
            {
                write_buffer_push(write_buffer, 1);
            }
            else
            {
                write_buffer_push(write_buffer, 0);
            }
        }
    }
    delete_write_buffer(write_buffer);
}

void dearchivate_file(FILE *archivePath, char destPath[])
{
    FILE *destFile = fopen(destPath, "wb");

    int fileLength;
    fread(&fileLength, sizeof(int), 1, archivePath);

    TTreePoint *root = read_tree_from_file(archivePath);
    TTreePoint *curPoint = root;

    TBinReadBuffer *readBuffer = get_read_buffer(archivePath);
    int charsCount = 0;
    while (charsCount != fileLength)
    {
        int bit = pop_bit_from_read_buffer(readBuffer);

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
            curPoint = root;
        }
    }
    fclose(destFile);
}

static void write_check_sum(FILE *dest, long checkSumPos, uint64_t checkSum)
{
    long curPos = ftell(dest);
    fseek(dest, checkSumPos - curPos, SEEK_CUR);
    fwrite(&checkSum, sizeof(uint64_t), 1, dest);
    fseek(dest, curPos - checkSumPos, SEEK_CUR);
}

TFileData archivate_file(char *sourcePath, char *serializedPath, FILE *archiveFile, TArchivatorResponse *respDest)
{
    TFileData result;
    result.path = sourcePath;
    result._isFreePathNeeded = false;

    FILE *srcFile = fopen(sourcePath, "rb");
    if (srcFile == NULL)
    {
        snprintf(respDest->errorMessage, ERROR_LENGTH, "Can't open file to read: %s", sourcePath);
        respDest->isError = true;
        return result;
    }

    TArchivatorData data = extract_file_data(srcFile);
    int fileLength = data.freqsData.fileLength;

    fwrite(&fileLength, sizeof(int), 1, archiveFile);
    write_string_to_file(archiveFile, serializedPath);

    long checkSumPos = ftell(archiveFile);
    fseek(archiveFile, sizeof(uint64_t), SEEK_CUR);

    uint64_t checkSum = 0;
    write_tree_to_file(archiveFile, data.codeTree, data.freqsData.codesCount);
    archivate_file_with_codes(srcFile, archiveFile, data.codes);
    write_check_sum(archiveFile, checkSumPos, checkSum);
    
    free(data.codes);
    free(data.freqsData.frequencies);
    delete_tree(data.codeTree);

    fclose(srcFile);

    return result;
}
