#ifndef PPU_H
#define PPU_H

#include "type.h"
#include <stdbool.h>

u8 ppu_read(u16 addr);
void ppu_write(u16 addr, u8 data);//与ppu总线通信

u8 ppu_cpu_read(u16 addr);
void ppu_cpu_write(u16 addr, u8 data);//将ppu连接到cpu总线上,与总主线通信

extern u8 tblName[2][1024]; // 名称表
extern u8 tblPalette[32];   // 调色板
//u8 tblPattern[2][4096]; // 模式表

/*cpu-ppu寄存器*/

union PPUCTRL //0x2000
	{
		struct
		{
			u8 nametable_x : 1;
			u8 nametable_y : 1;
			u8 increment_mode : 1;
			u8 pattern_sprite : 1;
			u8 pattern_background : 1;
			u8 sprite_size : 1;
			u8 slave_mode : 1; // unused
			u8 enable_nmi : 1;
		};

		u8 reg;
	};

union PPUMASK  //0x2001
	{
		struct
		{
			u8 grayscale : 1;
			u8 render_background_left : 1;
			u8 render_sprites_left : 1;
			u8 render_background : 1;
			u8 render_sprites : 1;
			u8 enhance_red : 1;
			u8 enhance_green : 1;
			u8 enhance_blue : 1;
		};

		u8 reg;
	};

union PPUSTATUS //0x2002
	{
		struct
		{
			u8 unused : 5;
			u8 sprite_overflow : 1;
			u8 sprite_zero_hit : 1;
			u8 vertical_blank : 1;
		};

		u8 reg;
	};





union loopy_register
{
	struct
	{

		u16 coarse_x : 5;
		u16 coarse_y : 5;
		u16 nametable_x : 1;
		u16 nametable_y : 1;
		u16 fine_y : 3;
		u16 unused : 1;
	};

	u16 reg;
};

typedef struct {
    // -------- 时序 --------
    u16 cycle;        // 0-340
    i16 scanline;     // -1 ~ 260
    u64 frame;

    // -------- 寄存器 --------
    union PPUCTRL control;   // $2000
    union PPUMASK mask;   // $2001
    union PPUSTATUS status; // $2002
    u8 oam_addr;   // $2003
    u8 oam_data;   // $2004
    u8 scroll; // $2005 (写 latch)
    u8 PPUADDR;   // $2006 (写 latch)
    u8 PPUDATA;   // $2007

    // -------- 内部状态 --------
    u16 vram_addr;   // 当前 VRAM 地址
    u16 tram_addr;   // 临时地址
    u8 fine_x;
    bool addr_latch;

    // -------- 内存 --------
    u8 vram[2048];    // Nametable
    u8 palette[32];
    u8 oam[256];

    // -------- 输出 --------
    u32 framebuffer[256 * 240];

    // -------- 中断 --------
    bool nmi;
} PPU;

extern PPU *ppu;
void ppu_init(PPU *ppu);


#endif