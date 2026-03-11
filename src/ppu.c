#include "../include/type.h"
#include "../include/ppu.h"
#include "../include/cartridge.h"
#include <string.h>
#include <stdlib.h>

PPU *ppu;

enum cpu_ppu_io //cpu与ppu的总线通信寄存器
{
    PPU_CTRL       = 0x0000,
    PPU_MASK       = 0x0001,
    PPU_STATUS     = 0x0002,
    OAM_ADDR       = 0x0003,
    OAM_DATA       = 0x0004,
    PPU_SCROLL     = 0x0005,
    PPU_ADDR       = 0x0006,
    PPU_DATA       = 0x0007,
    PPU_IO_END
};


void ppu_init(void) {
    ppu = (PPU *)malloc(sizeof(PPU));  // 自动清零
    if (!ppu) return;
    // 1. 清零整个结构体内存，确保所有数组和变量初始为 0
    memset(ppu, 0, sizeof(PPU));

    // 2. 初始化时序计数器
    ppu->cycle = 0;        // 周期计数器 0-340
    ppu->scanline = 0;     // 扫描线 -1~260
    ppu->frame = 0;        // 帧计数器

    // 3. 初始化 CPU-PPU 通信寄存器 (复位状态通常为 0)
    ppu->control.reg = 0x00;   // $2000
    ppu->mask.reg = 0x00;      // $2001
    ppu->status.reg = 0x00;    // $2002
    ppu->oam_addr = 0x00;      // $2003
    ppu->oam_data = 0x00;      // $2004
    ppu->scroll = 0x00;        // $2005
    ppu->PPUADDR = 0x00;       // $2006
    ppu->PPUDATA = 0x00;       // $2007

    // 4. 初始化内部地址锁存器
    ppu->vram_addr.reg = 0x0000; // 当前 VRAM 地址
    ppu->tram_addr.reg = 0x0000; // 临时地址寄存器 (结构体中定义为 tram_addr)
    ppu->latch_address = 0x00;      // 地址锁存
    ppu->ppu_addr = 0x0000;      // 编译好的内存地址


    // 5. 初始化内存区域 (memset 已清零，此处仅为逻辑示意)
    // ppu->vram[2048]    -> 已清零
    // ppu->palette[32]   -> 已清零
    // ppu->oam[256]      -> 已清零

    // 6. 初始化输出缓冲
    ppu->data_buffer = 0x00;
    // ppu->framebuffer[256 * 240] -> 已清零

    // 7. 初始化中断标志
    ppu->nmi = false;
}

u8 ppu_intern_read(u16 addr) {

}
void ppu_intern_write(u16 addr, u8 data) {

}


u8 ppu_cpu_read(u16 addr) {
    u8 data = 0;
    switch (addr & 0x0007)
    {
    case PPU_STATUS:
    {
        data = (ppu->status.reg & 0xE0) | (ppu->data_buffer & 0x1F);
        ppu->status.vertical_blank = 0;
        break;
    }
    case OAM_DATA:
        break;
    case PPU_DATA:
    {
        data = ppu->data_buffer;
        ppu->data_buffer = ppu_intern_read(ppu->vram_addr.reg);
         if (ppu->vram_addr.reg >= 0x3F00) {
            data = ppu->data_buffer;
        }
        ppu->vram_addr.reg += (ppu->control.increment_mode ? 32 : 1);
        break;
    }
    default:
        break;
    }
    return data;
}




void ppu_cpu_write(u16 addr, u8 data) {
    switch (addr & 0x0007)
    {
    case PPU_CTRL:
    {
        ppu->control.reg = data;
        ppu->tram_addr.nametable_x = ppu->control.nametable_x;
        ppu->tram_addr.nametable_y = ppu->control.nametable_y;
        break;
    }
    case PPU_MASK:
    {
        ppu->mask.reg = data;
        break;
    }
    case OAM_ADDR:
        break;
    case OAM_DATA:
        break;
    case PPU_SCROLL: 
        break;
    case PPU_ADDR:
    {
        if (ppu->latch_address == 0) {
            ppu->tram_addr.reg = ((data & 0x3F) << 8) | (ppu->tram_addr.reg & 0x00FF);
            ppu->latch_address = 1; 
        } else {
            ppu->tram_addr.reg = data | (ppu->tram_addr.reg & 0xFF00);
            ppu->vram_addr = ppu->tram_addr;
            ppu->latch_address = 0;
        }
        break;
    }
    case PPU_DATA:
    {
        ppu_intern_write(ppu->vram_addr.reg, data);
        ppu->vram_addr.reg += (ppu->control.increment_mode ? 32 : 1);
        break;
    }
    default:
        break;
    }

}