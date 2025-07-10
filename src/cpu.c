#include <stdio.h>
#include <string.h>

#include "../include/cpu.h"
#include "../include/memory.h"

OpInfo op_info[256];
u8 fetched = 0x00;
u8 page_acrossed = 0x00;


void cpu_init(CPU *cpu) {
    cpu->P = 0;
    cpu->A = 0;
    cpu->X = 0;
    cpu->Y = 0;
    cpu->S = 0;
    cpu->PC = 0;
    cpu->cycle = 0;
    set_flag(cpu, 0);
}

/*
    void set_flag(CPU *cpu, u8 flag, bool value)
    设置标志位
    flag: 标志位
*/

void set_flag(CPU *cpu, u8 flag) {
    cpu->P |= flag;
}

/*
    u8 get_flag(CPU *cpu)
    获取标志位
*/
u8 get_flag(CPU *cpu) {
    return cpu->P;
}

/*
    初始化操作表
    模拟6502芯片的指令集
*/
void init_op_table() {
    // BRK implied
    op_info[0x00] = (OpInfo){ OP_BRK, ADDR_IMPL, 7 };

    // ORA (X,ind)
    op_info[0x01] = (OpInfo){ OP_ORA, ADDR_INDX, 6 };

    // ORA zpg
    op_info[0x05] = (OpInfo){ OP_ORA, ADDR_ZP, 3 };

    // ASL zpg
    op_info[0x06] = (OpInfo){ OP_ASL, ADDR_ZP, 5 };

    // PHP implied
    op_info[0x08] = (OpInfo){ OP_PHP, ADDR_IMPL, 3 };

    // ORA #imm
    op_info[0x09] = (OpInfo){ OP_ORA, ADDR_IMM, 2 };

    // ASL A
    op_info[0x0A] = (OpInfo){ OP_ASL, ADDR_ACC, 2 };

    // ORA abs
    op_info[0x0D] = (OpInfo){ OP_ORA, ADDR_ABS, 4 };

    // ASL abs
    op_info[0x0E] = (OpInfo){ OP_ASL, ADDR_ABS, 6 };

    // BPL rel
    op_info[0x10] = (OpInfo){ OP_BPL, ADDR_REL, 2 }; // +1 if branch occurs, +2 if to new page

    // CLC implied
    op_info[0x18] = (OpInfo){ OP_CLC, ADDR_IMPL, 2 };

    // JSR abs
    op_info[0x20] = (OpInfo){ OP_JSR, ADDR_ABS, 6 };

    // AND (X,ind)
    op_info[0x21] = (OpInfo){ OP_AND, ADDR_INDX, 6 };

    // BIT zpg
    op_info[0x24] = (OpInfo){ OP_BIT, ADDR_ZP, 3 };

    // AND #imm
    op_info[0x29] = (OpInfo){ OP_AND, ADDR_IMM, 2 };

    // ROL A
    op_info[0x2A] = (OpInfo){ OP_ROL, ADDR_ACC, 2 };

    // PLP implied
    op_info[0x28] = (OpInfo){ OP_PLP, ADDR_IMPL, 4 };

    // BMI rel
    op_info[0x30] = (OpInfo){ OP_BMI, ADDR_REL, 2 }; // +1 if branch occurs, +2 if to new page

    // RTI implied
    op_info[0x40] = (OpInfo){ OP_RTI, ADDR_IMPL, 6 };

    // EOR (X,ind)
    op_info[0x41] = (OpInfo){ OP_EOR, ADDR_INDX, 6 };

    // JMP abs
    op_info[0x4C] = (OpInfo){ OP_JMP, ADDR_ABS, 3 };

    // LDA #imm
    op_info[0xA9] = (OpInfo){ OP_LDA, ADDR_IMM, 2 };

    // STA abs
    op_info[0x8D] = (OpInfo){ OP_STA, ADDR_ABS, 4 };

    // NOP implied
    op_info[0xEA] = (OpInfo){ OP_NOP, ADDR_IMPL, 2 };

}

u8 fetch(CPU *cpu) {
    return memory_read(cpu->PC++);
}


/*获取二进制指令的操作码*/
OpInfo get_op(CPU *cpu) {
    u8 opcode = fetch(cpu);
    OpInfo op  = op_info[opcode];
    return op;
}

/*根据指令的寻址方式来取指令*/
u16 get_operand_address(CPU *cpu, AddrMode mode) {
    switch (mode) {
        case ADDR_ACC:
            fetched = cpu->A;
            return 0;
        case ADDR_ABS: {
            u8 lo = memory_read(cpu->PC++);
            u8 hi = memory_read(cpu->PC++);
            return (hi << 8) | lo;
        }
        case ADDR_ABX: {
            u8 lo = memory_read(cpu->PC++);
            u8 hi = memory_read(cpu->PC++);
            u16 abs = (hi << 8) | lo;
            u16 final_addr = abs + cpu->X;
            if((final_addr & 0xFF00) != (abs & 0xFF00)) {
                page_acrossed = 1;
            } else {
                page_acrossed = 0;
            }
            return final_addr;
        }
        case ADDR_ABY: {
            u8 lo = memory_read(cpu->PC++);
            u8 hi = memory_read(cpu->PC++);
            u16 abs = (hi << 8) | lo;
            u16 final_addr = abs + cpu->Y;
            if((final_addr & 0xFF00) != (abs & 0xFF00)) {
                page_acrossed = 1;
            } else {
                page_acrossed = 0;
            }
            return final_addr;
        }
        case ADDR_IMM:
            fetched = memory_read(cpu->PC++); // 立即数，操作数在下一字节
            return 0; 
        case ADDR_IMPL:
            return 0;//隐含寻址
        case ADDR_IND: {
            u8 lo = memory_read(cpu->PC++);
            u8 hi = memory_read(cpu->PC++);
            u16 ptr = (hi << 8) | lo;
            u8 nlo = memory_read(ptr);
            u8 nhi;
            if ((ptr & 0x00FF) == 0x00FF) {
                // 间接寻址模拟6502 bug: 高字节从本页开头取，https://www.nesdev.org/obelisk-6502-guide/reference.html#JMP
                nhi = memory_read(ptr & 0xFF00);
            } else {
                nhi = memory_read(ptr + 1);
            }
            return (nhi << 8) | nlo;
        }
        case ADDR_INDX: {
            u8 zp_addr = memory_read(cpu->PC++);
            u8 ptr = (zp_addr + cpu->X) & 0xFF; // 零页循环
            u8 lo = memory_read(ptr);
            u8 hi = memory_read((ptr + 1) & 0xFF); 
            return (hi << 8) | lo;
        }
        case ADDR_INDY: {
            u8 zp_addr = memory_read(cpu->PC++);
            u8 lo = memory_read(zp_addr);
            u8 hi = memory_read((zp_addr + 1) & 0xFF); 
            u16 base = (hi << 8) | lo;
            u16 final_addr = base + cpu->Y;
            if ((final_addr & 0xFF00) != (base & 0xFF00)) {
                page_acrossed = 1;
            } else {
                page_acrossed = 0;
            }
            return final_addr;
        }        
        case ADDR_ZP:
            return memory_read(cpu->PC++); // 零页寻址，返回8位地址
        case ADDR_ZPX: {
            u8 base = memory_read(cpu->PC++);
            return (base + cpu->X) & 0xFF; 
        }
        case ADDR_ZPY: {
            u8 base = memory_read(cpu->PC++);
            return (base + cpu->Y) & 0xFF; 
        }
        case ADDR_REL: {
            u8 offset = memory_read(cpu->PC++);
            // 6502的REL是带符号的8位偏移
            if (offset & 0x80) {
                // 负数，补码扩展
                return cpu->PC + (offset | 0xFF00);
            } else {
                // 正数
                return cpu->PC + offset;
            }
        }
        default:
            return 0;
    }
}

u8 fetch_op_num(u16 addr) {
    if(addr == (u16)0) {
        return fetched;
    }else {
        return memory_read(addr);
    }
}

void run_instruction(CPU *cpu, OpType op, u16 addr, u8 num) {
    switch(op) {
        case OP_LDA:

        default:
            break;
    }
}



/*模拟CPU的执行过程*/
void cpu_run(CPU *cpu) {
    OpInfo op = get_op(cpu);    //取指令
    u16 addr = get_operand_address(cpu, op.addr_mode);  //译码
    u8 num = fetch_op_num(addr);
    run_instruction(cpu, op.op, addr, num);  //执行
}