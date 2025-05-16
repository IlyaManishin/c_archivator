#ifndef ARCHIVATOR_CORE_API_H
#define ARCHIVATOR_CORE_API_H 1

#include "types.h"

extern TFileData archivate_file(char *sourcePath, char *serializedPath, FILE *archiveFile, TArchivatorResponse *respDest);
extern TFileData dearchivate_file(FILE *archiveFile, char* absDestDir, TArchivatorResponse *errorDest);


extern void delete_file_data(TFileData data);

#endif