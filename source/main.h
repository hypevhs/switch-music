#ifndef SWITCHMUSIC_H
#define SWITCHMUSIC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <switch.h>
#include <mikmod.h>
#include <gme/gme.h>
#include "module_bin.h"
#include "gamemus_bin.h"

MODULE* mikModInit(void* mod_mempool_ptr, size_t mod_mempool_size);
void mikModPlay(MODULE* module);
void handle_error(const char* str);

#endif /* SWITCHMUSIC_H */