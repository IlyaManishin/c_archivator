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

    FILE *archiveFile = fopen(settings->archivePath, "rb");
    uint32_t filesCount;
    size_t res = fread(&filesCount, sizeof(uint32_t), 1, archiveFile);

    char *absDestDir;
    if (settings->destDir != NULL)
    {
        absDestDir = get_real_path(settings->destDir);
    }
    else
    {
        absDestDir = (char *)malloc(1 * sizeof(char));
        absDestDir[0] = '\0';
    }
    if (res == 0)
    {
        strcpy(respDest->errorMessage, "Invalid archive");
        respDest->isError = true;
        return;
    }
    for (int i = 0; i < filesCount; i++)
    {
        TFileData result = dearchivate_file(archiveFile, absDestDir, respDest);
    }
}
