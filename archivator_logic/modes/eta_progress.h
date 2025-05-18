#ifndef ETA_PROGRESS_H
#define ETA_PROGRESS_H

#include <stdint.h>
#include <stdlib.h>

#define ETA_FOOTER_LENGTH 10
#define MAX_LINE_LENGTH 256 

typedef struct {
    uint32_t curInd;
    uint32_t maxInd;

    int barLength;
    size_t lastLength;
} TEta;

extern int get_terminal_width();
extern TEta *get_eta_progress(uint32_t maxInd);
extern void update_eta(TEta *eta, int newInd);
extern void delete_eta(TEta *eta);
extern void print_string_with_eta(TEta *eta, char *line);



#endif 