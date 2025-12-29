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
    int chr_size = rom_head[5] * 8 * 1024;  // CHR-ROM大小

    // 读取PRG-ROM
    fread(&memory[0x8000], 1, prg_size, fp);

    // 如果PRG-ROM只有16KB，镜像到0xC000
    if(prg_size == 16 * 1024) {
        memcpy(&memory[0xC000], &memory[0x8000], 16 * 1024);
    }

    // 跳过CHR-ROM数据 (如果有的话)
    if(chr_size > 0) {
        fseek(fp, chr_size, SEEK_CUR);
    }

    // 读取中断向量 (PRG-ROM的最后6字节)
    // NMI向量 (0xFFFA-0xFFFB)
    fread(&memory[0xFFFA], 1, 2, fp);
    // Reset向量 (0xFFFC-0xFFFD)
    fread(&memory[0xFFFC], 1, 2, fp);
    // IRQ向量 (0xFFFE-0xFFFF)
    fread(&memory[0xFFFE], 1, 2, fp);

    fclose(fp);

    // 调试输出：显示设置的向量
    printf("ROM loaded:\n");
    printf("NMI vector: 0x%02X%02X (0x%04X)\n", memory[0xFFFB], memory[0xFFFA], (memory[0xFFFB] << 8) | memory[0xFFFA]);
    printf("Reset vector: 0x%02X%02X (0x%04X)\n", memory[0xFFFD], memory[0xFFFC], (memory[0xFFFD] << 8) | memory[0xFFFC]);
    printf("IRQ vector: 0x%02X%02X (0x%04X)\n", memory[0xFFFF], memory[0xFFFE], (memory[0xFFFF] << 8) | memory[0xFFFE]);

    return 0;
}
