#ifndef ROM_H 
#define ROM_H

#include "type.h"

struct romHead {
    char name[4];   // "NES" + 0x1A
    u8 prg_rom_chunks;  // 16KB为单位的PRG-ROM块数
    u8 chr_rom_chunks;  // 8KB为单位的CHR-ROM块数
    u8 mapper1;         // mapper信息
    u8 mapper2;        // mapper信息
    u8 prg_ram_size;    // 8KB为单位的PRG-RAM大小
    u8 tv_system1;      // TV系统（控制字节3）
    u8 tv_system2;    // TV系统（控制字节4）
    char unused[5]; // 未使用，填充为0
}; //NES ROM头结构体


u8 load_rom(const char *filename);



#endif