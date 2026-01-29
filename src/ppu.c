#include "../include/type.h"
#include "../include/ppu.h"
#include "../include/cartridge.h"
#include <string.h>

PPU *ppu;
u8 tblName[2][1024];    // 名称表，将 CHR 和 tile 联系起来
u8 tblPalette[32];   // 调色板
static u8 tblPattern[2][4096];  // 图案表

// NES调色板，包含64种颜色，格式为RGBA
static const u32 NES_PALETTE_RGBA[64] = {
    0x757575FF, 0x271B8FFF, 0x0000ABFF, 0x47009FFF,
    0x8F0077FF, 0xAB0013FF, 0xA70000FF, 0x7F0B00FF,
    0x432F00FF, 0x004700FF, 0x005100FF, 0x003F17FF,
    0x1B3F5FFF, 0x000000FF, 0x000000FF, 0x000000FF,

    0xBCBCBCFF, 0x0073EFFF, 0x233BEFFF, 0x8300F3FF,
    0xBF00BFFF, 0xE7005BFF, 0xDB2B00FF, 0xCB4F0FFF,
    0x8B7300FF, 0x009700FF, 0x00AB00FF, 0x00933BFF,
    0x00838BFF, 0x000000FF, 0x000000FF, 0x000000FF,

    0xFFFFFFFF, 0x3FBFFFFF, 0x5F97FFFF, 0xA78BFDFE,
    0xF77BFFFF, 0xFF77B7FF, 0xFF7763FF, 0xFF9B3BFF,
    0xF3BF3FFF, 0x83D313FF, 0x4FDF4BFF, 0x58F898FF,
    0x00EBDBFF, 0x000000FF, 0x000000FF, 0x000000FF,

    0xFFFFFFFF, 0xABE7FFFF, 0xC7D7FFFF, 0xD7CBFFFF,
    0xFFC7FFFF, 0xFFC7DBFF, 0xFFBFB3FF, 0xFFDBABFF,
    0xFFE7A3FF, 0xE3FFA3FF, 0xABF3BFFF, 0xB3FFCFFF,
    0x9FFFF3FF, 0x000000FF, 0x000000FF, 0x000000FF
};

void ppu_init(PPU *ppu) {
    if (ppu == NULL) {
        return;
    }

    memset(ppu, 0, sizeof(PPU));
    memset(tblName, 0, sizeof(tblName));
    memset(tblPalette, 0, sizeof(tblPalette));
}

/*图像存储器（pattern memory）在内存地址$0000~$1FFF区域，
名称表（name table）在$2000~$3EFF区域，
调色板内存（palette memory）位于$3F00~$3FFF区域。*/

//通过ppu总线进行读写

u8 ppu_read(u16 addr) {
    addr &= 0x3FFF;

    if (addr >= 0x0000 && addr <= 0x1FFF) {
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

    if (addr >= 0x0000 && addr <= 0x1FFF) {
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


void GetPatternTable (u8 index, u8 palette) {
    for(u16 tile_y = 0; tile_y < 16 ; tile_y++) {
        for(u16 tile_x = 0; tile_x < 16; tile_x++) { 
            u16 offset = tile_y * 256 + tile_x * 16;
            for(u16 row = 0; row < 8; row++) {
                // Get tile data (2 bytes per row)
                u8 tile_lsb = ppu_read(index * 0x1000 + offset + row + 0);
                u8 tile_msb = ppu_read(index * 0x1000 + offset + row + 8);
                for(u16 col = 0; col < 8; col++) { 
                    // 计算tile对应的调色板索引
                    u8 pixel = (tile_lsb & 0x01) + (tile_msb & 0x01);
                    tile_lsb >>= 1; tile_msb >>= 1;
                }
            }
        }
    }
}


u32 GetColourFromPalette (u8 palette, u8 pixel) {
    return NES_PALETTE_RGBA[ppu_read(0x3F00 + (palette << 2) +pixel)];
}