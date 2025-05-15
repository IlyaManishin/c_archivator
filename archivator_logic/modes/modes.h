#ifndef ARCHIVATOR_MODES_H
#define ARCHIVATOR_MODES_H 1

#include "../include/types.h"

void archivate_mode_run(TSetupSettings *settings, TArchivatorResponse *respDest);
void dearchivate_mode_run(TSetupSettings *settings, TArchivatorResponse *respDest);

#endif