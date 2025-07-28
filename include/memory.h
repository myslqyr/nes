#ifndef MEMORY_H
#define MEMORY_H

#include "type.h"


/*
$0000-$07FF  2KB RAM
$0800-$1FFF  RAM镜像
$2000-$2007  PPU寄存器
$2008-$3FFF  PPU寄存器镜像
$4000-$4017  APU/I/O寄存器
$4018-$401F  测试用途
$4020-$5FFF  扩展
$6000-$7FFF  SRAM
$8000-$FFFF  PRG-ROM
*/
#define NES_MEM_SIZE (1 << 16)   //NES内存分布
extern u8 memory[NES_MEM_SIZE];

void memory_init();

//模拟bus连接cpu与memory的通信
void memory_write(u16 addr, u8 data);
u8 memory_read(u16 addr);

u8 load_rom(const char *filename);

#endif