#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "archivator_api.h"
#include "modes/pathlib.h"
#include "modes/modes.h"

void delete_response(TArchivatorResponse *response)
{
    free(response->errorMessage);
    free(response);
}

TArchivatorResponse *run_archivator(TSetupSettings *settings)
{
    TArchivatorResponse *response = (TArchivatorResponse *)malloc(sizeof(TArchivatorResponse));
    response->isError = false;

    if (UNDEFINED_SYSTEM){
        strcpy(response->errorMessage, "Your system is not supported");
        response->isError = true;
        return response;
    }

    if (settings->infoDestPath != NULL)
    {
        settings->_infoDest = fopen(settings->infoDestPath, "a");

        if (settings->_infoDest == NULL)
        {
            response->isError = true;
            strcpy(response->errorMessage, "Invalid logger path");
            return response;
        }
    }
    else
    {
        settings->_infoDest = stdout;
    }

    if (settings->mode == archivateMode)
    {
        archivate_mode_run(settings, response);
    }
    else
    {
    }
    return response;
}