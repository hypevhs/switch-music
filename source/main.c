#include "main.h"

#define SAMPLE_RATE 48000
#define BUFFER_COUNT 2
#define CHANNEL_COUNT 2
#define SAMPLES_PER_MONO_CHUNK (SAMPLE_RATE / 30)
#define SAMPLES_PER_MULTICHANNEL_CHUNK (SAMPLES_PER_MONO_CHUNK * CHANNEL_COUNT)
#define AUDOUT_DATA_SIZE (SAMPLES_PER_MULTICHANNEL_CHUNK * sizeof(s16))
#define AUDOUT_BUFFER_SIZE ((AUDOUT_DATA_SIZE + 0xfff) & ~0xfff)

static AudioOutBuffer soundBuffer[2];
static u32 toBeLoaded = 2;
static u32 currentBuffer = 0;

int main() {
    gfxInitDefault();
    consoleInit(NULL);

    printf("Simple game-music-emu demonstration program\n");

    // Load game music file
    size_t gme_mempool_size = (gamemus_bin_size + 0xFFF) &~ 0xFFF;
    void* gme_mempool_ptr = memalign(0x1000, gme_mempool_size);
    memcpy(gme_mempool_ptr, gamemus_bin, gamemus_bin_size);
    armDCacheFlush(gme_mempool_ptr, gme_mempool_size);

    size_t gme2_mempool_size = (gamemus2_bin_size + 0xFFF) &~ 0xFFF;
    void* gme2_mempool_ptr = memalign(0x1000, gme2_mempool_size);
    memcpy(gme2_mempool_ptr, gamemus2_bin, gamemus2_bin_size);
    armDCacheFlush(gme2_mempool_ptr, gme2_mempool_size);

    // Init audout
    audoutInitialize();
    audoutStartAudioOut();
    for (int idx = 0; idx < BUFFER_COUNT; idx++) {
        soundBuffer[idx].next = NULL;
        soundBuffer[idx].buffer = memalign(0x1000, AUDOUT_BUFFER_SIZE);
        soundBuffer[idx].buffer_size = AUDOUT_BUFFER_SIZE;
        soundBuffer[idx].data_size = AUDOUT_DATA_SIZE;
        soundBuffer[idx].data_offset = 0;
        memset(soundBuffer[idx].buffer, 0, AUDOUT_BUFFER_SIZE);
    }

    // Init/start GME
    Music_Emu* emu;
    handle_error(gme_open_data(gme_mempool_ptr, gme_mempool_size, &emu, 48000));
    gme_start_track(emu, 0);

    Music_Emu* emu2;
    handle_error(gme_open_data(gme2_mempool_ptr, gme2_mempool_size, &emu2, 48000));
    gme_start_track(emu2, 3);

    // Main loop
    while (appletMainLoop()) {
        gfxSwapBuffers();
        hidScanInput();

        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);

        if (kDown & KEY_PLUS)
            break;

        static float panning = 0.0f;
        if (kHeld & KEY_RIGHT) {
            panning += (1/120.0f);
        }
        if (kHeld & KEY_LEFT) {
            panning -= (1/120.0f);
        }
        if (panning > +1.0f) panning = +1.0f;
        if (panning < -1.0f) panning = -1.0f;
        if (kHeld & (KEY_LEFT | KEY_RIGHT)) {
            printf("%f\n", panning);
        }
        update_audio(emu, emu2, panning);

        gfxFlushBuffers();
    }

    audoutStopAudioOut();
    audoutExit();
    for (int idx = 0; idx < BUFFER_COUNT; idx++) {
        free(soundBuffer[0].buffer);
        soundBuffer[0].buffer = NULL;
    }
    gme_delete(emu);

    gfxExit();
    return 0;
}

void update_audio(Music_Emu* emu, Music_Emu* emu2, float panning) {
    /* on an average frame, toBeLoaded = 0 at this point. but if there are no audio buffers added to the system's queue
     * yet, specifically when the program just started, we prevent overwriting this initial value of 2 (or whatever)
     * and instead just spend this time loading up all (2) of the buffers
     */
    if (toBeLoaded <= 0) {
        AudioOutBuffer* unused = NULL;
        audoutGetReleasedAudioOutBuffer(&unused, &toBeLoaded);
    }
    while (toBeLoaded > 0) {
        static s16 workingBufferA[SAMPLES_PER_MULTICHANNEL_CHUNK];
        static s16 workingBufferB[SAMPLES_PER_MULTICHANNEL_CHUNK];
        memset(workingBufferA, 0, AUDOUT_DATA_SIZE);
        memset(workingBufferB, 0, AUDOUT_DATA_SIZE);
        handle_error(gme_play(emu, SAMPLES_PER_MULTICHANNEL_CHUNK, workingBufferA));
        handle_error(gme_play(emu2, SAMPLES_PER_MULTICHANNEL_CHUNK, workingBufferB));
        panning = fmaxf(-1.0f, fminf(panning, 1.0f));
        float volumeA = 1.0f - fmaxf(0.0f, panning);
        float volumeB = 1.0f - fmaxf(0.0f, -panning);
        for (long idx = 0; idx < SAMPLES_PER_MULTICHANNEL_CHUNK; idx++) {
            s16* writeToMe = soundBuffer[currentBuffer].buffer;
            s16 trackA = (s16)(workingBufferA[idx] * volumeA);
            s16 trackB = (s16)(workingBufferB[idx] * volumeB);
            writeToMe[idx] = safe_add(trackA, trackB);
        }
        audoutAppendAudioOutBuffer(&soundBuffer[currentBuffer]);
        currentBuffer = (currentBuffer + 1) % BUFFER_COUNT;
        toBeLoaded--;
    }
}

s16 safe_add(s16 a, s16 b) {
    if (a > 0 && b > SHRT_MAX - a) {
        return SHRT_MAX;
    } else if (a < 0 && b < SHRT_MIN - a) {
        return SHRT_MIN;
    }
    return a + b;
}

void handle_error(const char* str) {
	if (str) {
		printf("Error: %s\n", str);
	}
}
