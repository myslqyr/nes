#include "../include/type.h"
#include "../include/ppu.h"
#include "../include/cartridge.h"
#include <string.h>

PPU *ppu;
u8 tblName[2][1024];    // 名称表
u8 tblPalette[32];   // 调色板
static u8 tblPattern[2][4096];  // 图案表

void ppu_init(PPU *ppu) {
    if (ppu == NULL) {
        return;
    }

    memset(ppu, 0, sizeof(PPU));
    memset(tblName, 0, sizeof(tblName));
    memset(tblPalette, 0, sizeof(tblPalette));
}

/*图像存储器（pattern memory）坐落在内存地址$0000~$1FFF区域，
而名称表（name table）坐落在$2000~$3EFF区域，
最后调色板内存（palette memory）位于$3F00~$3FFF区域。*/

u8 ppu_read(u16 addr) {
    addr &= 0x3FFF;

    if (addr <= 0x1FFF && addr >= 0x0000) {
        return tblPattern[(addr & 0x1000) >> 12][addr & 0x0FFF];
    } else if (addr >= 0x2000 && addr <= 0x3EFF) {
        addr &= 0x0FFF;
        if (cartridge_get_mirror() == MIRROR_VERTICAL) {
            if (addr <= 0x03FF) return tblName[0][addr & 0x03FF];
            if (addr <= 0x07FF) return tblName[1][addr & 0x03FF];
            if (addr <= 0x0BFF) return tblName[0][addr & 0x03FF];
            return tblName[1][addr & 0x03FF];
        } else {
            if (addr <= 0x03FF) return tblName[0][addr & 0x03FF];
            if (addr <= 0x07FF) return tblName[0][addr & 0x03FF];
            if (addr <= 0x0BFF) return tblName[1][addr & 0x03FF];
            return tblName[1][addr & 0x03FF];
        }
    } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
        addr &= 0x001F;
        if (addr == 0x0010) addr = 0x0000;
        if (addr == 0x0014) addr = 0x0004;
        if (addr == 0x0018) addr = 0x0008;
        if (addr == 0x001C) addr = 0x000C;
        return tblPalette[addr];
    }

    return 0;
}

void ppu_write(u16 addr, u8 data) {
    addr &= 0x3FFF;

    if (addr <= 0x1FFF && addr >= 0x0000) {
        tblPattern[(addr & 0x1000) >> 12][addr & 0x0FFF] = data;
        return;
    } else if (addr >= 0x2000 && addr <= 0x3EFF) {
        addr &= 0x0FFF;
        if (cartridge_get_mirror() == MIRROR_VERTICAL) {
            if (addr <= 0x03FF) { tblName[0][addr & 0x03FF] = data; return; }
            if (addr <= 0x07FF) { tblName[1][addr & 0x03FF] = data; return; }
            if (addr <= 0x0BFF) { tblName[0][addr & 0x03FF] = data; return; }
            tblName[1][addr & 0x03FF] = data;
            return;
        } else {
            if (addr <= 0x03FF) { tblName[0][addr & 0x03FF] = data; return; }
            if (addr <= 0x07FF) { tblName[0][addr & 0x03FF] = data; return; }
            if (addr <= 0x0BFF) { tblName[1][addr & 0x03FF] = data; return; }
            tblName[1][addr & 0x03FF] = data;
            return;
        }
    } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
        addr &= 0x001F;
        if (addr == 0x0010) addr = 0x0000;
        if (addr == 0x0014) addr = 0x0004;
        if (addr == 0x0018) addr = 0x0008;
        if (addr == 0x001C) addr = 0x000C;
        tblPalette[addr] = data;
        return;
    }
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