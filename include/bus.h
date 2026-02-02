#ifndef BUS_H
#define BUS_H

#include "type.h"
#include "cpu.h"
#include "ppu.h"

void cpu_write(u16 addr, u8 data);
u8 cpu_read(u16 addr);
void cpu_write(u16 addr, u8 data);
u8 cpu_read(u16 addr);

void bus_clock(CPU *cpu,PPU *ppu);

#endif