#ifndef DISASSEMBLY_H
#define DISASSEMBLY_H

#include "cpu.h"
#include "memory.h"

// 反汇编一条指令
void disassemble_instruction(CPU *cpu);

// 格式化操作数
const char* format_operand(CPU *cpu, OpInfo op, u16 addr);

// 反汇编一段内存区域
void disassemble_range(CPU *cpu, u16 start_addr, u16 end_addr);

#endif