#include "../include/type.h"
#include "../include/ppu.h"
#include "../include/cartridge.h"
#include "../include/sdl.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

PPU *ppu;

bool cart_ppu_mapped_addr(u16 addr) { 
    // 0x0000-0x1FFF 映射到卡带 CHR-ROM/CHR-RAM
    return false;
}
u8   cart_ppu_read(u16 addr) { 
    return 0x00;
}
void cart_ppu_write(u16 addr, u8 data) { 
}
static u16 mirror_vram_addr(u16 addr) { return addr & 0x07FF; }

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

enum ppu_address_space
{
    PATTERN_TABLE_0_BOUND       =   0x1000,
    PATTERN_TABLE_1_BOUND       =   0x2000,
    NAME_TABLE_0_BOUND          =   0x23C0,
    ATTRIB_TABLE_0_BOUND        =   0x2400,
    NAME_TABLE_1_BOUND          =   0x27C0,
    ATTRIB_TABLE_1_BOUND        =   0x2800,
    NAME_TABLE_2_BOUND          =   0x2BC0,
    ATTRIB_TABLE_2_BOUND        =   0x2C00,
    NAME_TABLE_3_BOUND          =   0x2FC0,
    ATTRIB_TABLE_3_BOUND        =   0x3000,
    UNUSED_BOUND                =   0x3F00,
    PAL_RAM_BOUND               =   0x3F20,
    PAL_RAM_MIRROR_BOUND        =   0x4000,
};

enum ppu_io_device
{
    UNKOWN,
    CART,
    VRAM,       // video ram, including data of name-table and attribe-table
    PRAM,       // palatte ram, including data of palatte
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

    // 6. 初始化输出缓冲
    ppu->data_buffer = 0x00;
    // ppu->framebuffer[256 * 240] -> 已清零

    // 7. 初始化中断标志
    ppu->nmi = false;
    ppu->frame_complete = false;
}

static int ppu_memory_map (u16 addr) {
    if ((addr < UNUSED_BOUND) && (cart_ppu_mapped_addr(addr)))                return CART;      
    else if (addr < PATTERN_TABLE_1_BOUND)  return CART;
    else if (addr < ATTRIB_TABLE_3_BOUND)   return VRAM;
    else if (addr < UNUSED_BOUND)           return UNKOWN;
    else                                    return PRAM;
}

static int ppu_bus_memory_map(u16 addr)
{
    if (addr < UNUSED_BOUND && cart_ppu_mapped_addr(addr))           
        return CART;

    else if (addr < PATTERN_TABLE_1_BOUND)  return CART;
    else if (addr < ATTRIB_TABLE_3_BOUND)   return VRAM;
    else if (addr < UNUSED_BOUND)           return UNKOWN;
    else                                    return PRAM;
}

static u8 ppu_read_vram(u16 addr)
{
    return ppu->vram[mirror_vram_addr(addr & 0x0FFF)];
}
static u8 ppu_read_pram(u16 addr)
{
    return 0x00;
}

static void ppu_write_vram(u16 addr, u8 data)
{
    u16 offset = mirror_vram_addr(addr & 0x0FFF);
    ppu->vram[offset] = data;
}
static void ppu_write_pram(u16 addr, u8 data)
{

}


u8 ppu_intern_read(u16 addr) {
    switch (ppu_memory_map(addr))
    {
    case CART: return cart_ppu_read(addr);
    case VRAM: {
        u8 temp = ppu_read_vram(addr);
        return temp;
    }
        //return ppu_read_vram(addr);
    case PRAM: return ppu_read_pram(addr);
        break;
    default:
        return -1;
        break;
    }
}
void ppu_intern_write(u16 addr, u8 data) {
    switch (ppu_bus_memory_map(addr))
    {
    case CART: cart_ppu_write(addr, data); return;
    case VRAM:  ppu_write_vram(addr, data);  return;
    case PRAM: ppu_write_pram(addr, data); return;
    default:
        break;
    }
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
        if(stop()){printf("ppu mask:0x%x\n",data);}
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

bool ppu_nmi_triggered() {
    if(ppu->nmi) {
        ppu->nmi = false;
        return true;
    }
    return false;
}
u16* ppu_frame_buffer()
{
    if (ppu->frame_complete) {
        ppu->frame_complete = false;
        return ppu->frame_buffer;
    }
    return NULL;
}
void ppu_clock() { 
     if (ppu->scanline == 241 && ppu->cycle == 1) {
        ppu->status.vertical_blank = 1;
        if (ppu->control.enable_nmi)
            ppu->nmi = true;
    }
    ppu->cycle++;
    if (ppu->cycle == 341) {
        ppu->cycle = 0;
        ppu->scanline++;
        if (ppu->scanline == 262) {
            ppu->scanline = 0;
        } else if (ppu->scanline == 261) {
            goto LAST_CYCLE_OF_FRAME;
        }
    }
    return;
LAST_CYCLE_OF_FRAME:
    if(ppu->mask.render_background) {
        for(int i = 0; i < 240; i++) {
            for(int j = 0; j < 256; j++) {
                u8 name = (ppu_intern_read(0x2000 | (((i >> 3) << 5) | (j >> 3))));
                // 生成渐变测试图案
                // u8 r = (j * 256 / 256) & 0xFF;  // 水平红色渐变
                // u8 g = (i * 256 / 240) & 0xFF;  // 垂直绿色渐变
                // u8 b = 128;                      // 固定蓝色
                
                // // 转换为 RGB565
                // u8 r5 = (r >> 3) & 0x1F;
                // u8 g5 = (g >> 2) & 0x3F;
                // u8 b5 = (b >> 3) & 0x1F;
                // ppu->frame_buffer[i * 256 + j] = (u16)((r5 << 11) | (g5 << 5) | b5);
                //ppu->frame_buffer[i * 256 + j] = (u16)(name);
                u8 r3 = (name >> 5) & 0x07;  // 高3位R（0~7）
                    u8 g3 = (name >> 2) & 0x07;  // 中间3位G（0~7）
                    u8 b2 = name & 0x03;         // 低2位B（0~3）
                    
                    // 扩展为RGB565各分量
                    u8 r5 = (r3 << 2) | (r3 >> 1);  // 3位→5位：左移2位 + 填充最高位的1位（r3>>1取最高位）
                    u8 g5 = (g3 << 3) | (g3);       // 3位→6位：左移3位 + 填充最高位的3位（g3本身重复）
                    u8 b5 = (b2 << 3) | (b2 << 1) | (b2 >> 1);  // 2位→5位：左移3位 + 填充最高位的2位
                    
                    // 组合为16位RGB565（高5位R，中6位G，低5位B）
                    ppu->frame_buffer[i * 256 + j] = (u16)((r5 << 11) | (g5 << 5) | b5);
            }
        }
        ppu->frame_complete = true;
    }
}