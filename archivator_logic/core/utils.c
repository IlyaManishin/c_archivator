#include "utils.h"  

#define BASE_BUFFER_LENGTH 64

char *read_string_from_file(FILE *file)
{
    int capacity = BASE_BUFFER_LENGTH;
    int length = 0;
    char *buffer = malloc(capacity);
    if (!buffer)
        return NULL;

    char ch;
    while (fread(&ch, sizeof(char), 1, file) != 0)
    {
        buffer[length] = (char)ch;
        length++;

        if (ch == '\0')
        {
            return buffer;
        }

        if (length >= capacity)
        {
            capacity *= 2;
            char *newBuffer = realloc(buffer, capacity);
            if (!newBuffer)
            {
                free(buffer);
                return NULL;
            }
            buffer = newBuffer;
        }
    }
    free(buffer);
    return NULL;
}
