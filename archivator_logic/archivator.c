#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "archivator_api.h"
#include "include/pathlib.h"
#include "modes/modes.h"

void delete_response(TArchivatorResponse *response)
{
    free(response->errorMessage);
    free(response);
}

TArchivatorResponse *run_archivator(TSetupSettings *settings)
{
    TArchivatorResponse *response = (TArchivatorResponse *)malloc(sizeof(TArchivatorResponse));
    response->errorMessage = (char *)malloc(ERROR_LENGTH * sizeof(char));
    response->isError = false;

    if (UNDEFINED_SYSTEM)
    {
        strcpy(response->errorMessage, "Your system is not supported");
        response->isError = true;
        return response;
    }

    if (settings->mode == archivateMode)
    {
        archivate_mode_run(settings, response);
    }
    else if (settings->mode == dearchivateMode)
    {
        dearchivate_mode_run(settings, response);
    }
    else if (settings->mode == infoMode || settings->mode == checkMode)
    {
        get_archive_info(settings, response);
    }
    return response;
}