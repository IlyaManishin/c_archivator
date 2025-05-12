#ifndef ARCHIVATOR_CORE_API_H
#define ARCHIVATOR_CORE_API_H 1

#include "types.h"  

TFileData archivate_file(char sourcePath[], FILE *archiveFile, TArchivatorResponse *respDest);


#endif 