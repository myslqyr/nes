#include <stdio.h>
#include <stdlib.h>

#include "../include/cpu.h"
#include "../include/memory.h"
#include "../include/disassembly.h"
#include <string.h>

int main() {
    CPU *cpu = (CPU *)malloc(sizeof(CPU));
    if (cpu == NULL) {
        printf("Failed to allocate memory for CPU\n");
        return 1;
    }

    init_op_table();
    memory_init();
    debug_log_init();

    printf("输入ROM文件路径运行游戏:\n");

    char input[100];
    //scanf("%s", input);
    strcpy(input, "/home/myslqyr/czh/NES/rom/mario.nes");
    u8 ret = load_rom(input);
    if(ret != 0) {
        printf("failed to load rom: %s\n", input);
        return 1;
    }

    // 在ROM加载后初始化CPU，这样reset()才能读取正确的向量
    cpu_init(cpu);
    printf("开始运行游戏...\n");
    printf("按Ctrl+C停止\n\n");
    while(1) {
        cpu_run(cpu);
    }
    debug_log_close();
    free(cpu);  // 清理内存
    return 0;
}