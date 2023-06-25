#ifndef MAINWINDOW_H_INCLUDED
#define MAINWINDOW_H_INCLUDED
#define SDL_MAIN_HANDLED
#define __STDC_WANT_LIB_EXT1__ 1
#include <stdio.h>
#include <SDL2/SDL.h>
#include "Gui.h"

void InitalizeMainWindow();
void RunWindows();
void RunPalette(ivec2 pos,Window * winin);
void bootaudio();
extern char st_text[200];
#endif // MAINWINDOW_H_INCLUDED
