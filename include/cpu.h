#ifndef CPU_H
#define CPU_H

#include "type.h"
#include <stdbool.h>

typedef struct {
    // enum {
    //     C = (1 << 0),       // 进位标志
    //     Z = (1 << 1),       // 零标志
    //     I = (1 << 2),       // 中断禁止标志
    //     D = (1 << 3),       // 十进制模式标志
    //     B = (1 << 4),       // bank指令标志
    //     V = (1 << 6),       // 溢出标志
    //     N = (1 << 7),       // 负数标志
    // } P;    //https://www.cnblogs.com/Abraverman/articles/15659200.html
    u8 P;    // 程序状态字
    u8 A;    // 累加器
    u8 X, Y; // 索引寄存器
    u8 S;    // 堆栈指针
    u16 PC;  // 程序计数器
} CPU;

void cpu_init(CPU *cpu);
void set_flag(CPU *cpu, u8 flag, bool value);
#endif
