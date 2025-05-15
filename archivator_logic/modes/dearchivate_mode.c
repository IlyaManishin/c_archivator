#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/types.h"
#include "../include/core_api.h"
#include "../include/pathlib.h"


void dearchivate_mode_run(TSetupSettings *settings, TArchivatorResponse *respDest)
{
    if (settings->archivePath == NULL)
    {
        strcpy(respDest->errorMessage, "No archive path");
        respDest->isError = true;
        return;
    }
    if (!is_file_exists(settings->archivePath))
    {
        snprintf(respDest->errorMessage, ERROR_LENGTH, "Can`t open archive: %s", settings->archivePath);
        respDest->isError = true;
        return;
    }

    FILE *archive = fopen(settings->archivePath, "rb");
    uint32_t filesCount;
    size_t res = fread(&filesCount, sizeof(uint32_t), 1, archive);
    if (res == 0)
    {
        strcpy(respDest->errorMessage, "Invalid archive");
        respDest->isError = true;
        return;
    }
    for (int i = 0; i < filesCount; i++)
    {
        TFileData result = dearchivate_file(settings->archivePath, respDest);
    }
}
