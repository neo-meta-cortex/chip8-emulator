# chip8-emulator

A Chip-8 emulator written in C++.

## Build
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Usage
```bash
./chip8 path/to/rom.ch8
```

## Controls

| Chip-8 | Keyboard |
|--------|----------|
| 1 2 3 C | 1 2 3 4 |
| 4 5 6 D | Q W E R |
| 7 8 9 E | A S D F |
| A 0 B F | Z X C V |

## ROMs

Free public domain ROMs can be found at [chip8Archive](https://github.com/JohnEarnest/chip8Archive).

## Credits
http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
https://austinmorlan.com/posts/chip8_emulator/#how-does-a-cpu-work
https://tobiasvl.github.io/blog/write-a-chip-8-emulator/