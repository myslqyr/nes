#include <stdio.h>
#include <stdlib.h>

#include "../include/cpu.h"
#include "../include/memory.h"

int main() {
    // 分配 CPU 结构体内存
    CPU *cpu = (CPU *)malloc(sizeof(CPU));
    if (cpu == NULL) {
        printf("Failed to allocate memory for CPU\n");
        return 1;
    }

    cpu_init(cpu);
    init_op_table();
    memory_init();

    char *rom_path = malloc(100);
    printf("请输入ROM文件路径: ");
    scanf("%s", rom_path);
    u8 ret = load_rom(rom_path);    //将rom加载到内存中
    if(ret != 0) {
        printf("failed to load rom\n");
        return 1;
    }
    // load_rom("../nes-test/cpu_dummy_reads/cpu_dummy_reads.nes");

    while(1) {
        cpu_run(cpu);
    }

    free(cpu);  // 清理内存
    return 0;
}