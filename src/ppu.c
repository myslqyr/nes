#include "../include/type.h"
#include "../include/ppu.h"

PPU *ppu;
u8 tblName[2][1024];
u8 tblPalette[32];

void ppu_init(PPU *ppu) {
    //TODO
    return;
}

u8 ppu_read(u16 addr) {
    addr &= 0x3FFF;
    return 0;
}

void ppu_write(u16 addr, u8 data) {
    addr &= 0x3FFF;
    return;
}

u8 ppu_cpu_read(u16 addr) {
    switch (addr)
    {
    case 0x0000:    // PPUCTRL
        return 0;
    case 0x0001:    // PPUMASK
        return 0;
    case 0x0002:    // PPUSTATUS
        return 0;
    case 0x0003:    // OAMADDR
        return 0;
    case 0x0004:    // OAMDATA
        return 0;
    case 0x0005:    // PPUSCROLL
        return 0;
    case 0x0006:    // PPUADDR
        return 0;
    case 0x0007:    // PPUDATA
        return 0;
    default:
        break;
    }
    return 0;
}

void ppu_cpu_write(u16 addr, u8 data) {
     switch (addr)
    {
    case 0x0000:    // PPUCTRL
        ppu->control.reg = data;
        break;
    case 0x0001:    // PPUMASK
        ppu->mask.reg = data;
        break;
    case 0x0002:    // PPUSTATUS
        ppu->status.reg = data;
        break;
    case 0x0003:    // OAMADDR
        return ;
    case 0x0004:    // OAMDATA
        return ;
    case 0x0005:    // PPUSCROLL
        return ;
    case 0x0006:    // PPUADDR
        return ;
    case 0x0007:    // PPUDATA
        return ;
    default:
        break;
    }
    return;
}