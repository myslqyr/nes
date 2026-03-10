#include "../include/type.h"
#include "../include/ppu.h"
#include "../include/cartridge.h"
#include <string.h>

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