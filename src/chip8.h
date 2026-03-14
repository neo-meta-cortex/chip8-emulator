#include <cstdint>

class Chip8 {
private:
    uint8_t memory[4096];
    uint8_t registers[16];
    uint16_t index;
    uint16_t progCounter;
    uint16_t stack[16];
    uint8_t stackPointer;
    uint8_t delayTimer;

public:
    Chip8();
    bool loadROM(const char* filename);
    void cycle();
    void tickTimers();
    uint8_t display[64 * 32];
    uint8_t keypad[16];
    uint8_t soundTimer;
    bool drawFlag;
};
