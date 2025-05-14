#include "pathlib.h"

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    char **parts;
    int partsCount;
} TSplittedPath;

void delete_path_arr(TPathArr arr)
{
    for (int i = 0; i < arr.pathsCount; i++)
    {
        free(arr.paths[i]);
    }
    free(arr.paths);
}

bool is_dir_exists(char *dir)
{
#ifdef _WIN32
    return false;
#endif

#ifdef __linux__

    if (access(dir, F_OK) == -1)
    {
        return false;
    }
    DIR *dirCur = opendir(dir);
    if (dirCur == NULL)
    {
        return false;
    }
    closedir(dirCur);
    return true;

#endif

    return NULL;
}

bool is_file_exists(char *path)
{
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        return false;
    }
    fclose(file);
    return true;
}

static void list_dir_recursion(char *curDir, TPathArr *dest)
{
#ifdef _WIN32
    return;
#endif

#ifdef __linux__
    DIR *dir = opendir(curDir);
    if (!dir)
    {
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        int entryPathLength = strlen(curDir) + strlen(entry->d_name) + 2;
        char *entryPath = (char *)malloc(entryPathLength * sizeof(char));

        snprintf(entryPath, entryPathLength * sizeof(char), "%s%c%s", curDir, PATH_SEP, entry->d_name);

        bool pathIsDir = (entry->d_type == DT_DIR);
        if (pathIsDir)
        {
            list_dir_recursion(entryPath, dest);
            free(entryPath);
        }
        else
        {
            if (dest->_capacity == dest->pathsCount)
            {
                dest->_capacity *= 2;
                dest->paths = (char **)realloc(dest->paths, dest->_capacity * sizeof(char *));
            }
            dest->paths[dest->pathsCount] = entryPath;
            dest->pathsCount++;
        }
    }
    closedir(dir);
#endif
}

TPathArr list_dir(char *dirPath)
{
    TPathArr result;
    result.pathsCount = 0;
    result._capacity = BASE_PATH_ARR_CAPACITY;
    result.paths = (char **)malloc(BASE_PATH_ARR_CAPACITY * sizeof(char *));

    list_dir_recursion(dirPath, &result);
    return result;
}

char *get_real_path(char *src)
{
    if (IS_WINDOWS)
    {
        return NULL;
    }
    else
    {
        char *real = realpath(src, NULL);
        return real;
    }
}

static void replace_path_sep(char *path, char oldSep, char newSep)
{
    int length = strlen(path);
    for (int i = 0; i < length; i++)
    {
        if (path[i] == oldSep)
        {
            path[i] = newSep;
        }
    }
}

static TSplittedPath path_to_splitted(char *path, char *buffer)
{
    int pathLength = strlen(path);

    int isFreeBuffer = false;
    if (buffer == NULL)
    {
        buffer = (char *)malloc((pathLength + 1) * sizeof(char));
        isFreeBuffer = true;
    }

    TSplittedPath sPath;
    sPath.partsCount = 0;

    int minPartsSize = 1;
    for (int i = 0; i < pathLength; i++)
    {
        if (path[i] == PATH_SEP)
        {
            minPartsSize++;
        }
    }
    sPath.parts = (char **)malloc(minPartsSize * sizeof(char *));

    int partLength = 0;
    for (int i = 0; i <= pathLength; i++)
    {
        if ((i == pathLength || path[i] == PATH_SEP) && partLength != 0)
        {
            buffer[partLength] = '\0';
            sPath.parts[sPath.partsCount] = (char *)malloc((partLength + 1) * sizeof(char));
            strcpy(sPath.parts[sPath.partsCount], buffer);

            sPath.partsCount++;
            partLength = 0;
        }
        if (i == pathLength)
        {
            break;
        }
        if (path[i] == PATH_SEP)
        {
            continue;
        }
        buffer[partLength] = path[i];
        partLength++;
    }
    if (isFreeBuffer)
    {
        free(buffer);
    }
    return sPath;
}

static TSplittedPath *split_paths(TPathArr paths)
{
    TSplittedPath *result = (TSplittedPath *)malloc(paths.pathsCount * sizeof(TSplittedPath));

    char *pathBuffer = (char *)malloc((PATH_MAX + 1) * sizeof(char));
    for (int i = 0; i < paths.pathsCount; i++)
    {
        result[i] = path_to_splitted(paths.paths[i], pathBuffer);
    }
    free(pathBuffer);
    return result;
}

static void delete_splitted_paths(TSplittedPath *sPaths, int pathCount)
{
    if (sPaths == NULL || pathCount == 0)
    {
        return;
    }

    for (int i = 0; i < pathCount; i++)
    {
        TSplittedPath path = sPaths[i];
        for (int j = 0; j < path.partsCount; j++)
        {
            free(path.parts[j]);
        }
    }
    free(sPaths);
}

static int get_common_dirs_count(TSplittedPath *sPaths, int sPathCount)
{
    if (sPathCount == 0)
        return 0;

    int maxCommon = -1;
    for (int i = 0; i < sPathCount; i++)
    {
        if (maxCommon == -1)
        {
            maxCommon = sPaths[i].partsCount;
            continue;
        }
        maxCommon = min(maxCommon, sPaths[i].partsCount);
    }
    for (int i = 0; i < maxCommon; i++)
    {
        char *part = sPaths[0].parts[i];
        for (int pathInd = 1; pathInd < sPathCount; pathInd++)
        {
            if (strcmp(part, sPaths[pathInd].parts[i]) != 0)
            {
                int commonDirs = i;
                return i;
            }
        }
    }
    return maxCommon;
}

static void delete_double_seps(TPathArr paths)
{
    for (int i = 0; i < paths.pathsCount; i++)
    {
        char *path = paths.paths[i];
        int length = strlen(path);

        int curIndex = 0;
        for (int j = 0; j < length; j++)
        {
            if (j + 1 < length && path[j] == PATH_SEP && path[j + 1] == PATH_SEP)
            {
                continue;
            }
            if (curIndex == 0 && path[j] == PATH_SEP)
            {
                continue;
            }
            path[curIndex] = path[j];
            curIndex++;
        }
        path[curIndex] = '\0';
    }
}

static TPathArr join_splitted_paths(TSplittedPath *sPaths, int pathsCount, int commonCount)
{
    TPathArr result;
    result.pathsCount = pathsCount;
    result.paths = (char **)malloc(pathsCount * sizeof(char *));
    result.isFreeNeeded = true;

    for (int pathInd = 0; pathInd < pathsCount; pathInd++)
    {
        TSplittedPath sPath = sPaths[pathInd];
        int fullLength = 0;
        for (int i = 0; i < sPath.partsCount; i++)
        {
            fullLength += strlen(sPath.parts[i]);
        }
        char *joinedPath = (char *)malloc((fullLength + 1) * sizeof(char));
        int curIndex = 0;
        for (int i = 0; i < sPath.partsCount; i++)
        {
            char *part = sPath.parts[i];
            int partLength = strlen(part);
            for (int j = 0; j < partLength; j++)
            {
                joinedPath[curIndex] = part[j];
                curIndex++;
            }
        }
        joinedPath[curIndex] = '\0';
    }
    return result;
}

static char *get_parent(char *path)
{
    int length = strlen(path);

    TSplittedPath sPath = path_to_splitted(path, NULL);
    int partsCount = sPath.partsCount;
    if (partsCount == 0)
    {
        return NULL;
    }
    char *result = (char *)malloc(length * sizeof(char));
    int curIndex = 0;
    for (int i = 0; i < partsCount - 1; i++)
    {
        char *part = sPath.parts[i];
        int partLength = strlen(part);
        for (int j = 0; j < partLength; j++)
        {
            result[curIndex] = part[j];
            curIndex++;
        }
        if (i != partsCount - 1)
        {
            result[curIndex] = PATH_SEP;
            curIndex++;
        }
    }
    for (int i = 0; i < partsCount; i++)
    {
        free(sPath.parts[i]);
    }
    return result;
}

TPathArr serialize_dir_paths(TPathArr paths, char *dirPath)
{
    char *diraAbsPath = get_real_path(dirPath);

    TPathArr result;
    result.paths = (char **)malloc(paths.pathsCount * sizeof(char *));
    result.pathsCount = paths.pathsCount;

    char *dirParent = get_parent(dirPath);
    int parentLength = strlen(dirParent);
    for (int i = 0; i < paths.pathsCount; i++)
    {
        char *srcPath = paths.paths[i];
        int srcLength = strlen(srcPath);

        int trimLength = srcLength - parentLength + 1;
        result.paths[i] = (char *)malloc(trimLength * sizeof(char));
        for (int j = 0; j < trimLength - 1; j++)
        {
            result.paths[i][j] = srcPath[j + parentLength];
        }
        result.paths[i][trimLength - 1] = '\0';
    }
    delete_double_seps(result);
    for (int i = 0; i < result.pathsCount; i++)
    {
        replace_path_sep(result.paths[i], PATH_SEP, SERIALIZE_SEP);
    }
    return result;
}

static char *get_file_name(char *filePath)
{
    int length = strlen(filePath);
    int resLength = 0;
    char *resStart = filePath + length - 1;
    for (int i = length - 1; i >= 0; i--)
    {
        if (filePath[i] == PATH_SEP)
        {
            break;
        }
        resLength++;
        resStart--;
    }
    if (resLength == 0)
    {
        return NULL;
    }
    char *res = (char *)malloc((resLength + 1) * sizeof(char));
    strcpy(res, resStart);
    return res;
}

TPathArr serialize_files_paths(TPathArr paths)
{
    TPathArr result;
    result.pathsCount = paths.pathsCount;
    result.paths = (char **)malloc(paths.pathsCount * sizeof(char *));
    for (int i = 0; i < paths.pathsCount; i++)
    {
        char *fileName = get_file_name(paths.paths[i]);
        if (fileName == NULL)
        {
            result.pathsCount = 0;
            for (int j = 0; j < i; j++)
                free(result.paths[j]);
            free(result.paths);

            return result;
        }
        result.paths[i] = fileName;
    }
    return result;
}

static int test()
{
    // TPathArr paths;
    // paths.pathsCount = 3;
    // paths.paths = malloc(3 * sizeof(char *));
    // paths.paths[0] = "/check/in/dir/name";
    // paths.paths[1] = "/check/in/file/name";
    // paths.paths[2] = "/check/in/test/name";

    // TPathArr res = serialize_dir_paths(paths, "/check/in");
    char *absCurDir = get_real_path("../../../../../");
    TPathArr paths = list_dir(absCurDir);
    TPathArr res = serialize_dir_paths(paths, absCurDir);
    for (int i = 0; i < res.pathsCount; i++)
    {
        printf("%s\n", res.paths[i]);
    }
    return 0;
}

// static int path_to_relative(char *src, char *cwd, int cwdLength, char *dest)
// {
//     char absPath[1024];
//     char *res;
//     int cmpRes = strncmp(cwd, src, cwdLength);

//     int srcLength = strlen(src);
//     int destLength = srcLength - cmpRes;
// }

// static char *get_cwd()
// {
// #ifdef _WIN32
//     return NULL;
// #endif

// #ifdef __linux__
//     char *cwdPath = (char *)malloc(PATH_MAX * sizeof(char));
//     getcwd(cwdPath, PATH_MAX);
//     return cwdPath;
// #endif

//     return NULL;
// }
