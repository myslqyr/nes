#include "../include/disassembly.h"
#include "../include/bus.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

// 全局日志文件指针
static FILE *debug_log_file = NULL;

// 日志文件名
static const char *DEBUG_LOG_FILENAME = "nes_debug.log";

// 初始化调试日志文件
void debug_log_init() {
    debug_log_file = fopen(DEBUG_LOG_FILENAME, "w");
    if (debug_log_file == NULL) {
        fprintf(stderr, "Warning: Could not open debug log file %s\n", DEBUG_LOG_FILENAME);
        return;
    }

    // 写入日志文件头部信息
    time_t now = time(NULL);
    fprintf(debug_log_file, "=== NES CPU Debug Log ===\n");
    fprintf(debug_log_file, "Started at: %s", ctime(&now));
    fprintf(debug_log_file, "PC:程序计数器  指令码  操作数  汇编指令  寄存器状态  标志位\n");
    fprintf(debug_log_file, "---------------------------------------------------------------\n");
    fflush(debug_log_file);
}

// 关闭调试日志文件
void debug_log_close() {
    if (debug_log_file != NULL) {
        fprintf(debug_log_file, "\n=== Debug Log End ===\n");
        fclose(debug_log_file);
        debug_log_file = NULL;
        printf("Debug log saved to %s\n", DEBUG_LOG_FILENAME);
    }
}

// 用于存储格式化的操作数
static char operand_buffer[32];

const char* format_operand(CPU *cpu, OpInfo op, u16 addr) {
    u8 pc = cpu->PC;
    switch(op.addr_mode) {
        case ADDR_IMM:
            snprintf(operand_buffer, sizeof(operand_buffer), "#$%02X", cpu_read(pc + 1));
            break;
        case ADDR_ABS:
            snprintf(operand_buffer, sizeof(operand_buffer), "$%04X", addr);
            break;
        case ADDR_ABX:
            snprintf(operand_buffer, sizeof(operand_buffer), "$%04X,X", addr - cpu->X);
            break;
        case ADDR_ABY:
            snprintf(operand_buffer, sizeof(operand_buffer), "$%04X,Y", addr - cpu->Y);
            break;
        case ADDR_ZP:
            snprintf(operand_buffer, sizeof(operand_buffer), "$%02X", (u8)addr);
            break;
        case ADDR_ZPX:
            snprintf(operand_buffer, sizeof(operand_buffer), "$%02X,X", (u8)(addr - cpu->X));
            break;
        case ADDR_ZPY:
            snprintf(operand_buffer, sizeof(operand_buffer), "$%02X,Y", (u8)(addr - cpu->Y));
            break;
        case ADDR_REL: {
            // 相对寻址显示目标地址
            i8 offset = (i8)cpu_read(pc + 1);
            u16 target = pc + 2 + offset;
            snprintf(operand_buffer, sizeof(operand_buffer), "$%02X(%+d)", (u8)target, offset);
            break;
        }
        case ADDR_IND: {
            u16 ptr = (cpu_read(pc + 2) << 8) | cpu_read(pc + 1);
            snprintf(operand_buffer, sizeof(operand_buffer), "($%04X)", ptr);
            break;
        }
        case ADDR_INDX: {
            u8 zp = cpu_read(pc + 1);
            snprintf(operand_buffer, sizeof(operand_buffer), "($%02X,X)", zp);
            break;
        }
        case ADDR_INDY: {
            u8 zp = cpu_read(pc + 1);
            snprintf(operand_buffer, sizeof(operand_buffer), "($%02X),Y", zp);
            break;
        }
        case ADDR_ACC:
            snprintf(operand_buffer, sizeof(operand_buffer), "A");
            break;
        case ADDR_IMPL:
        default:
            operand_buffer[0] = '\0';
            break;
    }
    return operand_buffer;
}

static u16 peek_operand_address(CPU *cpu, u16 pc, AddrMode mode) {
    switch(mode) {
        case ADDR_IMM:
            return 0; // immediate has no address (operand at pc+1)
        case ADDR_ABS: {
            u8 lo = cpu_read(pc + 1);
            u8 hi = cpu_read(pc + 2);
            return (hi << 8) | lo;
        }
        case ADDR_ABX:
        case ADDR_ABY: {
            u8 lo = cpu_read(pc + 1);
            u8 hi = cpu_read(pc + 2);
            return (hi << 8) | lo; // disassembler reports base address
        }
        case ADDR_ZP:
        case ADDR_ZPX:
        case ADDR_ZPY:
        case ADDR_INDX:
        case ADDR_INDY:
        case ADDR_REL:
            // these encodings use a single extra byte
            return (u16)cpu_read(pc + 1);
        case ADDR_IND: {
            u8 lo = cpu_read(pc + 1);
            u8 hi = cpu_read(pc + 2);
            return (hi << 8) | lo;
        }
        case ADDR_ACC:
        case ADDR_IMPL:
        default:
            return 0;
    }
}

void disassemble_instruction(CPU *cpu) {
    u16 pc = cpu->PC;
    u8 opcode = cpu_read(pc);
    OpInfo op = get_op_type(opcode);
    u16 addr = peek_operand_address(cpu, pc, op.addr_mode);
    
    // 打印地址和操作码
    printf("%04X  %02X ", pc, opcode);
    
    // 根据寻址方式打印额外的字节
    switch(op.addr_mode) {
        case ADDR_IMM:
        case ADDR_ZP:
        case ADDR_ZPX:
        case ADDR_ZPY:
        case ADDR_INDX:
        case ADDR_INDY:
        case ADDR_REL:
            printf("%02X    ", cpu_read(pc + 1));
            break;
        case ADDR_ABS:
        case ADDR_ABX:
        case ADDR_ABY:
        case ADDR_IND:
            printf("%02X %02X ", cpu_read(pc + 1), cpu_read(pc + 2));
            break;
        default:
            printf("     ");
            break;
    }
    
    // 打印指令和操作数
    printf("%s ", op.name);
    if(op.addr_mode != ADDR_IMPL) {
        printf("%s", format_operand(cpu, op, addr));
    }
    printf("\n");
}

void disassemble_range(CPU *cpu, u16 start_addr, u16 end_addr) {
    u16 current = start_addr;
    while(current <= end_addr) {
        cpu->PC = current;
        disassemble_instruction(cpu);
        
        // 根据指令的寻址方式增加PC
        OpInfo op = get_op_type(cpu_read(current));
        switch(op.addr_mode) {
            case ADDR_ABS:
            case ADDR_ABX:
            case ADDR_ABY:
            case ADDR_IND:
                current += 3;
                break;
            case ADDR_IMM:
            case ADDR_ZP:
            case ADDR_ZPX:
            case ADDR_ZPY:
            case ADDR_INDX:
            case ADDR_INDY:
            case ADDR_REL:
                current += 2;
                break;
            default:
                current += 1;
                break;
        }
    }
}

// 调试函数：格式化操作数用于调试输出
void format_operand_for_debug(CPU *cpu, OpInfo entry, char *buffer, size_t buffer_size, u16 current_pc) {
    switch (entry.addr_mode) {
        case ADDR_IMPL:
            buffer[0] = '\0';
            break;
        case ADDR_ACC:
            snprintf(buffer, buffer_size, "A");
            break;
        case ADDR_IMM:
            snprintf(buffer, buffer_size, "#$%02X", cpu_read(current_pc + 1));
            break;
        case ADDR_ABS:
            {
                u8 lo = cpu_read(current_pc + 1);
                u8 hi = cpu_read(current_pc + 2);
                u16 addr = (hi << 8) | lo;
                snprintf(buffer, buffer_size, "$%04X", addr);
            }
            break;
        case ADDR_ABX:
            {
                u8 lo = cpu_read(current_pc + 1);
                u8 hi = cpu_read(current_pc + 2);
                u16 addr = (hi << 8) | lo;
                snprintf(buffer, buffer_size, "$%04X,X", addr);
            }
            break;
        case ADDR_ABY:
            {
                u8 lo = cpu_read(current_pc + 1);
                u8 hi = cpu_read(current_pc + 2);
                u16 addr = (hi << 8) | lo;
                snprintf(buffer, buffer_size, "$%04X,Y", addr);
            }
            break;
        case ADDR_ZP:
            snprintf(buffer, buffer_size, "$%02X", cpu_read(current_pc + 1));
            break;
        case ADDR_ZPX:
            snprintf(buffer, buffer_size, "$%02X,X", cpu_read(current_pc + 1));
            break;
        case ADDR_ZPY:
            snprintf(buffer, buffer_size, "$%02X,Y", cpu_read(current_pc + 1));
            break;
        case ADDR_IND:
            {
                u8 lo = cpu_read(current_pc + 1);
                u8 hi = cpu_read(current_pc + 2);
                u16 addr = (hi << 8) | lo;
                snprintf(buffer, buffer_size, "($%04X)", addr);
            }
            break;
        case ADDR_INDX:
            snprintf(buffer, buffer_size, "($%02X,X)", cpu_read(current_pc + 1));
            break;
        case ADDR_INDY:
            snprintf(buffer, buffer_size, "($%02X),Y", cpu_read(current_pc + 1));
            break;
        case ADDR_REL:
            {
                i8 offset = (i8)cpu_read(current_pc + 1);
                u16 target = current_pc + 2 + offset;
                snprintf(buffer, buffer_size, "$%04X", target);
            }
            break;
        default:
            snprintf(buffer, buffer_size, "???");
            break;
    }
}

// 调试函数：输出CPU状态和指令信息
void debug_print_instruction(CPU *cpu, u8 opcode, OpInfo entry, u16 current_pc) {
    char operand_str[32] = "";
    format_operand_for_debug(cpu, entry, operand_str, sizeof(operand_str), current_pc);

    // 创建完整的调试输出字符串
    char debug_line[256];
    int offset = 0;

    // 输出PC和指令信息
    offset += sprintf(debug_line + offset, "PC:%04X  %02X", current_pc, opcode);

    // 显示操作数字节
    switch (entry.addr_mode) {
        case ADDR_IMM:
        case ADDR_ZP:
        case ADDR_ZPX:
        case ADDR_ZPY:
        case ADDR_INDX:
        case ADDR_INDY:
        case ADDR_REL:
            offset += sprintf(debug_line + offset, " %02X      ", cpu_read(current_pc + 1));
            break;
        case ADDR_ABS:
        case ADDR_ABX:
        case ADDR_ABY:
        case ADDR_IND:
            offset += sprintf(debug_line + offset, " %02X %02X   ", cpu_read(current_pc + 1), cpu_read(current_pc + 2));
            break;
        case ADDR_IMPL:
        case ADDR_ACC:
        default:
            offset += sprintf(debug_line + offset, "         ");
            break;
    }

    // 输出汇编指令
    offset += sprintf(debug_line + offset, "%-4s ", entry.name);
    if (strlen(operand_str) > 0) {
        offset += sprintf(debug_line + offset, "%s", operand_str);
    }

    // 对于读取类指令，显示读取的值
    if (entry.op == OP_BIT || entry.op == OP_LDA || entry.op == OP_LDX || entry.op == OP_LDY ||
        entry.op == OP_CMP || entry.op == OP_CPX || entry.op == OP_CPY ||
        entry.op == OP_AND || entry.op == OP_ORA || entry.op == OP_EOR) {
        offset += sprintf(debug_line + offset, " = $%02X", cpu->fetched);
    }

    // 输出寄存器状态
    offset += sprintf(debug_line + offset, "    A:%02X X:%02X Y:%02X P:%02X SP:%02X",
                     cpu->A, cpu->X, cpu->Y, cpu->P, cpu->S);

    // 解释状态寄存器标志位
    offset += sprintf(debug_line + offset, " [");
    offset += sprintf(debug_line + offset, "%c", (cpu->P & FLAG_N) ? 'N' : 'n');
    offset += sprintf(debug_line + offset, "%c", (cpu->P & FLAG_V) ? 'V' : 'v');
    offset += sprintf(debug_line + offset, "%c", (cpu->P & FLAG_U) ? 'U' : 'u');
    offset += sprintf(debug_line + offset, "%c", (cpu->P & FLAG_B) ? 'B' : 'b');
    offset += sprintf(debug_line + offset, "%c", (cpu->P & FLAG_D) ? 'D' : 'd');
    offset += sprintf(debug_line + offset, "%c", (cpu->P & FLAG_I) ? 'I' : 'i');
    offset += sprintf(debug_line + offset, "%c", (cpu->P & FLAG_Z) ? 'Z' : 'z');
    offset += sprintf(debug_line + offset, "%c", (cpu->P & FLAG_C) ? 'C' : 'c');
    offset += sprintf(debug_line + offset, "]");

    // 添加PPU状态和周期计数 (简化版)
    offset += sprintf(debug_line + offset, " PPU:  0,  0 CYC:%u", cpu->cycle);

    offset += sprintf(debug_line + offset, "\n");

    // 输出到控制台
    printf("%s", debug_line);

    // 输出到日志文件
    if (debug_log_file != NULL) {
        fprintf(debug_log_file, "%s", debug_line);
        fflush(debug_log_file); // 立即刷新到文件
    }
}
