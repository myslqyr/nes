#ifndef MAPPER0_H
#define MAPPER0_H

#include "type.h"
#include <stdbool.h>

bool mapper0_cpu_map_read(u16 addr, int prg_rom_size, u32 *mapped_addr);
bool mapper0_cpu_map_write(u16 addr, int prg_rom_size, u32 *mapped_addr);
bool mapper0_ppu_map_read(u16 addr, int chr_rom_size, u32 *mapped_addr);
bool mapper0_ppu_map_write(u16 addr, int chr_rom_size, bool chr_is_ram, u32 *mapped_addr);

#endif
