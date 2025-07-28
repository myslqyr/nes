#include "../include/disassembly.h"
#include <stdio.h>
#include <string.h>

// 用于存储格式化的操作数
static char operand_buffer[32];

const char* format_operand(CPU *cpu, OpInfo op, u16 addr) {
    u8 pc = cpu->PC;
    switch(op.addr_mode) {
        case ADDR_IMM:
            snprintf(operand_buffer, sizeof(operand_buffer), "#$%02X", memory_read(pc + 1));
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
            i8 offset = (i8)memory_read(pc + 1);
            u16 target = pc + 2 + offset;
            snprintf(operand_buffer, sizeof(operand_buffer), "$%02X(%+d)", (u8)target, offset);
            break;
        }
        case ADDR_IND: {
            u16 ptr = (memory_read(pc + 2) << 8) | memory_read(pc + 1);
            snprintf(operand_buffer, sizeof(operand_buffer), "($%04X)", ptr);
            break;
        }
        case ADDR_INDX: {
            u8 zp = memory_read(pc + 1);
            snprintf(operand_buffer, sizeof(operand_buffer), "($%02X,X)", zp);
            break;
        }
        case ADDR_INDY: {
            u8 zp = memory_read(pc + 1);
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

void disassemble_instruction(CPU *cpu) {
    u16 pc = cpu->PC;
    u8 opcode = memory_read(pc);
    OpInfo op = get_op_type(opcode);
    u16 addr = get_operand_address(cpu, op.addr_mode);
    
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
            printf("%02X    ", memory_read(pc + 1));
            break;
        case ADDR_ABS:
        case ADDR_ABX:
        case ADDR_ABY:
        case ADDR_IND:
            printf("%02X %02X ", memory_read(pc + 1), memory_read(pc + 2));
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
        OpInfo op = get_op_type(memory_read(current));
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
