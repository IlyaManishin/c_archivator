#ifndef LINECOMMAND_READER_H
#define LINECOMMAND_READER_H 1

#include "archivator_logic/archivator_api.h"

extern TSetupSettings *read_setup_settings(int argc, char **argv);
void delete_settings(TSetupSettings *settings);
extern void print_settings(TSetupSettings* settings);

#endif