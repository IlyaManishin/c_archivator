#include "archivator_logic/archivator_api.h"
#include "line_command_reader.h"

#include <stdlib.h>

int main(int argc, char **argv)
{
    TSetupSettings *settings = read_setup_settings(argc, argv);
    if (settings->isError)
    {
        fprintf(stderr, "ERROR: %s\n", settings->errorMessage);
        delete_settings(settings);
        return EXIT_FAILURE;
    }
    int returnCode = EXIT_SUCCESS;
    TArchivatorResponse *resp = run_archivator(settings);
    if (resp->isError)
    {
        fprintf(stderr, "%s\n", resp->errorMessage);
        returnCode = EXIT_FAILURE;
    }
    delete_response(resp);
    delete_settings(settings);
    return returnCode;
}