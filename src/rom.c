#include "../include/rom.h"
#include "../include/memory.h"
#include <stdio.h>
#include <string.h>

// ROM指游戏卡带而非内存中的ROM区域

/*读取rom到内存中,并完成一些初始化工作*/
u8 load_rom(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if(fp == NULL) {
        printf("failed to open rom:%s\n",filename);
        return -1;
    }

    // 读取 iNES 头
    struct romHead header;
    if (fread(&header, 1, sizeof(header), fp) != sizeof(header)) {
        printf("failed to read rom header\n");
        fclose(fp);
        return -1;
    }
    int prg_size = header.prg_rom_chunks * 16 * 1024; // PRG-ROM大小
    int chr_size = header.chr_rom_chunks * 8 * 1024;  // CHR-ROM大小

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

    fclose(fp);

    // 调试输出：显示设置的向量
    printf("ROM loaded:\n");
    printf("NMI vector: 0x%02X%02X (0x%04X)\n", memory[0xFFFB], memory[0xFFFA], (memory[0xFFFB] << 8) | memory[0xFFFA]);
    printf("Reset vector: 0x%02X%02X (0x%04X)\n", memory[0xFFFD], memory[0xFFFC], (memory[0xFFFD] << 8) | memory[0xFFFC]);
    printf("IRQ vector: 0x%02X%02X (0x%04X)\n", memory[0xFFFF], memory[0xFFFE], (memory[0xFFFF] << 8) | memory[0xFFFE]);

    return 0;
}
