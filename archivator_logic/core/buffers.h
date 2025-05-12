#ifndef BUFFERS_H
#define BUFFERS_H 1

#include <stdio.h>

typedef struct
{
    FILE *destFile;
    int length;
    char byte;

    int isEmpty;
    int isBinWriteMode;
} TBinWriteBuffer;

TBinWriteBuffer *get_write_buffer(FILE *ofile, int isBinWrite);
extern void delete_write_buffer(TBinWriteBuffer *buffer);
extern void write_buffer_push(TBinWriteBuffer *buffer, int bit);
extern void write_byte_as_bin_to_file(FILE *destFile, char byte);
extern void write_byte_as_text_to_file(FILE *destFile, char byte);


typedef struct
{
    FILE *readFile;
    int length;
    char byte;
} TBinReadBuffer;

extern TBinReadBuffer *get_read_buffer(FILE *ifile);
extern void delete_read_buffer(TBinReadBuffer *buffer);
extern int pop_bit_from_read_buffer(TBinReadBuffer *buffer);


#endif 