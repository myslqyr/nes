#include "../include/bus.h"
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