#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "buffers.h"
#include "settings.h"

TBinWriteBuffer *get_write_buffer(FILE *ofile)
{
    TBinWriteBuffer *buffer = (TBinWriteBuffer *)malloc(sizeof(TBinWriteBuffer));
    buffer->length = 0;
    buffer->byte = 0;
    buffer->destFile = ofile;

    buffer->checkSum = 0;
    buffer->bitsCount = 0;
    return buffer;
}

static void write_byte_as_bin_to_file(FILE *destFile, char byte)
{
    fwrite(&byte, sizeof(char), 1, destFile);
}

void flash_write_buffer(TBinWriteBuffer *buffer)
{
    for (int i = buffer->length; i < BYTE_LENGTH; i++)
    {
        buffer->byte = buffer->byte << 1;
    }
    write_byte_as_bin_to_file(buffer->destFile, buffer->byte);
    buffer->bitsCount += BYTE_LENGTH;
    buffer->length = 0;
}

void write_buffer_push(TBinWriteBuffer *buffer, int bit)
{
    if (buffer->length == BYTE_LENGTH)
    {
        flash_write_buffer(buffer);
        buffer->byte = bit;
        buffer->length = 1;
        return;
    }
    buffer->byte = (buffer->byte << 1) + bit;
    if (bit)
    {
        buffer->checkSum++;
    }
    buffer->length++;
}

long write_buffer_ftell(TBinWriteBuffer *buffer)
{
    return ftell(buffer->destFile);
}

void write_buffer_fseek_cur(TBinWriteBuffer *buffer, long pos)
{
    fseek(buffer->destFile, pos, SEEK_CUR);
}

void buffer_write_arg(TBinWriteBuffer *buffer, void *arg_ptr, size_t size)
{
    int numSizeBits = size * BYTE_LENGTH;
    int numCheckSum = 0;

    uchar *byte = (uchar *)arg_ptr;
    for (int byteInd = 0; byteInd < size; byteInd++)
    {
        for (int i = 0; i < BYTE_LENGTH; i++)
        {
            if (((*byte) >> i) & 1)
            {
                numCheckSum++;
            }
        }
        byte++;
    }

    buffer->checkSum += numCheckSum;
    buffer->bitsCount += numSizeBits;

    fwrite(arg_ptr, size, 1, buffer->destFile);
}

void buffer_write_string(TBinWriteBuffer *buffer, char *s)
{
    int length = strlen(s);
    buffer_write_arg(buffer, s, length);
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

    int res = fread(&buffer->byte, sizeof(char), 1, ifile);
    if (res == 0)
    {
        buffer->length = 0;
    }
    else
    {
        buffer->length = BYTE_LENGTH;
    }

    return buffer;
}

int pop_bit_from_read_buffer(TBinReadBuffer *buffer)
{
    if (buffer->length == 0)
    {
        int res = fread(&buffer->byte, sizeof(char), 1, buffer->readFile);
        if (res == 0)
            return 0;

        buffer->length = BYTE_LENGTH;
    }

    int bit = (buffer->byte >> (buffer->length - 1)) & 1;
    buffer->length--;

    return bit;
}

void delete_read_buffer(TBinReadBuffer *buffer)
{
    free(buffer);
}