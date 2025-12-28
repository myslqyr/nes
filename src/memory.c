#include <stdio.h>
#include <string.h>

#include "../include/memory.h"
#include <assert.h>

u8 memory[NES_MEM_SIZE];

void memory_init() {
    for(int i = 0; i < NES_MEM_SIZE; i++) {
        memory[i] = (u8)0x00;
    }
}


/*读取rom到内存中*/
u8 load_rom(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if(fp == NULL) {
        printf("failed to open rom:%s\n",filename);
        return -1;
    }

    //rom头
    u8 rom_head[16];
    fread(rom_head, 1, 16, fp);
    int prg_size = rom_head[4] * 16 * 1024; // PRG-ROM大小
    fseek(fp, 16, SEEK_SET);
    fread(&memory[0x8000], 1, prg_size, fp);
    if(prg_size == 16 * 1024) {
        memcpy(&memory[0xC000], &memory[0x8000], 16 * 1024);
    }
    fclose(fp);

    // for (int i = 0; i < 16; i++) {
    //     printf("rom head[%d]:0x%02X\n", i, rom_head[i]);
    // }
    // for (int i = 0; i < 16; i++) {
    //     printf("memory[0x8000 + %d]:0x%02X\n", i, memory[0x8000 + i]);
    // }

    return 0;
}
