#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/core_api.h"
#include "../include/pathlib.h"
#include "../include/types.h"
#include "eta_progress.h"

static void write_headers_to_archive(FILE *archive, TPathArr paths)
{
    fwrite(&paths.pathsCount, sizeof(uint32_t), 1, archive);
}

static bool is_valid_paths(TPathArr paths, TArchivatorResponse *resp)
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

TPathArr get_paths_to_archivate(TSetupSettings *settings, TArchivatorResponse *resp, char *archivePath)
{
    TPathArr paths;
    if (settings->dirToArchivate != NULL)
    {
        if (!is_dir_exists(settings->dirToArchivate))
        {
            snprintf(resp->errorMessage, ERROR_LENGTH, "Invalid directory to archivate: %s\n", settings->dirToArchivate);
            resp->isError = true;
            return paths;
        }

        char *absArchivateDir = get_real_path(settings->dirToArchivate);
        if (absArchivateDir == NULL)
        {
            strcpy(resp->errorMessage, "Invalid archivate directory\n");
            resp->isError = true;
            return paths;
        }
        paths = list_dir(absArchivateDir);
        paths.isFreeNeeded = true;

        char *archiveAbsPath = get_real_path(archivePath);
        for (int i = 0; i < paths.pathsCount; i++)
        {
            char *path = paths.paths[i];
            if (strcmp(path, archiveAbsPath) == 0)
            {
                free(path);
                paths.paths[i] = paths.paths[paths.pathsCount - 1];
                paths.pathsCount--;
            }
        }
        free(archiveAbsPath);
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
        resp->errorMessage = "No files to archivate\n";
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

static void print_result(TFileData data, TEta *eta, int index)
{
    char line[MAX_LINE_LENGTH];
    snprintf(line, MAX_LINE_LENGTH - 1, "Added: %s", data.path);
    print_string_with_eta(eta, line);
    update_eta(eta, index);
    // printf("added: %s\n", data.path);
    // printf("Base size: %lu bytes (%Lf MB), compressed size: %lu bytes (%Lf MB)\n",
    //        data.baseSizeBytes, (long double)data.baseSizeBytes / 1024 / 1024,
    //        data.compressSizeBytes, (long double)data.compressSizeBytes / 1024 / 1024);
    // printf("\n");
}

char *add_archive_extention(char *path)
{
    size_t pathLength = strlen(path);
    char *destPathWithExt = (char *)malloc(pathLength + strlen(ARCHIVE_EXTENTION) + 1);
    strcpy(destPathWithExt, path);
    strcpy(destPathWithExt + pathLength, ARCHIVE_EXTENTION);
    return destPathWithExt;
}

void archivate_mode_run(TSetupSettings *settings, TArchivatorResponse *respDest)
{
    if (UNDEFINED_SYSTEM)
    {
        strcpy(respDest->errorMessage, "Your system is not supported. You might buy a good device\n");
        respDest->isError = true;
        return;
    }

    char *destPath = settings->archivePath;
    char *destPathWithExt = add_archive_extention(destPath);
    FILE *archive = fopen(destPathWithExt, "wb");
    if (archive == NULL)
    {
        snprintf(respDest->errorMessage, ERROR_LENGTH, "Can't create archive with path %s\n", destPath);
        respDest->isError = true;
        free(destPathWithExt);
        return;
    }
    free(destPathWithExt);

    TPathArr paths = get_paths_to_archivate(settings, respDest, destPathWithExt);
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
        snprintf(respDest->errorMessage, ERROR_LENGTH, "Can`t save paths to archive\n");
        respDest->isError = true;
        goto exit;
    }
    uint64_t fullSize = 0;
    TEta *eta = get_eta_progress(paths.pathsCount);
    write_headers_to_archive(archive, paths);
    for (int i = 0; i < paths.pathsCount; i++)
    {
        TFileData result = archivate_file(paths.paths[i], serializedPaths.paths[i], archive, respDest);

        if (respDest->isError)
        {
            remove(destPath);
            delete_file_data(result);
            delete_eta(eta);
            goto exit;
        }
        fullSize += result.baseSizeBytes;
        print_result(result, eta, i + 1);
        delete_file_data(result);
    }
    delete_eta(eta);
    int fullSizeMB = fullSize / 1024 / 1024;
    printf("TOTAL EXTRACTED: %" PRId64 " BYTES(%d MB)\n", fullSize, fullSizeMB);

exit:
    if (paths.isFreeNeeded)
    {
        delete_path_arr(paths);
    }
    delete_path_arr(serializedPaths);
    return;
}