#include <iostream>
#include <cmath>
#include <SDL2/SDL.h>
#include "chip8.h"

const int SCALE = 10;
const int WINDOW_W = 64 * SCALE;
const int WINDOW_H = 32 * SCALE;
const int CYCLE_DELAY = 2;
const int SAMPLE_RATE = 44100;
const double BEEP_FREQUENCY = 440.0;

// CHIP-8 hex keypad (0-F) mapped to physical keys
const SDL_Keycode keymap[16] = {
    SDLK_x, SDLK_1, SDLK_2, SDLK_3,
    SDLK_q, SDLK_w, SDLK_e, SDLK_a,
    SDLK_s, SDLK_d, SDLK_z, SDLK_c,
    SDLK_4, SDLK_r, SDLK_f, SDLK_v
};

struct AudioState {
    bool active;
    double phase;
    Chip8* chip8;
};

void audioCallback(void* userdata, Uint8* stream, int len) {
    AudioState* state = static_cast<AudioState*>(userdata);
    state->active = state->chip8->soundTimer > 0;

    int sampleCount = len / sizeof(Sint16);
    Sint16* buffer = reinterpret_cast<Sint16*>(stream);

    for (int i = 0; i < sampleCount; i++) {
        if (state->active) {
            double sineValue = sin(state->phase);
            buffer[i] = static_cast<Sint16>(sineValue * 28000); // ~85% of Sint16 max amplitude
            state->phase += 2.0 * M_PI * BEEP_FREQUENCY / SAMPLE_RATE;
            if (state->phase > 2.0 * M_PI)
                state->phase -= 2.0 * M_PI;
        } else {
            buffer[i] = 0;
            state->phase = 0.0;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: chip8 <path_to_rom>" << std::endl;
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cout << "SDL Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Chip-8 Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H,
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED
    );

    Chip8 chip8;
    if(!chip8.loadROM(argv[1])){
        std::cout << "Error: could not open ROM file: " << argv[1] << std::endl;
        return 1;
    }

    AudioState audioState;
    audioState.active = false;
    audioState.phase = 0.0;
    audioState.chip8 = &chip8;

    SDL_AudioSpec desired;
    desired.freq = SAMPLE_RATE;
    desired.format = AUDIO_S16SYS;
    desired.channels = 1;
    desired.samples = 512;
    desired.callback = audioCallback;
    desired.userdata = &audioState;

    SDL_AudioDeviceID audioDevice = SDL_OpenAudioDevice(
        nullptr, 0, &desired, nullptr, 0
    );
    SDL_PauseAudioDevice(audioDevice, 0); // 0 = unpause (start playback)

    bool running = true;
    SDL_Event event;
    Uint32 lastTimerTick = SDL_GetTicks();

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;

            if (event.type == SDL_KEYDOWN) {
                for (int i = 0; i < 16; i++)
                    if (event.key.keysym.sym == keymap[i])
                        chip8.keypad[i] = 1;
            }

            if (event.type == SDL_KEYUP) {
                for (int i = 0; i < 16; i++)
                    if (event.key.keysym.sym == keymap[i])
                        chip8.keypad[i] = 0;
            }
        }

        chip8.cycle();

        Uint32 now = SDL_GetTicks();
        if(now - lastTimerTick >= 16){
            chip8.tickTimers();
            lastTimerTick = now;
        }

        SDL_Delay(CYCLE_DELAY);

        if (chip8.drawFlag) {
            chip8.drawFlag = false;

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);

            for (int row = 0; row < 32; row++) {
                for (int col = 0; col < 64; col++) {
                    if (chip8.display[row * 64 + col]) {
                        SDL_Rect pixel;
                        pixel.x = col * SCALE;
                        pixel.y = row * SCALE;
                        pixel.w = SCALE;
                        pixel.h = SCALE;
                        SDL_RenderFillRect(renderer, &pixel);
                    }
                }
            }

            SDL_RenderPresent(renderer);
        }
    }

    SDL_CloseAudioDevice(audioDevice);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
