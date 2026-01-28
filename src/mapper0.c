#include "../include/mapper0.h"

bool mapper0_cpu_map_read(u16 addr, int prg_rom_size, u32 *mapped_addr) {
    if (addr >= 0x8000 && addr <= 0xFFFF) {
        int prg_banks = prg_rom_size / (16 * 1024);
        *mapped_addr = addr & (prg_banks > 1 ? 0x7FFF : 0x3FFF);
        return true;
    }
    return false;
}

bool mapper0_cpu_map_write(u16 addr, int prg_rom_size, u32 *mapped_addr) {
    return mapper0_cpu_map_read(addr, prg_rom_size, mapped_addr);
}

bool mapper0_ppu_map_read(u16 addr, int chr_rom_size, u32 *mapped_addr) {
    (void)chr_rom_size;
    if (addr <= 0x1FFF) {
        *mapped_addr = addr;
        return true;
    }
    return false;
}

bool mapper0_ppu_map_write(u16 addr, int chr_rom_size, bool chr_is_ram, u32 *mapped_addr) {
    (void)chr_rom_size;
    if (addr <= 0x1FFF && chr_is_ram) {
        *mapped_addr = addr;
        return true;
    }
    return false;
}
