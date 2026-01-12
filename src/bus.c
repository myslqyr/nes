#include "../include/bus.h"
#include "../include/memory.h"
#include "../include/ppu.h"
#include <assert.h>

/* 
    void bus_write(u16 addr, u8 data);
    向内存指定区域写入
*/
void bus_write(u16 addr, u8 data) {
    if(addr <=0xFFFF && addr >= 0x0000)
        memory[addr] = data;
    return;
}

/*
    u8 bus_read(u16 addr);
    从指定地址读出一个字节的数据
*/
u8 bus_read(u16 addr) {
    if (addr <=0xFFFF && addr >= 0x0000) {
        return memory[addr];
    }
    return 0;
}

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
    if(addr >= 0x0000 && addr <= 0x1FFF) {
        data = cpuRam[addr & 0x07FF]; // 2KB RAM镜像
    } else if(addr >= 0x2000 && addr <= 0x3FFF) {
        // PPU寄存器镜像
        data = ppu_cpu_read(addr & 0x0007);
    }
    return data;
}