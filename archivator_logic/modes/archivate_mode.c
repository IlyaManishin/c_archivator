#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/types.h"
#include "../include/core_api.h"
#include "../include/pathlib.h"

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
            return false;
        }
    }
    return true;
}

// check memory!!!!
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
        else
        {
            paths.paths = NULL;
        }
        paths.pathsCount = 0;
    }
    return paths;
}

static void write_result(TFileData data, TSetupSettings *settings)
{
    printf("added: %s\n", data.path);
    printf("Base size: %lu bytes (%Lf MB), compressed size: %lu bytes (%Lf MB)\n",
           data.baseSizeBytes, (long double)data.baseSizeBytes / 1024 / 1024,
           data.compressSizeBytes, (long double)data.compressSizeBytes / 1024 / 1024);
    printf("\n");
    // bool isInfo = settings->withInfo;
    // bool isConsoleInfo = false;
    // if (isInfo && settings->_infoDest == stdout)
    // {
    //     isConsoleInfo = true;
    // }
}

void archivate_mode_run(TSetupSettings *settings, TArchivatorResponse *respDest)
{
    if (IS_WINDOWS)
    {
        strcpy(respDest->errorMessage, "Your system is not supported. You might buy a good device");
        respDest->isError = true;
        return;
    }

    char *destPath = settings->archivePath;
    FILE *archive = fopen(destPath, "wb");
    if (archive == NULL)
    {
        snprintf(respDest->errorMessage, ERROR_LENGTH, "Can't create archive with path %s", destPath);
        respDest->isError = true;
        return;
    }

    TPathArr paths = get_paths_to_archivate(settings, respDest);
    if (respDest->isError)
    {
        return;
    }
    TPathArr serializedPaths;
    if (settings->dirToArchivate != NULL)
    {
        serializedPaths = serialize_dir_paths(paths, settings->dirToArchivate);
    }
    else
    {
        serializedPaths = serialize_files_paths(paths);
    }
    if (serializedPaths.pathsCount == 0)
    {
        snprintf(respDest->errorMessage, ERROR_LENGTH, "Can`t save paths to archive");
        respDest->isError = true;
        goto exit;
    }

    write_headers_to_archive(archive, paths);
    for (int i = 0; i < paths.pathsCount; i++)
    {
        TFileData result = archivate_file(paths.paths[i], serializedPaths.paths[i], archive, respDest);

        if (respDest->isError)
        {
            remove(destPath);
            delete_file_data(result);
            goto exit;
        }
        write_result(result, settings);
        delete_file_data(result);
    }

exit:
    if (paths.isFreeNeeded)
    {
        delete_path_arr(paths);
    }
    if (settings->_absDirToArchivate != NULL)
    {
        free(settings->_absDirToArchivate);
    }
}