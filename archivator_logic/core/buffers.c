#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "buffers.h"
#include "settings.h"

#define MAX_STRING_LENGTH 4096

TBinWriteBuffer *get_write_buffer(FILE *ofile)
{
    TBinWriteBuffer *buffer = (TBinWriteBuffer *)malloc(sizeof(TBinWriteBuffer));
    buffer->length = 0;
    buffer->byte = 0;
    buffer->destFile = ofile;

    buffer->checkSum = 0;
    buffer->bytesCount = 0;
    return buffer;
}

static void write_byte_as_bin_to_file(FILE *destFile, char byte)
{
    fwrite(&byte, sizeof(char), 1, destFile);
}

void flash_write_buffer(TBinWriteBuffer *buffer)
{
    if (buffer->length == 0)
    {
        return;
    }
    for (int i = buffer->length; i < BYTE_LENGTH; i++)
    {
        buffer->byte = buffer->byte << 1;
    }
    write_byte_as_bin_to_file(buffer->destFile, buffer->byte);
    buffer->bytesCount += 1;
    buffer->length = 0;
}

void write_buffer_push(TBinWriteBuffer *buffer, int bit)
{
    if (bit)
    {
        buffer->checkSum++;
    }
    if (buffer->length == BYTE_LENGTH)
    {
        flash_write_buffer(buffer);
        buffer->byte = bit;
        buffer->length = 1;
        return;
    }
    buffer->byte = (buffer->byte << 1) + bit;
    buffer->length++;
}

void buffer_write_arg(TBinWriteBuffer *buffer, void *arg_ptr, size_t size)
{
    uchar *byte = (uchar *)arg_ptr;
    for (int byteInd = 0; byteInd < size; byteInd++)
    {
        for (int i = 0; i < BYTE_LENGTH; i++)
        {
            if (((*byte) >> i) & 1)
            {
                buffer->checkSum++;
            }
        }
        byte++;
    }
    buffer->bytesCount += size;
    fwrite(arg_ptr, size, 1, buffer->destFile);
}

void buffer_write_string(TBinWriteBuffer *buffer, char *s)
{
    int length = strlen(s) + 1;
    buffer_write_arg(buffer, s, length);
}

void flash_read_buffer(TBinReadBuffer *buffer)
{
    buffer->length = 0;
}

void delete_write_buffer(TBinWriteBuffer *buffer)
{
    flash_write_buffer(buffer);
    free(buffer);
}

TBinReadBuffer *get_read_buffer(FILE *ifile)
{
    TBinReadBuffer *buffer = (TBinReadBuffer *)malloc(sizeof(TBinReadBuffer));
    buffer->readFile = ifile;
    buffer->bytesCount = 0;
    buffer->length = 0;
    buffer->checkSum = 0;

    return buffer;
}

int pop_bit_from_read_buffer(TBinReadBuffer *buffer)
{
    if (buffer->length == 0)
    {
        int res = fread(&buffer->byte, sizeof(char), 1, buffer->readFile);
        if (res == 0)
            return -1;

        buffer->length = BYTE_LENGTH;
        buffer->bytesCount++;
    }

    int bit = (buffer->byte >> (buffer->length - 1)) & 1;
    if (bit)
    {
        buffer->checkSum++;
    }
    buffer->length--;

    return bit;
}

char *buffer_read_string(TBinReadBuffer *readBuffer)
{
    int baseBufferLength = 32;
    int capacity = baseBufferLength;
    char *stringBuffer = malloc(capacity);
    if (!stringBuffer)
        return NULL;

    char ch;
    int length = 0;
    while (fread(&ch, sizeof(char), 1, readBuffer->readFile) != 0)
    {
        readBuffer->bytesCount++;

        stringBuffer[length] = ch;
        length++;
        if (length > MAX_STRING_LENGTH)
        {
            free(stringBuffer);
            return NULL;
        }

        for (int i = 0; i < BYTE_LENGTH; i++)
        {
            if (((int)ch >> i) & 1)
            {
                readBuffer->checkSum++;
            }
        }
        if (ch == '\0')
        {
            return stringBuffer;
        }

        if (length >= capacity)
        {
            capacity *= 2;
            char *newBuffer = realloc(stringBuffer, capacity);
            if (!newBuffer)
            {
                free(stringBuffer);
                return NULL;
            }
            stringBuffer = newBuffer;
        }
    }
    free(stringBuffer);
    return NULL;
}

size_t buffer_read_arg(TBinReadBuffer *buffer, void *arg, size_t size)
{
    buffer->bytesCount += size;
    return fread(arg, size, 1, buffer->readFile);
}

void delete_read_buffer(TBinReadBuffer *buffer)
{
    free(buffer);
}