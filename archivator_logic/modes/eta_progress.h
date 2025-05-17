#ifndef ETA_PROGRESS_H
#define ETA_PROGRESS_H

#include <stdint.h>
#include <stdlib.h>

#define ETA_FOOTER_LENGTH 10

typedef struct {
    uint32_t curInd;
    uint32_t maxInd;

    int barLength;
    size_t lastLength;
} TEta;

extern int get_terminal_width();
extern TEta *get_eta_progress(uint32_t maxInd);


#endif 