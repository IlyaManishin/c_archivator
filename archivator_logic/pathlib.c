#include "include/pathlib.h"

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef struct
{
    char **parts;
    int partsCount;
} TSplittedPath;

void delete_path_arr(TPathArr arr)
{
    if (arr.paths == NULL)
    {
        return;
    }
    for (int i = 0; i < arr.pathsCount; i++)
    {
        free(arr.paths[i]);
    }
    free(arr.paths);
    arr.paths = NULL;
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

int make_dir(char *name)
{
#if defined(_WIN32)
    return -1;

#elif defined(__linux__)
    int res = mkdir(name, 0755);
    if (res == -1 && errno == EEXIST)
    {
        return 0;
    }
    return res;
#else
    return -1;
#endif
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

static void delete_double_seps(char* path)
{

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

static char *get_parent(char *path, char sep)
{
    int length = strlen(path);
    int endInd = -1;
    for (int i = length - 1; i >= 0; i--)
    {
        if (path[i] == sep)
        {
            endInd = i;
            break;
        }
    }
    if (endInd == -1)
    {
        return NULL;
    }
    char *result = (char *)malloc((endInd + 2) * sizeof(char));
    strncpy(result, path, endInd + 1);
    result[endInd + 1] = '\0';
    return result;
}

TPathArr serialize_dir_paths(TPathArr paths, char *dirPath)
{
    TPathArr result;
    result.paths = (char **)malloc(paths.pathsCount * sizeof(char *));
    result.pathsCount = paths.pathsCount;

    char *dirAbsPath = get_real_path(dirPath);
    char *dirParent = get_parent(dirAbsPath, PATH_SEP);
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
    for (int i = 0; i < result.pathsCount; i++)
    {
        delete_double_seps(result.paths[i]);
        replace_path_sep(result.paths[i], PATH_SEP, SERIALIZE_SEP);
    }
    free(dirAbsPath);
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

int create_dirs_for_file(char *filePath)
{
    char *fileDir = get_parent(filePath, SERIALIZE_SEP);
    if (fileDir == NULL)
    {
        return 0;
    }
    int length = strlen(fileDir);
    char *buffer = (char *)malloc((length + 1) * sizeof(char));
    for (int i = 0; i < length; i++)
    {
        if (fileDir[i] == SERIALIZE_SEP)
        {
            if (i != 0)
            {
                buffer[i] = '\0';
                int res = make_dir(buffer);
                if (res == -1)
                {
                    return -1;
                }
            }
        }
        buffer[i] = fileDir[i];
    }
    free(buffer);
    return 0;
}

char *path_concat(char *path1, char *path2, char sep)
{
    int length1 = strlen(path1);
    int length2 = strlen(path2);

    char *res = (char *)malloc((length1 + length2 + 2) * sizeof(char));
    char *rightDest = res;
    if (path1[length1] != sep)
    {
        strcpy(res, path1);
        res[length1] = sep;
        rightDest += length1 + 1;
    }
    else
    {
        strcpy(res, path1);
        rightDest += length1;
    }

    if (length2 == 0)
    {
        return res;
    }
    if (path2[0] != sep)
    {
        stpcpy(rightDest, path2);
    }
    else
    {
        strcpy(rightDest, path2 + 1);
    }
    return res;
}

char *get_free_file_path(char *destPath)
{
    if (destPath == NULL)
    {
        return NULL;
    }

    int length = strlen(destPath);
    int maxPostfixLength = 16;
    int resLength = length + maxPostfixLength;
    char *res = (char *)malloc((resLength + 1) * sizeof(char));

    if (!is_file_exists(destPath))
    {
        strcpy(res, destPath);
        return res;
    }

    char *destCopy = (char *)malloc((length + 1) * sizeof(char));
    strcpy(destCopy, destPath);

    char *extention = (char *)malloc((length + 1) * sizeof(char));
    extention[0] = '\0';

    for (int i = length - 1; i >= 0; i--)
    {
        if (destPath[i] == SERIALIZE_SEP)
        {
            break;
        }
        char ch = destPath[i];
        if (destPath[i] == '.')
        {
            destCopy[i] = '\0';
            strcpy(extention, destPath + i + 1);
            break;
        }
    }

    int maxIterations = 1000;
    bool isFind = false;
    for (int fileInd = 1; fileInd < maxIterations; fileInd++)
    {
        if (extention[0] == '\0')
        {
            snprintf(res, resLength, "%s(%d)", destCopy, fileInd);
        }
        else
        {
            snprintf(res, resLength, "%s(%d).%s", destCopy, fileInd, extention);
        }
        if (!is_file_exists(res))
        {
            isFind = true;
            break;
        }
    }
    free(destCopy);
    free(extention);
    if (isFind)
    {
        return res;
    }
    else
    {
        free(res);
        return NULL;
    }
}

int test()
{
    // TPathArr paths;
    // paths.pathsCount = 3;
    // paths.paths = malloc(3 * sizeof(char *));
    // paths.paths[0] = "/check/in/dir/name";
    // paths.paths[1] = "/check/in/file/name";
    // paths.paths[2] = "/check/in/test/name";

    // TPathArr res = serialize_dir_paths(paths, "/check/in");
    // char *absCurDir = get_real_path("../../../../../");
    // TPathArr paths = list_dir(absCurDir);
    // TPathArr res = serialize_dir_paths(paths, absCurDir);
    // for (int i = 0; i < res.pathsCount; i++)
    // {
    //     printf("%s\n", res.paths[i]);
    // }
    // return 0;
    char check[1000] = "main.c";
    char *res = get_free_file_path(check);
    printf("%s", res);
    return 0;
}

