#include "chip8.h"
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <cstring>
#include <ios>
#include <vector>

//font data
uint8_t fontData[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

//initialize emulator
Chip8::Chip8(){
    progCounter=0x200;
    index=0;
    stackPointer=0;
    drawFlag = false;
    srand(static_cast<unsigned>(time(nullptr)));

    //initialize memory and filling it with zeros
    //in c++ arrays contain leftover metadata from whatever
    // was there before wich could cause bugs
    memset(memory, 0, sizeof(memory));
    memset(registers, 0, sizeof(registers));
    memset(stack, 0, sizeof(stack));
    memset(display, 0, sizeof(display));
    memset(keypad, 0, sizeof(keypad));

    //write fonts to memory
    for (int i = 0; i < 80; i++) {
    memory[i] = fontData[i];
}

    delayTimer=0;
    soundTimer=0;
}

bool Chip8::loadROM(const char* filename){
    //open the file at the end position
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if(!file.is_open())
        return false;

    //tellg() will return file size since we are at end position
    std::streamsize size=file.tellg();
    //finally return to begining to start reading data
    file.seekg(0, std::ios::beg);

    //since chip8 ha 4096 bytes of memory and 512 are reserved for fonts
    //we could hardcode buffer at 4096-512
    //instead we use a dynamic array
    //char is used because .read() expects a char array
    if (size > static_cast<std::streamsize>(sizeof(memory) - 0x200))
        size = sizeof(memory) - 0x200;

    std::vector<char> buffer(size);
    file.read(buffer.data(), size);

    //write ROM to memory
    for (int i=0; i<size; i++)
        memory[0x200+i]=buffer[i];

    return true;
}

void Chip8::cycle(){
    //The original implementation of the Chip-8 language includes 36 different instructions
    //All instructions are 2 bytes long (16 bits)
    //To fetch 2 bytes:read byte at progCounter and shift it 8 bits then or the next byte
    uint16_t opcode=(memory[progCounter]<<8) | memory[progCounter+1];
    progCounter+=2;

    //type of instruction
    uint8_t kk=(opcode & 0xF000)>>12;
    //first register index
    uint8_t x=(opcode & 0x0F00)>>8;
    //second register index
    uint8_t y=(opcode & 0x00F0)>>4;
    //4bit number
    uint8_t n=(opcode & 0x000F);
    //8bit number
    uint8_t nn=(opcode & 0x00FF);
    //12bit memory address
    uint16_t addr=(opcode & 0x0FFF);

    //decoder
    switch(kk){
        case 0x0:
            switch(nn){
                case 0xE0:
                memset(display,0,sizeof(display));
                drawFlag = true;
                break;
                case 0xEE:
                stackPointer--;
                progCounter=stack[stackPointer];
                break;
            }break;
        case 0x1:
        progCounter=addr;
        break;
        case 0x2:
        stack[stackPointer]=progCounter;
        stackPointer++;
        progCounter=addr;
        break;
        case 0x3:
        if(registers[x]==nn)
            progCounter+=2;
        break;
        case 0x4:
        if(registers[x]!=nn)
            progCounter+=2;
        break;
        case 0x5:
        if(registers[x]==registers[y])
            progCounter+=2;
        break;
        case 0x6:
        registers[x]=nn;
        break;
        case 0x7:
        registers[x]+=nn;
        break;
        case 0x8:
            switch (n) {
                case 0x0:
                registers[x]=registers[y];
                break;
                case 0x1:
                registers[x] |= registers[y];
                registers[0xF] = 0;
                break;
                case 0x2:
                registers[x]&=registers[y];
                registers[0xF] = 0;
                break;
                case 0x3:
                registers[x]^=registers[y];
                registers[0xF] = 0;
                break;
                case 0x4:{
                uint16_t result=registers[x]+registers[y];
                registers[0xF] = (result > 255) ? 1 : 0;
                registers[x]=result & 0xFF;
                break;}
                case 0x5:
                registers[0xF]=(registers[x]>=registers[y])? 1 : 0;
                registers[x]-=registers[y];
                break;
                case 0x6:
                registers[0xF]= registers[y] & 0x1;
                registers[x]=registers[y]>>1;
                break;
                case 0x7:
                registers[0xF]=(registers[y]>=registers[x])?1:0;
                registers[x]=registers[y]-registers[x];
                break;
                case 0xE:
                registers[0xF]=(registers[y]>>7)&0x1;
                registers[x]=registers[y]<<1;
                break;
            }break;
        case 0x9:
        if(registers[x]!=registers[y])
        progCounter+=2;
        break;
        case 0xA:
        index=addr;
        break;
        case 0xB:
        progCounter=addr+registers[0];
        break;
        case 0xC:
        registers[x]=(rand()%256)&nn;
        break;
        case 0xD:{
            uint8_t xPos=registers[x]%64;
            uint8_t yPos=registers[y]%32;
            registers[0xF]=0;
            for(int row=0; row<n; row++){
                uint8_t spriteByte=memory[index+row];
                for(int col=0; col<8; col++){
                    uint8_t spritePixel=(spriteByte>>(7-col))& 0x1;
                    int screenIndex=(yPos+row)*64 + (xPos+col);
                    if((yPos+row)>=32||(xPos+col)>=64)
                        continue;
                if(spritePixel&&display[screenIndex]){
                    registers[0xF]=1;
                }
            display[screenIndex]^=spritePixel;        
                }
            }
        }
        drawFlag = true;
        break;
        case 0xE:
        switch (nn) {
            case 0x9E:
            if(keypad[registers[x]])
                progCounter+=2;
            break;
            case 0xA1:
            if(!keypad[registers[x]])
                progCounter+=2;
            break;
        }break;
        case 0xF:
            switch (nn) {
                case 0x07:
                registers[x]=delayTimer;
                break;
                case 0x0A:{
                    bool keyPressed=false;
                    for(int i=0; i<16; i++){
                        if (keypad[i]){
                            registers[x]=i;
                            keyPressed=true;
                            break;
                        }
                    }
                if(!keyPressed)
                    progCounter-=2;
                break;
                }
            case 0x15:
            delayTimer=registers[x];
            break;
            case 0x18:
            soundTimer=registers[x];
            break;
            case 0x1E:
            index+=registers[x];
            break;
            case 0x29:
            index=registers[x]*5;
            break;
            case 0x33:
            memory[index]=registers[x]/100;
            memory[index+1]=(registers[x]/10)%10;
            memory[index+2]=registers[x]%10;
            break;
            case 0x55:
            for(int i=0; i<=x; i++)
                memory[index+i]=registers[i];
            break;
            case 0x65:
            for(int i=0; i<=x; i++)
                registers[i]=memory[index+i];
            break;
            }
        break;
        }
        
}

void Chip8::tickTimers(){
    if(delayTimer>0) delayTimer--;
    if(soundTimer>0) soundTimer--;
}