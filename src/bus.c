#include "../include/bus.h"
#include "../include/cpu.h"
#include "../include/cartridge.h"
#include "../include/ppu.h"

/* 
    void cpu_write(u16 addr, u8 data);
    向内存指定区域写入
*/
void cpu_write(u16 addr, u8 data) {
    if(addr >= 0x0000 && addr <= 0x1FFF) {
        cpuRam[addr & 0x07FF] = data; // 2KB RAM镜像
    } else if(addr >= 0x2000 && addr <= 0x3FFF) {
        // PPU寄存器镜像
        ppu_cpu_write(addr & 0x0007, data);
    }

}

u8 cpu_read(u16 addr) {
    u8 data = 0;
    if(addr >= 0x0000 && addr <= 0x1FFF) { // 8KB范围 映射2KB ram镜像
        data = cpuRam[addr & 0x07FF]; // 2KB RAM镜像
    } else if(addr >= 0x2000 && addr <= 0x3FFF) {
        // PPU寄存器镜像
        data = ppu_cpu_read(addr & 0x0007, false);
    } else if (addr >= 0x4000 && addr <= 0x4017) {
        
    } else if (addr >= 0x4018 && addr <= 0x401F) {

    } else if (addr >= 0x4020 && addr <= 0x5FFF) {

    } else if (addr >= 0x6000 && addr <= 0x7FFF) {

    }else if (addr >= 0x8000 && addr <= 0xFFFF) {
        // 从cartridge读取PRG-ROM数据
        data = cartridge_cpu_read(addr);
    }
    return data;
}