#include "utils.h"  


char *read_string_from_file(FILE *file)
{
    int baseBufferLength = 32;
    int capacity = baseBufferLength;
    int length = 0;
    char *string_buffer = malloc(capacity);
    if (!string_buffer)
        return NULL;

    char ch;
    while (fread(&ch, sizeof(char), 1, file) != 0)
    {
        string_buffer[length] = (char)ch;
        length++;

        if (ch == '\0')
        {
            return string_buffer;
        }

        if (length >= capacity)
        {
            capacity *= 2;
            char *newBuffer = realloc(string_buffer, capacity);
            if (!newBuffer)
            {
                free(string_buffer);
                return NULL;
            }
            string_buffer = newBuffer;
        }
    }
    free(string_buffer);
    return NULL;
}
