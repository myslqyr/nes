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
        /*数组的第1维通过检查PPU地址的最高二进制位来确定是选择左边的拼图还是右边的拼图
        数组的第2维通过屏蔽PPU地址中剩余的二进制位来计算该内存中的偏移量*/
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

u8 ppu_cpu_read(u16 addr, bool rdonly) {
    u8 data = 0x00;
    if (ppu == NULL) {
        return 0;
    }

    if (rdonly) {
        switch (addr) {
        case 0x0000:    // PPUCTRL
            data = ppu->control.reg;
            break;
        case 0x0001:    // PPUMASK
            data = ppu->mask.reg;
            break;
        case 0x0002:    // PPUSTATUS
            data = ppu->status.reg;
            break;
        case 0x0003:    // OAMADDR  
            break;
        case 0x0004:    // OAMDATA
            break;
        case 0x0005:    
            break;
        case 0x0006:
            break;
        case 0x0007:    // PPUDATA
            break;
        default:
            break;
        }
        return data;
    }
    else {
        switch (addr) {
        case 0x0000:    // PPUCTRL (not readable)
            break;
        case 0x0001:    // PPUMASK (not readable)
            break;
        case 0x0002:    // PPUSTATUS
            data = (ppu->status.reg & 0xE0) | (ppu->ppu_data_buffer & 0x1F);
            ppu->status.vertical_blank = 0;
            ppu->addr_latch = 0;
            break;
        case 0x0003:    // OAMADDR
            break;
        case 0x0004:    // OAMDATA
            break;
        case 0x0005:    // PPUSCROLL
            break;
        case 0x0006:    // PPUADDR
            break;
        case 0x0007:    // PPUDATA
            data = ppu->ppu_data_buffer;
            ppu->ppu_data_buffer = ppu_read(ppu->vram_addr);
            if (ppu->vram_addr >= 0x3F00) {
                data = ppu->ppu_data_buffer;
            }
            ppu->vram_addr += (ppu->control.increment_mode ? 32 : 1);
            break;
        default:
            break;
        }
    }
    return data;
}

void ppu_cpu_write(u16 addr, u8 data) {
    if (ppu == NULL) {
        return;
    }

    switch (addr)
    {
    case 0x0000:    // PPUCTRL
        ppu->control.reg = data;
        ppu->tram_addr = (ppu->tram_addr & 0xF3FF) | ((ppu->control.nametable_y << 11) | (ppu->control.nametable_x << 10));
        break;
    case 0x0001:    // PPUMASK
        ppu->mask.reg = data;
        break;
    case 0x0002:    // PPUSTATUS
        break;
    case 0x0003:    // OAMADDR
        ppu->oam_addr = data;
        break;
    case 0x0004:    // OAMDATA
        ppu->oam[ppu->oam_addr] = data;
        break;
    case 0x0005:    // PPUSCROLL
        if (ppu->addr_latch == 0)
        {
            ppu->fine_x = data & 0x07;
            ppu->tram_addr = (ppu->tram_addr & 0xFFE0) | (data >> 3);
            ppu->addr_latch = 1;
        }
        else
        {
            ppu->tram_addr = (ppu->tram_addr & 0x8FFF) | ((data & 0x07) << 12);
            ppu->tram_addr = (ppu->tram_addr & 0xFC1F) | ((data & 0xF8) << 2);
            ppu->addr_latch = 0;
        }
        break;
    case 0x0006:    // PPUADDR
        if (ppu->addr_latch == 0)
        {
            ppu->tram_addr = (ppu->tram_addr & 0x00FF) | ((data & 0x3F) << 8);
            ppu->addr_latch = 1;
        }
        else
        {
            ppu->tram_addr = (ppu->tram_addr & 0xFF00) | data;
            ppu->vram_addr = ppu->tram_addr;
            ppu->addr_latch = 0;
        }
        break;
    case 0x0007:    // PPUDATA
        ppu_write(ppu->vram_addr, data);
        ppu->vram_addr += (ppu->control.increment_mode ? 32 : 1);
        break;
    default:
        break;
    }
}