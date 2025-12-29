#ifndef DISASSEMBLY_H
#define DISASSEMBLY_H

#include "cpu.h"
#include "memory.h"
#include <stddef.h>

// 反汇编一条指令
void disassemble_instruction(CPU *cpu);

// 格式化操作数
const char* format_operand(CPU *cpu, OpInfo op, u16 addr);

// 反汇编一段内存区域
void disassemble_range(CPU *cpu, u16 start_addr, u16 end_addr);

// 调试输出：显示指令信息和当前CPU状态
void debug_print_instruction(CPU *cpu, u8 opcode, OpInfo entry, u16 current_pc);

// 格式化操作数用于调试输出
void format_operand_for_debug(CPU *cpu, OpInfo entry, char *buffer, size_t buffer_size, u16 current_pc);

// 初始化调试日志文件
void debug_log_init();

// 关闭调试日志文件
void debug_log_close();

#endif