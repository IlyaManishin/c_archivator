#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/types.h"
#include "../include/core_api.h"
#include "pathlib.h"

void write_headers_to_archive(FILE *archive, TPathArr paths)
{
    fwrite(&paths.pathsCount, sizeof(uint32_t), 1, archive);
}

bool is_valid_paths(TPathArr paths, TArchivatorResponse *resp)
{
    for (int i = 0; i < paths.pathsCount; i++)
    {
        if (!is_file_exists(paths.paths[i]))
        {
            resp->isError = true;
            snprintf(resp->errorMessage, ERROR_LENGTH, "Can't open path: %s", paths.paths[i]);
            return true;
        }
    }
    return false;
}

//check memory
TPathArr get_paths_to_archivate(TSetupSettings *settings, TArchivatorResponse *resp)
{
    TPathArr paths;
    if (settings->dirToArchivate != NULL)
    {
        if (!is_dir_exists(settings->dirToArchivate))
        {
            snprintf(resp->errorMessage, ERROR_LENGTH, "Invalid directory to archivate: %s", settings->dirToArchivate);
            resp->isError = true;
            return paths;
        }

        char *absArchivateDir = get_real_path(settings->dirToArchivate);
        if (absArchivateDir == NULL)
        {
            strcpy(resp->errorMessage, "Invalid archivate directory");
            resp->isError = true;
            return paths;
        }
        settings->dirToArchivate = absArchivateDir;

        paths = list_dir(absArchivateDir);
        paths.isFreeNeeded = true;
    }
    else
    {
        paths.paths = settings->filesToArchivate;
        paths.pathsCount = settings->filesCount;
        paths.isFreeNeeded = false;
    }

    if (paths.pathsCount == 0)
    {
        if (paths.isFreeNeeded)
        {
            delete_path_arr(paths);
        }

        resp->isError = true;
        resp->errorMessage = "No files to archivate!";
        return paths;
    }
    if (!is_valid_paths(paths, resp))
    {
        if (paths.isFreeNeeded)
        {
            delete_path_arr(paths);
        }
        paths.pathsCount = 0;
    }
    return paths;
}

void archivate_mode_run(TSetupSettings *settings, TArchivatorResponse *resp)
{
    if (IS_WINDOWS)
    {
        strcpy(resp->errorMessage, "Your system is not supported. You might buy a good device");
        resp->isError = true;
        return;
    }

    char *destPath = settings->archivePath;
    FILE *archive = fopen(destPath, "wb");
    if (archive == NULL)
    {
        snprintf(resp->errorMessage, ERROR_LENGTH, "Can't create archive with path %s", destPath);
        resp->isError = true;
        return;
    }

    TPathArr paths = get_paths_to_archivate(settings, resp);
    if (resp->isError)
    {
        return;
    }

    write_headers_to_archive(archive, paths);
    for (int i = 0; i < paths.pathsCount; i++)
    {
        TFileData result = archivate_file(paths.paths[i], archive, resp);

        if (result._isFreePathNeeded)
        {
            free(result.path);
        }
        if (resp->isError)
        {
            return;
        }
    }

exit:
    if (paths.isFreeNeeded)
    {
        delete_path_arr(paths);
    }
}