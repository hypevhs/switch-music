#include "main.h"
#include "main.h"

int main() {
    gfxInitDefault();
    consoleInit(NULL);

    printf("Simple libmikmod and game-music-emu demonstration program\n");

    // Load game music file
    size_t gme_mempool_size = (gamemus_bin_size + 0xFFF) &~ 0xFFF;
    void* gme_mempool_ptr = memalign(0x1000, gme_mempool_size);
    memcpy(gme_mempool_ptr, gamemus_bin, gamemus_bin_size);
    armDCacheFlush(gme_mempool_ptr, gme_mempool_size);

    // Load module file
    size_t mod_mempool_size = (module_bin_size + 0xFFF) &~ 0xFFF;
    void* mod_mempool_ptr = memalign(0x1000, mod_mempool_size);
    memcpy(mod_mempool_ptr, module_bin, module_bin_size);
    armDCacheFlush(mod_mempool_ptr, mod_mempool_size);

    // Init/start MikMod
    MODULE* module = mikModInit(mod_mempool_ptr, mod_mempool_size);
    mikModPlay(module);
    Player_TogglePause();

    // Init/start GME
    Music_Emu* emu;
    handle_error(gme_open_data(gme_mempool_ptr, gme_mempool_size, &emu, 48000));
    gme_start_track(emu, 0);
    s16 samps[48000];
    handle_error(gme_play(emu, 48000, samps));
    AudioOutBuffer omfg;
    u32 data_size = (48000 * 2 * 2);
    u32 buffer_size = (data_size + 0xfff) & ~0xfff;
    omfg.buffer = memalign(0x1000, buffer_size);
    omfg.data_size = data_size;
    omfg.next = NULL;
    omfg.data_offset = 0;
    memcpy(omfg.buffer, samps, sizeof(samps));
    audoutAppendAudioOutBuffer(&omfg);

    // Main loop
    while (appletMainLoop()) {
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
    gme_delete(emu);

    gfxExit();
    return 0;
}

MODULE* mikModInit(void* mod_mempool_ptr, size_t mod_mempool_size) {
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
    return module;
}

void mikModPlay(MODULE* module) {
    // Start playing
    printf("Playing %s (%s, %d chn)\n", module->songname, module->modtype,
        module->numchn);
    Player_Start(module);
    printf("Press A to toggle pause.\n");
}

void handle_error(const char* str) {
	if (str) {
		printf("Error: %s\n", str);
	}
}
