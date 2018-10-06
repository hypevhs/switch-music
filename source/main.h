#ifndef SWITCHMUSIC_H
#define SWITCHMUSIC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <switch.h>
#include <gme/gme.h>
#include "gamemus_bin.h"
#include "gamemus2_bin.h"

void update_audio(Music_Emu* emu, Music_Emu* emu2);
void handle_error(const char* str);

#endif /* SWITCHMUSIC_H */