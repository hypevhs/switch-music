#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <switch.h>
#include <mikmod.h>
#include "module_bin.h"

int main() {
    gfxInitDefault();
    consoleInit(NULL);

    printf("Simple libmikmod demonstration program\n");

    // Load module file
    size_t mod_mempool_size = (module_bin_size + 0xFFF) &~ 0xFFF;
    void* mod_mempool_ptr = memalign(0x1000, mod_mempool_size);
    memcpy(mod_mempool_ptr, module_bin, module_bin_size);
    armDCacheFlush(mod_mempool_ptr, mod_mempool_size);

    // Initialize MikMod
    MikMod_RegisterDriver(&drv_switch);
    MikMod_RegisterAllLoaders();
    md_mode |= DMODE_SOFT_MUSIC | DMODE_NOISEREDUCTION;
    if (MikMod_Init("")) {
        printf("Could not initialize sound, reason: %s\n",
                MikMod_strerror(MikMod_errno));
    } else {
        printf("MikMod initialized.\n");
    }
    char* drivers = MikMod_InfoDriver();
    printf("Drivers installed:\n%s\n", drivers);

    // Load module
    MODULE* module = Player_LoadMem(mod_mempool_ptr, mod_mempool_size, 32, 0);
    if (module) {
        printf("MikMod module loaded.\n");
        module->wrap = true;
    } else {
        printf("Couldn't load module, reason: %s\n",
                MikMod_strerror(MikMod_errno));
    }

    // Start playing
    printf("Playing %s (%s, %d chn)\n", module->songname, module->modtype,
        module->numchn);
    Player_Start(module);
    printf("Press A to toggle pause.\n");

    // Main loop
    while (appletMainLoop())
    {
        gfxSwapBuffers();
        hidScanInput();

        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_PLUS)
            break;

        if (kDown & KEY_A) {
            Player_TogglePause();
            printf("Player (un)paused\n");
        }

        if (Player_Active()) {
            MikMod_Update();
        }

        gfxFlushBuffers();
    }

    Player_Stop();
    Player_Free(module);

    gfxExit();
    return 0;
}
