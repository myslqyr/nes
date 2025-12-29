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

    // 调试测试模式
    printf("=== NES CPU 调试器 ===\n");
    printf("输入 'test' 进入测试模式，或输入ROM文件路径运行游戏:\n");

    char input[100];
    scanf("%s", input);

    if (strcmp(input, "test") == 0) {
        // 测试模式：执行一些简单指令
        printf("进入测试模式...\n");
        printf("PC:程序计数器  指令码  操作数  汇编指令  寄存器状态  标志位\n");
        printf("---------------------------------------------------------------\n");

        // 设置一些测试指令到内存
        // LDA #$42 (A9 42) - 加载立即数42到A寄存器
        memory[0x8000] = 0xA9;
        memory[0x8001] = 0x42;
        // STA $00 (85 00) - 将A保存到零页地址0x00
        memory[0x8002] = 0x85;
        memory[0x8003] = 0x00;
        // LDA $00 (A5 00) - 从零页地址0x00加载到A
        memory[0x8004] = 0xA5;
        memory[0x8005] = 0x00;
        // INX (E8) - X寄存器加1
        memory[0x8006] = 0xE8;
        // ADC #$01 (69 01) - A = A + 1 + 进位
        memory[0x8007] = 0x69;
        memory[0x8008] = 0x01;
        // BRK (00) - 中断
        memory[0x8009] = 0x00;

        // 设置复位向量指向我们的测试代码
        memory[0xFFFC] = 0x00;
        memory[0xFFFD] = 0x80;

        // 设置IRQ向量指向一个安全的地址（比如0x0000）
        memory[0xFFFE] = 0x00;
        memory[0xFFFF] = 0x00;

        // 在设置向量后初始化CPU，这样reset()才能读取正确的向量
        cpu_init(cpu);

        // 执行测试指令
        for(int i = 0; i < 20; i++) {
            cpu_run(cpu);
            if (cpu->PC >= 0x800A || (cpu->PC == 0x0000 && i > 5)) break; // BRK后停止
        }

        printf("测试完成\n");
        debug_log_close();
        return 0;
    } else {
        // 正常模式：加载ROM
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
    }

    debug_log_close();
    free(cpu);  // 清理内存
    return 0;
}