#ifndef PPU_H
#define PPU_H

#include "type.h"

u8 ppu_read(u16 addr);
void ppu_write(u16 addr, u8 data);//与ppu总线通信

u8 ppu_cpu_read(u16 addr);
void ppu_cpu_write(u16 addr, u8 data);//将ppu连接到cpu总线上,与总主线通信

u8 tblName[2][1024]; // 名称表
u8 tblPalette[32];   // 调色板
//u8 tblPattern[2][4096]; // 模式表


#endif