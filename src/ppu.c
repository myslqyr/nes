#include "../include/type.h"

u8 ppu_read(u16 addr) {
    addr &= 0x3FFF;
    return 0;
}

void ppu_write(u16 addr, u8 data) {
    addr &= 0x3FFF;
    return;
}

u8 ppu_cpu_read(u16 addr) {
    return 0;
}

void ppu_cpu_write(u16 addr, u8 data) {
    return;
}