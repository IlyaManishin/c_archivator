#ifndef BUFFERS_H
#define BUFFERS_H 1

#include <stdio.h>
#include <stdint.h>

typedef struct
{
    FILE *destFile;
    int length;
    char byte;

    uint64_t checkSum;
    uint64_t bytesCount;
} TBinWriteBuffer;

TBinWriteBuffer *get_write_buffer(FILE *ofile);
extern void delete_write_buffer(TBinWriteBuffer *buffer);
extern void write_buffer_push(TBinWriteBuffer *buffer, int bit);
extern void flash_write_buffer(TBinWriteBuffer *buffer);

extern long write_buffer_ftell(TBinWriteBuffer *buffer);
extern void write_buffer_fseek_cur(TBinWriteBuffer *buffer, long pos);
extern void buffer_write_arg(TBinWriteBuffer *buffer, void *arg_ptr, size_t size);
extern void buffer_write_string(TBinWriteBuffer *buffer, char *s);

typedef struct
{
    FILE *readFile;
    int length;
    char byte;

    uint64_t checkSum;
} TBinReadBuffer;

extern TBinReadBuffer *get_read_buffer(FILE *ifile);
extern void delete_read_buffer(TBinReadBuffer *buffer);
extern int pop_bit_from_read_buffer(TBinReadBuffer *buffer);
extern char *buffer_read_string(TBinReadBuffer* readBuffer);
extern size_t buffer_read_arg(TBinReadBuffer *buffer, void* arg, size_t size);


#endif