#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/types.h"
#include "../include/core_api.h"
#include "../include/pathlib.h"
#include "eta_progress.h"

static void print_result(TFileData data, TEta *eta, int index)
{
    char line[MAX_LINE_LENGTH];
    snprintf(line, MAX_LINE_LENGTH - 1, "Extracted: %s", data.path);
    print_string_with_eta(eta, line);
    update_eta(eta, index);
}
void dearchivate_mode_run(TSetupSettings *settings, TArchivatorResponse *respDest)
{
    if (settings->archivePath == NULL)
    {
        strcpy(respDest->errorMessage, "No archive path\n");
        respDest->isError = true;
        return;
    }
    if (!is_file_exists(settings->archivePath))
    {
        snprintf(respDest->errorMessage, ERROR_LENGTH, "Can`t open archive: %s\n", settings->archivePath);
        respDest->isError = true;
        return;
    }

    FILE *archiveFile = fopen(settings->archivePath, "rb");
    uint32_t filesCount;
    size_t res = fread(&filesCount, sizeof(uint32_t), 1, archiveFile);
    if (res == 0)
    {
        strcpy(respDest->errorMessage, "Invalid archive\n");
        respDest->isError = true;
        return;
    }

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
    TEta *eta = get_eta_progress(filesCount);
    for (int i = 0; i < filesCount; i++)
    {
        TFileData result = dearchivate_file(archiveFile, absDestDir, respDest);
        print_result(result, eta, i + 1);
    }
}
