#include "main.h"

#define SAMPLE_RATE 48000
#define CHANNEL_COUNT 2
#define SAMPLES_PER_MONO_CHUNK SAMPLE_RATE
#define SAMPLES_PER_MULTICHANNEL_CHUNK (SAMPLES_PER_MONO_CHUNK * CHANNEL_COUNT)
#define AUDOUT_DATA_SIZE (SAMPLES_PER_MULTICHANNEL_CHUNK * sizeof(s16))
#define AUDOUT_BUFFER_SIZE ((AUDOUT_DATA_SIZE + 0xfff) & ~0xfff)

AudioOutBuffer soundBuffer[2];

int main() {
    gfxInitDefault();
    consoleInit(NULL);

    printf("Simple game-music-emu demonstration program\n");

    // Load game music file
    size_t gme_mempool_size = (gamemus_bin_size + 0xFFF) &~ 0xFFF;
    void* gme_mempool_ptr = memalign(0x1000, gme_mempool_size);
    memcpy(gme_mempool_ptr, gamemus_bin, gamemus_bin_size);
    armDCacheFlush(gme_mempool_ptr, gme_mempool_size);

    // Init/start GME
    Music_Emu* emu;
    handle_error(gme_open_data(gme_mempool_ptr, gme_mempool_size, &emu, 48000));
    gme_start_track(emu, 0);

    // Init audout
    audoutInitialize();
    audoutStartAudioOut();
    soundBuffer[0].next = NULL;
    soundBuffer[0].buffer = memalign(0x1000, AUDOUT_BUFFER_SIZE);
    soundBuffer[0].buffer_size = AUDOUT_BUFFER_SIZE;
    soundBuffer[0].data_size = AUDOUT_DATA_SIZE;
    soundBuffer[0].data_offset = 0;
    memset(soundBuffer[0].buffer, 0, AUDOUT_BUFFER_SIZE);
    soundBuffer[1].next = NULL;
    soundBuffer[1].buffer = memalign(0x1000, AUDOUT_BUFFER_SIZE);
    soundBuffer[1].buffer_size = AUDOUT_BUFFER_SIZE;
    soundBuffer[1].data_size = AUDOUT_DATA_SIZE;
    soundBuffer[1].data_offset = 0;
    memset(soundBuffer[1].buffer, 0, AUDOUT_BUFFER_SIZE);
    // u32 initial_buffers = 2;
    // handle_error(gme_play(emu, SAMPLECOUNT * 4, soundBuffer[0].buffer));
    handle_error(gme_play(emu, SAMPLES_PER_MULTICHANNEL_CHUNK, soundBuffer[0].buffer));
    handle_error(gme_play(emu, SAMPLES_PER_MULTICHANNEL_CHUNK, soundBuffer[1].buffer));
    audoutAppendAudioOutBuffer(&soundBuffer[0]);
    audoutAppendAudioOutBuffer(&soundBuffer[1]);

    // Main loop
    while (appletMainLoop()) {
        gfxSwapBuffers();
        hidScanInput();

        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_PLUS)
            break;


        gfxFlushBuffers();
    }

    gme_delete(emu);
    free(soundBuffer[0].buffer);
    free(soundBuffer[1].buffer);
    soundBuffer[0].buffer = NULL;
    soundBuffer[1].buffer = NULL;

    gfxExit();
    return 0;
}

void handle_error(const char* str) {
	if (str) {
		printf("Error: %s\n", str);
	}
}
