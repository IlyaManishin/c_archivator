#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "eta_progress.h"
#define BASE_TERMINAL_SIZE 90
#define MAX_FOOTER_SIZE 20
#define SIDES_LENGTH 2
#define FILL_SYMBOL '#'
#define UNFILL_SYMBOL '-'

#define GREEN "\033[0;32m"
#define RESET "\033[0m"

static void print_eta(TEta *eta);

#if defined(_WIN64)
#include <windows.h>

int get_terminal_width()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
    {
        int size = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        return size;
    }
    return BASE_TERMINAL_SIZE;
}

#else
#include <unistd.h>
#include <sys/ioctl.h>

int get_terminal_width()
{
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0)
    {
        int size = w.ws_col;
        return size;
    }
    return BASE_TERMINAL_SIZE;
}
#endif

TEta *get_eta_progress(uint32_t maxInd)
{
    TEta *eta = (TEta *)malloc(sizeof(TEta));
    eta->curInd = 0;
    eta->lastLength = 0;
    eta->maxInd = maxInd;
    eta->barLength = get_terminal_width();

    print_eta(eta);
    return eta;
}

void delete_eta(TEta *eta)
{
    printf("\n");
    free(eta);
}

static void print_eta(TEta *eta)
{
    if (eta->maxInd == 0)
    {
        return;
    }
    int fullLength;
    int terminalWidth = get_terminal_width();

    int barLength = terminalWidth - MAX_FOOTER_SIZE;
    int fillSymbolCount = (int)((barLength - SIDES_LENGTH) * ((float)eta->curInd / eta->maxInd));
    int unFillSymbolCount = barLength - fillSymbolCount - SIDES_LENGTH;
    printf("[");
    printf(GREEN);
    for (int i = 0; i < fillSymbolCount; i++)
    {
        printf("%c", FILL_SYMBOL);
    }
    printf(RESET);

    for (int i = 0; i < unFillSymbolCount; i++)
    {
        printf("%c", UNFILL_SYMBOL);
    }
    printf("]");

    fullLength = 2 + unFillSymbolCount + fillSymbolCount;
    char footer[MAX_FOOTER_SIZE];
    snprintf(footer, MAX_FOOTER_SIZE, "   %d/%d", eta->curInd, eta->maxInd);
    printf("%s\r", footer);
    fflush(stdout);

    fullLength += strlen(footer);
    eta->lastLength = fullLength;
}

void update_eta(TEta *eta, int newInd)
{
    eta->curInd = newInd;
    print_eta(eta);
}

void print_string_with_eta(TEta *eta, char *line)
{
    int length = strlen(line);
    for (int i = length - 1; i >= 0; i++)
    {
        if (line[i] == '\n')
        {
            line[i] = '\0';
            length--;
        }
        else
        {
            break;
        }
    }
    printf("%s", line);

    if (length < eta->lastLength)
    {
        int delta = eta->lastLength - length;
        for (int i = 0; i < delta; i++)
        {
            printf("%c", ' ');
        }
    }
    printf("\n");
}


