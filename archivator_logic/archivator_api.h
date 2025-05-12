#ifndef ARCHIVATOR_API_H
#define ARCHIVATOR_API_H 1

#include <stdbool.h>

#include "include/types.h"

extern TArchivatorResponse* run_archivator(TSetupSettings *settings);
extern void delete_response(TArchivatorResponse* response);

#endif