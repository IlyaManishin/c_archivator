#ifndef PATHLIB_H
#define PATHLIB_H

#include <stdbool.h>
#include <stdint.h>

#if defined(_WIN32)
#include <windows.h>

#define UNDEFINED_SYSTEM 0
#define PATH_SEP '\\'

#elif defined(__linux__)

#define _GNU_SOURCE 1
#define UNDEFINED_SYSTEM 0

#define PATH_SEP '/'

#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#else
#define UNDEFINED_SYSTEM 1
#endif

#define SERIALIZE_SEP '/'

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

#define BASE_PATH_ARR_CAPACITY 10
typedef struct
{
    char **paths;
    uint32_t pathsCount;

    bool isFreeNeeded;
    int _capacity;
} TPathArr;

extern void delete_path_arr(TPathArr arr);

extern bool is_dir_exists(char *dir);
extern bool is_file_exists(char *path);
extern TPathArr list_dir(char *dirPath);
extern TPathArr serialize_dir_paths(TPathArr paths, char *dirPath);
extern TPathArr serialize_files_paths(TPathArr paths);

extern char *get_real_path(char *src);
extern void delete_path_arr(TPathArr arr);

extern int create_dirs_for_file(char *filePath);
extern char *get_free_file_path(char *destPath);
extern char *path_concat(char *path1, char *path2, char sep);


#endif