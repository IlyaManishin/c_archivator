#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "eta_progress.h"
#define BASE_TERMINAL_SIZE 90

#if defined(_WIN64)
#include <windows.h>

int get_terminal_width()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
    {
        int size = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        return size
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
    return eta;
}
