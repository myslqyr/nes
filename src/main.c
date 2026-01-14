#include <stdio.h>
#include <stdlib.h>

#include "../include/cpu.h"
#include "../include/cartridge.h"
#include "../include/disassembly.h"
#include "../include/ppu.h"
#include <string.h>

int main() {
    CPU *cpu = (CPU *)malloc(sizeof(CPU));
    PPU *ppu = (PPU *)malloc(sizeof(PPU));

    if (cpu == NULL || ppu == NULL) {
        printf("Failed to allocate memory for CPU or PPU\n");
        return 1;
    }

    init_op_table();
    debug_log_init();

    printf("输入ROM文件路径运行游戏:\n");

    char input[100];
    //scanf("%s", input);
    strcpy(input, "/home/myslqyr/czh/NES/rom/nestest.nes");
    bool ret = cartridge_load(input);
    if(!ret) {
        printf("failed to load rom: %s\n", input);
        return 1;
    }

    // 在ROM加载后初始化CPU，这样reset()才能读取正确的向量
    cpu_init(cpu);
    ppu_init(ppu);
    printf("开始运行游戏...\n");
    printf("按Ctrl+C停止\n\n");
    while(1) {
        cpu_run(cpu);
    }
    debug_log_close();
    free(cpu);  // 清理内存
    return 0;
}