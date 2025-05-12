#include "archivator_logic/archivator_api.h"
#include "line_command_reader.h"

#include <stdlib.h>

int main(int argc, char **argv)
{
    TSetupSettings *settings = read_setup_settings(argc, argv);
    if (settings->isError)
    {
        fprintf(stderr, "ERROR: %s\n", settings->errorMessage);
        return -1;
    }
    TArchivatorResponse *resp = run_archivator(settings);
    if (resp->isError)
    {
        fprintf(stderr, "%s\n", resp->errorMessage);
    }
    delete_response(resp);
    delete_settings(settings);
}