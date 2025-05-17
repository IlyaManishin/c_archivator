#ifndef ARCHIVATOR_MODES_H
#define ARCHIVATOR_MODES_H 1

#include "../include/types.h"

extern void archivate_mode_run(TSetupSettings *settings, TArchivatorResponse *respDest);
extern void dearchivate_mode_run(TSetupSettings *settings, TArchivatorResponse *respDest);
extern void get_archive_info(TSetupSettings *settings, TArchivatorResponse *respDest);

#endif