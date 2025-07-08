#include "../include/memory.h"
#include <assert.h>

u8 memory[NES_MEM_SIZE];

void memory_init() {
    for(int i = 0; i < NES_MEM_SIZE; i++) {
        memory[i] = (u8)0x00;
    }
}

/* 
    void memory_write(u16 addr, u8 data);
    向内存指定区域写入数字
*/
void memory_write(u16 addr, u8 data) {
    assert(addr <=0xFFFF && addr >= 0x0000);
    memory[addr] = data;
}

/*
    u8 memory_read(u16 addr);
    从指定地址读出一个字节的数据
*/
u8 memory_read(u16 addr) {
    assert(addr <=0xFFFF && addr >= 0x0000);
    u8 data = memory[addr];
    return data;
}