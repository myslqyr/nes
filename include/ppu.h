#ifndef PPU_H
#define PPU_H

#include "type.h"
#include <stdbool.h>

/*cpu-ppu通信寄存器*/

typedef union {
struct {
    u8 unused          : 5;
    u8 sprite_overflow : 1;
    u8 sprite_hit_zero : 1;
    u8 vertical_blank  : 1;
};
    u8 reg;
} status_register;

typedef union {
struct {
    u8 grayscale                : 1;
    u8 render_background_left   : 1;
    u8 render_sprites_left      : 1;
    u8 render_background        : 1;
    u8 render_sprites           : 1;
    u8 enhance_red              : 1;
    u8 enhance_green            : 1;
    u8 enhance_blue             : 1;
};
    u8 reg;
} mask_regsiter;

typedef union {
struct {
    u8 nametable_x          : 1;
    u8 nametable_y          : 1;
    u8 increment_mode       : 1;
    u8 pattern_sprite       : 1;
    u8 pattern_background   : 1;
    u8 sprite_size          : 1;
    u8 slave_mode           : 1; // unused
    u8 enable_nmi           : 1;
};
    u8 reg;
} control_register;

typedef union  {
struct {
    u16 coarse_x            : 5;
    u16 coarse_y            : 5;
    u16 nametable_x         : 1;
    u16 nametable_y         : 1;
    u16 fine_y              : 3;
    u16 unused              : 1;
};
    u16 reg;
} loopy_address_register;

typedef struct {
    // -------- 时序 --------
    u16 cycle;        // 0-340  周期
    i16 scanline;     // -1 ~ 260  扫描线
    u64 frame;

    // -------- 寄存器 --------
    control_register control;   // $2000
    mask_regsiter mask;   // $2001
    status_register status; // $2002
    u8 oam_addr;   // $2003
    u8 oam_data;   // $2004
    u8 scroll; // $2005 (写 latch)
    u8 PPUADDR;   // $2006 (写 latch)
    u8 PPUDATA;   // $2007
	loopy_address_register vram_addr;
	loopy_address_register tram_addr;


    // -------- 内部状态 --------
    u8 latch_address;	//地址锁存器
    u8 ppu_data_buffer; //PPU数据缓存
	u16 ppu_addr;//保存编译好的内存地址

    // -------- 内存 --------
    u8 vram[2048];    // Nametable
    u8 palette[32];
    u8 oam[256];

    // -------- 输出 --------
	u8 data_buffer;
    u16 framebuffer[256 * 240];

    // -------- 中断 --------
    bool nmi;
} PPU;

u8 ppu_intern_read(u16 addr);
void ppu_intern_write(u16 addr, u8 data);//与ppu内部总线通信

u8 ppu_cpu_read(u16 addr);
void ppu_cpu_write(u16 addr, u8 data);//将ppu连接到cpu总线上,与总主线通信

void ppu_init();
void ppu_clock();


#endif