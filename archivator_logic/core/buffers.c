#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "buffers.h"
#include "settings.h"


TBinWriteBuffer *get_write_buffer(FILE *ofile, int isBinWriteMode)
{
    TBinWriteBuffer *buffer = (TBinWriteBuffer *)malloc(sizeof(TBinWriteBuffer));
    buffer->length = 0;
    buffer->byte = 0;
    buffer->destFile = ofile;
    buffer->isBinWriteMode = isBinWriteMode;
    return buffer;
}

void write_byte_as_bin_to_file(FILE *destFile, char byte)
{
    fwrite(&byte, sizeof(char), 1, destFile);
}

void write_byte_as_text_to_file(FILE *destFile, char byte)
{
    for (int i = BYTE_LENGTH - 1; i >= 0; i--)
    {
        fprintf(destFile, "%d", (int)((byte >> i) & 1));
    }
}

void write_buffer_push(TBinWriteBuffer *buffer, int bit)
{
    if (buffer->length == 8)
    {
        if (buffer->isBinWriteMode)
        {
            write_byte_as_bin_to_file(buffer->destFile, buffer->byte);
        }
        else
        {
            write_byte_as_text_to_file(buffer->destFile, buffer->byte);
        }
        buffer->byte = bit;
        buffer->length = 1;
        return;
    }
    buffer->byte = (buffer->byte << 1) + bit;
    buffer->length++;
}

void delete_write_buffer(TBinWriteBuffer *buffer)
{
    if (buffer->length == 0)
        return;

    for (int i = buffer->length; i < 8; i++)
    {
        buffer->byte = buffer->byte << 1;
    }
    if (buffer->isBinWriteMode)
    {
        write_byte_as_bin_to_file(buffer->destFile, buffer->byte);
    }
    else
    {
        write_byte_as_text_to_file(buffer->destFile, buffer->byte);
    }
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
        buffer->length = 8;
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

        buffer->length = 8;
    }

    int bit = (buffer->byte >> (buffer->length - 1)) & 1;
    buffer->length--;

    return bit;
}

void delete_read_buffer(TBinReadBuffer *buffer)
{
    free(buffer);
}