#include <stdio.h>
#include "../include/cpu.h"
#include "../include/memory.h"

// 中断处理函数
void reset(CPU *cpu) {
    // 读取复位向量，通电重置后会触发一个RESET中断，也就是会将CPU的指令指针寄存器PC(Program Counter)跳转到RESET中断存储的地址
    u8 lo = memory_read(RESET_VECTOR);
    u8 hi = memory_read(RESET_VECTOR + 1);
    cpu->PC = (hi << 8) | lo;
    
    // 初始化寄存器
    cpu->A = 0;
    cpu->X = 0;
    cpu->Y = 0;
    cpu->S = 0xFD;
    
    // 设置状态寄存器
    cpu->P = FLAG_U | FLAG_I;  // 设置中断禁止和未使用标志
    
    // 清除中断标志
    cpu->nmi_pending = false;
    cpu->irq_pending = false;
    
    // 复位需要7个时钟周期
    cpu->cycle += 7;
}

void irq(CPU *cpu) {
    // 如果中断禁止标志被设置，则忽略IRQ
    if (cpu->P & FLAG_I) {
        printf("IRQ中断被禁止\n");
        return;
    }
    
    // 保存当前程序计数器和状态寄存器
    push_stack16(cpu, cpu->PC);
    push_stack(cpu, cpu->P & ~FLAG_B);  // B标志在中断时被清除
    
    // 设置中断禁止标志
    cpu->P |= FLAG_I;
    
    // 读取中断向量
    u8 lo = memory_read(IRQ_VECTOR);
    u8 hi = memory_read(IRQ_VECTOR + 1);
    cpu->PC = (hi << 8) | lo;
    
    // IRQ需要7个时钟周期
    cpu->cycle += 7;
}

void nmi(CPU *cpu) {
    // NMI不能被禁止
    
    // 保存当前程序计数器和状态寄存器
    push_stack16(cpu, cpu->PC);
    push_stack(cpu, cpu->P & ~FLAG_B);  // B标志在中断时被清除
    
    // 设置中断禁止标志
    cpu->P |= FLAG_I;
    
    // 读取NMI向量
    u8 lo = memory_read(NMI_VECTOR);
    u8 hi = memory_read(NMI_VECTOR + 1);
    cpu->PC = (hi << 8) | lo;
    
    // NMI需要7个时钟周期
    cpu->cycle += 7;
}

// 在每个时钟周期检查中断
void check_interrupts(CPU *cpu) {
    if (cpu->nmi_pending) {
        nmi(cpu);
        cpu->nmi_pending = false;
    } else if (cpu->irq_pending && !(cpu->P & FLAG_I)) {
        irq(cpu);
        cpu->irq_pending = false;
    }
}
