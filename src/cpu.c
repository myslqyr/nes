#include <stdio.h>
#include <string.h>

#include "../include/cpu.h"
#include "../include/memory.h"
#include "../include/irq.h"

OpInfo op_info[256];
u8 fetched = 0x00;
u8 page_acrossed = 0x00;

// 堆栈操作辅助函数,堆栈固定在第一个页中
void push_stack(CPU *cpu, u8 value) {
    memory_write(0x100 + cpu->S, value);
    cpu->S--;
}

void push_stack16(CPU *cpu, u16 value) {
    push_stack(cpu, (value >> 8) & 0xFF);  // 高字节
    push_stack(cpu, value & 0xFF);         // 低字节
}

u8 pull_stack(CPU *cpu) {
    cpu->S++;
    return memory_read(0x100 + cpu->S);
}

u16 pull_stack16(CPU *cpu) {
    u8 lo = pull_stack(cpu);
    u8 hi = pull_stack(cpu);
    return (hi << 8) | lo;
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

    for(int i = 0; i < 256; i++) {
        op_info[i] = (OpInfo){ OP_NOP, ADDR_IMPL, 2, "NOP" };
    }

/*加法运算*/
    /*带进位加法*/
    op_info[0x69] = (OpInfo){ OP_ADC, ADDR_IMM, 2, "ADC" };
    op_info[0x65] = (OpInfo){ OP_ADC, ADDR_ZP, 3, "ADC" };
    op_info[0x6D] = (OpInfo){ OP_ADC, ADDR_ABS, 4, "ADC" };
    op_info[0x75] = (OpInfo){ OP_ADC, ADDR_ZPX, 4, "ADC" };
    op_info[0x7D] = (OpInfo){ OP_ADC, ADDR_ABX, 4, "ADC" };
    op_info[0x79] = (OpInfo){ OP_ADC, ADDR_ABY, 4, "ADC" };
    op_info[0x61] = (OpInfo){ OP_ADC, ADDR_INDX, 6, "ADC" };
    op_info[0x71] = (OpInfo){ OP_ADC, ADDR_INDY, 5, "ADC" };

    /*存储器+1*/
    op_info[0xE6] = (OpInfo){ OP_INC, ADDR_ZP, 5, "INC" };
    op_info[0xF6] = (OpInfo){ OP_INC, ADDR_ZPX, 6, "INC" };
    op_info[0xEE] = (OpInfo){ OP_INC, ADDR_ABS, 6, "INC" };
    op_info[0xFE] = (OpInfo){ OP_INC, ADDR_ABX, 7, "INC" };

    /*X+1*/
    op_info[0xE8] = (OpInfo){ OP_INX, ADDR_IMPL, 2, "INX" };

    /*Y+1*/
    op_info[0xC8] = (OpInfo){ OP_INY, ADDR_IMPL, 2, "INY" };

//减法操作
    /*带借位减法*/
    op_info[0xE1] = (OpInfo){ OP_SBC, ADDR_INDX, 6, "SBC" };
    op_info[0xF1] = (OpInfo){ OP_SBC, ADDR_INDY, 5, "SBC" };
    op_info[0xE9] = (OpInfo){ OP_SBC, ADDR_IMM, 2, "SBC" };   
    op_info[0xE5] = (OpInfo){ OP_SBC, ADDR_ZP, 3, "SBC" };    
    op_info[0xF5] = (OpInfo){ OP_SBC, ADDR_ZPX, 4, "SBC" };   
    op_info[0xED] = (OpInfo){ OP_SBC, ADDR_ABS, 4, "SBC" }; 
    op_info[0xFD] = (OpInfo){ OP_SBC, ADDR_ABX, 4, "SBC" };   
    op_info[0xF9] = (OpInfo){ OP_SBC, ADDR_ABY, 4, "SBC" };   

    /*存储器-1*/
    op_info[0xC6] = (OpInfo){ OP_DEC, ADDR_ZP, 5, "DEC" };
    op_info[0xD6] = (OpInfo){ OP_DEC, ADDR_ZPX, 6, "DEC" };
    op_info[0xCE] = (OpInfo){ OP_DEC, ADDR_ABS, 6, "DEC" };
    op_info[0xDE] = (OpInfo){ OP_DEC, ADDR_ABX, 7, "DEC" };

    /*X-1*/
    op_info[0xCA] = (OpInfo){ OP_DEX, ADDR_IMPL, 2, "DEX" };

    /*Y-1*/
    op_info[0x88] = (OpInfo){ OP_DEY, ADDR_IMPL, 2, "DEY" };

//逻辑运算
    /*逻辑与*/
    op_info[0x29] = (OpInfo){ OP_AND, ADDR_IMM, 2, "AND" };
    op_info[0x25] = (OpInfo){ OP_AND, ADDR_ZP, 3, "AND" };
    op_info[0x2D] = (OpInfo){ OP_AND, ADDR_ABS, 4, "AND" };
    op_info[0x35] = (OpInfo){ OP_AND, ADDR_ZPX, 4, "AND" };
    op_info[0x3D] = (OpInfo){ OP_AND, ADDR_ABX, 4, "AND" };
    op_info[0x39] = (OpInfo){ OP_AND, ADDR_ABY, 4, "AND" };
    op_info[0x21] = (OpInfo){ OP_AND, ADDR_INDX, 6, "AND" };
    op_info[0x31] = (OpInfo){ OP_AND, ADDR_INDY, 5, "AND" };

    /*逻辑或*/
    op_info[0x09] = (OpInfo){ OP_ORA, ADDR_IMM, 2, "ORA" };
    op_info[0x05] = (OpInfo){ OP_ORA, ADDR_ZP, 3, "ORA" };
    op_info[0x0D] = (OpInfo){ OP_ORA, ADDR_ABS, 4, "ORA" };
    op_info[0x15] = (OpInfo){ OP_ORA, ADDR_ZPX, 4, "ORA" };
    op_info[0x1D] = (OpInfo){ OP_ORA, ADDR_ABX, 4, "ORA" };
    op_info[0x19] = (OpInfo){ OP_ORA, ADDR_ABY, 4, "ORA" };
    op_info[0x01] = (OpInfo){ OP_ORA, ADDR_INDX, 6, "ORA" };
    op_info[0x11] = (OpInfo){ OP_ORA, ADDR_INDY, 5, "ORA" };

    /*逻辑异或*/
    op_info[0x49] = (OpInfo){ OP_EOR, ADDR_IMM, 2, "EOR" };
    op_info[0x45] = (OpInfo){ OP_EOR, ADDR_ZP, 3, "EOR" };
    op_info[0x4D] = (OpInfo){ OP_EOR, ADDR_ABS, 4, "EOR" };
    op_info[0x55] = (OpInfo){ OP_EOR, ADDR_ZPX, 4, "EOR" };
    op_info[0x5D] = (OpInfo){ OP_EOR, ADDR_ABX, 4, "EOR" };
    op_info[0x59] = (OpInfo){ OP_EOR, ADDR_ABY, 4, "EOR" };
    op_info[0x41] = (OpInfo){ OP_EOR, ADDR_INDX, 6, "EOR" };
    op_info[0x51] = (OpInfo){ OP_EOR, ADDR_INDY, 5, "EOR" };

    /*位测试*/
    op_info[0x24] = (OpInfo){ OP_BIT, ADDR_ZP, 3, "BIT" };
    op_info[0x2C] = (OpInfo){ OP_BIT, ADDR_ABS, 4, "BIT" };

//移位
    /*左移*/
    op_info[0x0A] = (OpInfo){ OP_ASL, ADDR_ACC, 2, "ASL" };
    op_info[0x06] = (OpInfo){ OP_ASL, ADDR_ZP, 5, "ASL" };
    op_info[0x0E] = (OpInfo){ OP_ASL, ADDR_ABS, 6, "ASL" };
    op_info[0x16] = (OpInfo){ OP_ASL, ADDR_ZPX, 6, "ASL" };
    op_info[0x1E] = (OpInfo){ OP_ASL, ADDR_ABX, 7, "ASL" };

    /*右移*/
    op_info[0x4A] = (OpInfo){ OP_LSR, ADDR_ACC, 2, "LSR" };
    op_info[0x46] = (OpInfo){ OP_LSR, ADDR_ZP, 5, "LSR" };
    op_info[0x4E] = (OpInfo){ OP_LSR, ADDR_ABS, 6, "LSR" };
    op_info[0x56] = (OpInfo){ OP_LSR, ADDR_ZPX, 6, "LSR" };
    op_info[0x5E] = (OpInfo){ OP_LSR, ADDR_ABX, 7, "LSR" };

    /*循环左移*/
    op_info[0x2A] = (OpInfo){ OP_ROL, ADDR_ACC, 2, "ROL" };
    op_info[0x26] = (OpInfo){ OP_ROL, ADDR_ZP, 5, "ROL" };
    op_info[0x2E] = (OpInfo){ OP_ROL, ADDR_ABS, 6, "ROL" };
    op_info[0x36] = (OpInfo){ OP_ROL, ADDR_ZPX, 6, "ROL" };
    op_info[0x3E] = (OpInfo){ OP_ROL, ADDR_ABX, 7, "ROL" };

    /*循环右移*/
    op_info[0x6A] = (OpInfo){ OP_ROR, ADDR_ACC, 2, "ROR" };
    op_info[0x66] = (OpInfo){ OP_ROR, ADDR_ZP, 5, "ROR" };
    op_info[0x6E] = (OpInfo){ OP_ROR, ADDR_ABS, 6, "ROR" };
    op_info[0x76] = (OpInfo){ OP_ROR, ADDR_ZPX, 6, "ROR" };
    op_info[0x7E] = (OpInfo){ OP_ROR, ADDR_ABX, 7, "ROR" };

//条件分支
    /*C*=0*/
    op_info[0x90] = (OpInfo){ OP_BCC, ADDR_REL, 2, "BCC" };

    /*C*=1*/
    op_info[0xB0] = (OpInfo){ OP_BCS, ADDR_REL, 2, "BCS" };

    /*Z*=0*/
    op_info[0xD0] = (OpInfo){ OP_BNE, ADDR_REL, 2, "BNE" };

    /*Z=1*/
    op_info[0xF0] = (OpInfo){ OP_BEQ, ADDR_REL, 2, "BEQ" };

    /*N*=0*/
    op_info[0x10] = (OpInfo){ OP_BPL, ADDR_REL, 2, "BPL" };

    /*N*=1*/
    op_info[0x30] = (OpInfo){ OP_BMI, ADDR_REL, 2, "BMI" };

    /*V*=0*/
    op_info[0x50] = (OpInfo){ OP_BVC, ADDR_REL, 2, "BVC" };

    /*V*=1*/ 
    op_info[0x70] = (OpInfo){ OP_BVS, ADDR_REL, 2, "BVS" };

//强制转移
    /*强制暂停*/
    op_info[0x00] = (OpInfo){ OP_BRK, ADDR_IMPL, 7, "BRK" };

    /*转移*/
    op_info[0x4C] = (OpInfo){ OP_JMP, ADDR_ABS, 3, "JMP" };
    op_info[0x6C] = (OpInfo){ OP_JMP, ADDR_IND, 5, "JMP" };

    /*转子*/
    op_info[0x20] = (OpInfo){ OP_JSR, ADDR_ABS, 6, "JSR" };

    /*中断返回*/
    op_info[0x40] = (OpInfo){ OP_RTI, ADDR_IMPL, 6, "RTI" };

    /*子程序返回*/
    op_info[0x60] = (OpInfo){ OP_RTS, ADDR_IMPL, 6, "RTS" };

//设置状态处理器
    /*清除进位标志*/
    op_info[0x18] = (OpInfo){ OP_CLC, ADDR_IMPL, 2, "CLC" };

    /*清十进制标志*/
    op_info[0xD8] = (OpInfo){ OP_CLD, ADDR_IMPL, 2, "CLD" };

    /*清除溢出标志*/
    op_info[0xB8] = (OpInfo){ OP_CLV, ADDR_IMPL, 2, "CLV" };

    /*清除中断标志*/
    op_info[0x58] = (OpInfo){ OP_CLI, ADDR_IMPL, 2, "CLI" };

    /*设置进位标志*/
    op_info[0x38] = (OpInfo){ OP_SEC, ADDR_IMPL, 2, "SEC" };

    /*设置十进制标志*/
    op_info[0xF8] = (OpInfo){ OP_SED, ADDR_IMPL, 2, "SED" };

    /*设置溢出标志*/
    op_info[0x78] = (OpInfo){ OP_SEI, ADDR_IMPL, 2, "SEI" };

//比较，M为存储器
    /*A-M比较*/
    op_info[0xC9] = (OpInfo){ OP_CMP, ADDR_IMM, 2, "CMP" };
    op_info[0xC5] = (OpInfo){ OP_CMP, ADDR_ZP, 3, "CMP" };
    op_info[0xCD] = (OpInfo){ OP_CMP, ADDR_ABS, 4, "CMP" };
    op_info[0xD5] = (OpInfo){ OP_CMP, ADDR_ZPX, 4, "CMP" };
    op_info[0xDD] = (OpInfo){ OP_CMP, ADDR_ABX, 4, "CMP" };
    op_info[0xD9] = (OpInfo){ OP_CMP, ADDR_ABY, 4, "CMP" };
    op_info[0xC1] = (OpInfo){ OP_CMP, ADDR_INDX, 6, "CMP" };
    op_info[0xD1] = (OpInfo){ OP_CMP, ADDR_INDY, 5, "CMP" };

    /*X-M比较*/
    op_info[0xE0] = (OpInfo){ OP_CPX, ADDR_IMM, 2, "CPX" };
    op_info[0xE4] = (OpInfo){ OP_CPX, ADDR_ZP, 3, "CPX" };
    op_info[0xEC] = (OpInfo){ OP_CPX, ADDR_ABS, 4, "CPX" };

    /*Y-M比较*/
    op_info[0xC0] = (OpInfo){ OP_CPY, ADDR_IMM, 2, "CPY" };
    op_info[0xC4] = (OpInfo){ OP_CPY, ADDR_ZP, 3, "CPY" };
    op_info[0xCC] = (OpInfo){ OP_CPY, ADDR_ABS, 4, "CPY" };

//寄存器相关
    /*M->A*/
    op_info[0xA9] = (OpInfo){ OP_LDA, ADDR_IMM, 2, "LDA" };
    op_info[0xA5] = (OpInfo){ OP_LDA, ADDR_ZP, 3, "LDA" };
    op_info[0xAD] = (OpInfo){ OP_LDA, ADDR_ABS, 4, "LDA" };
    op_info[0xB5] = (OpInfo){ OP_LDA, ADDR_ZPX, 4, "LDA" };
    op_info[0xBD] = (OpInfo){ OP_LDA, ADDR_ABX, 4, "LDA" };
    op_info[0xB9] = (OpInfo){ OP_LDA, ADDR_ABY, 4, "LDA" };
    op_info[0xA1] = (OpInfo){ OP_LDA, ADDR_INDX, 6, "LDA" };
    op_info[0xB1] = (OpInfo){ OP_LDA, ADDR_INDY, 5, "LDA" };

    /*M->X*/
    op_info[0xA2] = (OpInfo){ OP_LDX, ADDR_IMM, 2, "LDX" };
    op_info[0xA6] = (OpInfo){ OP_LDX, ADDR_ZP, 3, "LDX" };
    op_info[0xAE] = (OpInfo){ OP_LDX, ADDR_ABS, 4, "LDX" };
    op_info[0xB6] = (OpInfo){ OP_LDX, ADDR_ZPX, 4, "LDX" };
    op_info[0xBE] = (OpInfo){ OP_LDX, ADDR_ABY, 4, "LDX" };

    /*M->Y*/
    op_info[0xA0] = (OpInfo){ OP_LDY, ADDR_IMM, 2, "LDY" };
    op_info[0xA4] = (OpInfo){ OP_LDY, ADDR_ZP, 3, "LDY" };
    op_info[0xAC] = (OpInfo){ OP_LDY, ADDR_ABS, 4, "LDY" };
    op_info[0xB4] = (OpInfo){ OP_LDY, ADDR_ZPX, 4, "LDY" };
    op_info[0xBC] = (OpInfo){ OP_LDY, ADDR_ABX, 4, "LDY" };

    /*A->M*/
    op_info[0x85] = (OpInfo){ OP_STA, ADDR_ZP, 3, "STA" };
    op_info[0x8D] = (OpInfo){ OP_STA, ADDR_ABS, 4, "STA" };
    op_info[0x95] = (OpInfo){ OP_STA, ADDR_ZPX, 4, "STA" };
    op_info[0x9D] = (OpInfo){ OP_STA, ADDR_ABX, 5, "STA" };
    op_info[0x99] = (OpInfo){ OP_STA, ADDR_ABY, 5, "STA" };
    op_info[0x81] = (OpInfo){ OP_STA, ADDR_INDX, 6, "STA" };
    op_info[0x91] = (OpInfo){ OP_STA, ADDR_INDY, 6, "STA" };

    /*X->M*/
    op_info[0x86] = (OpInfo){ OP_STX, ADDR_ZP, 3, "STX" };
    op_info[0x8E] = (OpInfo){ OP_STX, ADDR_ABS, 4, "STX" };
    op_info[0x96] = (OpInfo){ OP_STX, ADDR_ZPY, 4, "STX" };

    /*Y->M*/
    op_info[0x84] = (OpInfo){ OP_STY, ADDR_ZP, 3, "STY" };
    op_info[0x8C] = (OpInfo){ OP_STY, ADDR_ABS, 4, "STY" };
    op_info[0x94] = (OpInfo){ OP_STY, ADDR_ZPX, 4, "STY" };

    /*A->X*/
    op_info[0xAA] = (OpInfo){ OP_TAX, ADDR_IMPL, 2, "TAX" };

    /*A->Y*/
    op_info[0xA8] = (OpInfo){ OP_TAY, ADDR_IMPL, 2, "TAY" };

    /*S->X*/
    op_info[0xBA] = (OpInfo){ OP_TSX, ADDR_IMPL, 2, "TSX" };

    /*X->S*/
    op_info[0x9A] = (OpInfo){ OP_TXS, ADDR_IMPL, 2, "TXS" };

    /*X->A*/
    op_info[0x8A] = (OpInfo){ OP_TXA, ADDR_IMPL, 2, "TXA" };

    /*Y->A*/
    op_info[0x98] = (OpInfo){ OP_TYA, ADDR_IMPL, 2, "TYA" };

//堆栈操作
    /*A->S*/
    op_info[0x48] = (OpInfo){ OP_PHA, ADDR_IMPL, 3, "PHA" };

    /*S->A*/
    op_info[0x68] = (OpInfo){ OP_PLA, ADDR_IMPL, 4, "PLA" };

    /*P->S*/
    op_info[0x08] = (OpInfo){ OP_PHP, ADDR_IMPL, 3, "PHP" };

    /*S->P*/
    op_info[0x28] = (OpInfo){ OP_PLP, ADDR_IMPL, 4, "PLP" };

//空操作
    op_info[0xEA] = (OpInfo){ OP_NOP, ADDR_IMPL, 2, "NOP" };
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
        /*相对寻址主要实现分支和跳转指令*/
        case ADDR_REL: {
            u8 offset = memory_read(cpu->PC++);
            u16 base_pc = cpu->PC;
            // 6502的REL是带符号的8位偏移
            if (offset & 0x80) {
                // 负数，补码扩展
                base_pc = cpu->PC + (offset | 0xFF00);
            } else {
                // 正数
                base_pc = cpu->PC + offset;
            }
            // 检查是否跨页
            if ((base_pc & 0xFF00) != (cpu->PC & 0xFF00)) {
                page_acrossed = 2;  // 跨页加2个周期
            } else {
                page_acrossed = 1;  // 同页加1个周期
            }
            return base_pc;
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

//TODO:集中处理各个标志位
void set_cpu_p(CPU *cpu, u8 num) {
    return ;
}


void run_instruction(CPU *cpu, OpType op, u16 addr, u8 num) {
    switch(op) {
        //TODO:运算指令 需要考虑各个标志位
        /*加法运算*/
        case OP_ADC: {
            /*N，Z，C，V*/
            u16 result = (cpu->A + num + (cpu->P & FLAG_C));
            u8 old_a = cpu->A;
            cpu->A = result & 0xFF;
            
            // Carry flag
            cpu->P = (cpu->P & ~FLAG_C) | ((result >> 8) & FLAG_C);
            
            // Zero flag
            if(cpu->A == 0) {
                cpu->P |= FLAG_Z;
            } else {
                cpu->P &= ~FLAG_Z;
            }

            // Negative flag
            if(cpu->A & 0x80) {
                cpu->P |= FLAG_N;
            } else {
                cpu->P &= ~FLAG_N;
            }

            // Overflow flag#define FLAG_V  (1 << 6)  // 溢出标志
            // 检查原操作数A和加数的符号位是否相同，且结果的符号位与原操作数不同
            if(((old_a ^ num) & 0x80) == 0 && ((old_a ^ cpu->A) & 0x80) != 0) {
                cpu->P |= FLAG_V;
            } else {
                cpu->P &= ~FLAG_V;
            }
            break;
        }

            

        case OP_INC: {
            u8 result = num + 1;
            memory_write(addr, result);
            if(result == 0) {
                cpu->P |= FLAG_Z;
            } else {
                cpu->P &= ~FLAG_Z;
            }
            if(result & 0x80) {
                cpu->P |= FLAG_N;
            } else {
                cpu->P &= ~FLAG_N;
            }
            break;
        }

        case OP_INX: {
            cpu->X = cpu->X + 1;
            if(cpu->X == 0) {
                cpu->P |= FLAG_Z;
            } else {
                cpu->P &= ~FLAG_Z;
            }
            if(cpu->X & 0x80) {
                cpu->P |= FLAG_N;
            } else {
                cpu->P &= ~FLAG_N;
            }
            break;
        }

        case OP_INY: {
            cpu->Y = cpu->Y + 1;
            if(cpu->Y == 0) {
                cpu->P |= FLAG_Z;
            } else {
                cpu->P &= ~FLAG_Z;
            }
            if(cpu->Y & 0x80) {
                cpu->P |= FLAG_N;
            } else {
                cpu->P &= ~FLAG_N;
            }
            break;
        }

        /*减法运算*/
        case OP_SBC: {
            /*N，Z，C，V - 带借位减法 A = A - M - (1-C)*/
            // SBC等价于ADC with M取反
            u8 old_a = cpu->A;
            u16 temp = num ^ 0xFF;  // 取反M
            u16 result = cpu->A + temp + (cpu->P & FLAG_C);
            cpu->A = result & 0xFF;
            
            // Carry flag - SBC中C=1表示无借位，C=0表示有借位
            cpu->P = (cpu->P & ~FLAG_C) | ((result >> 8) & FLAG_C);
            
            // Zero flag
            if(cpu->A == 0) {
                cpu->P |= FLAG_Z;
            } else {
                cpu->P &= ~FLAG_Z;
            }

            // Negative flag
            if(cpu->A & 0x80) {
                cpu->P |= FLAG_N;
            } else {
                cpu->P &= ~FLAG_N;
            }

            // Overflow flag - 检查有符号减法溢出
            if(((old_a ^ temp) & 0x80) == 0 && ((old_a ^ cpu->A) & 0x80) != 0) {
                cpu->P |= FLAG_V;
            } else {
                cpu->P &= ~FLAG_V;
            }
            break;
        }

        case OP_DEC: {
            /*存储器-1*/
            u8 result = num - 1;
            memory_write(addr, result);
            
            // Zero flag
            if(result == 0) {
                cpu->P |= FLAG_Z;
            } else {
                cpu->P &= ~FLAG_Z;
            }
            
            // Negative flag
            if(result & 0x80) {
                cpu->P |= FLAG_N;
            } else {
                cpu->P &= ~FLAG_N;
            }
            break;
        }

        case OP_DEX: {
            /*X-1*/
            cpu->X = cpu->X - 1;
            
            // Zero flag
            if(cpu->X == 0) {
                cpu->P |= FLAG_Z;
            } else {
                cpu->P &= ~FLAG_Z;
            }
            
            // Negative flag
            if(cpu->X & 0x80) {
                cpu->P |= FLAG_N;
            } else {
                cpu->P &= ~FLAG_N;
            }
            break;
        }

        case OP_DEY: {
            /*Y-1*/
            cpu->Y = cpu->Y - 1;
            
            // Zero flag
            if(cpu->Y == 0) {
                cpu->P |= FLAG_Z;
            } else {
                cpu->P &= ~FLAG_Z;
            }
            
            // Negative flag
            if(cpu->Y & 0x80) {
                cpu->P |= FLAG_N;
            } else {
                cpu->P &= ~FLAG_N;
            }
            break;
        }

//逻辑运算
        case OP_AND: {
            cpu->A = cpu->A & num;
            
            // Zero flag
            if(cpu->A == 0) {
                cpu->P |= FLAG_Z;
            } else {
                cpu->P &= ~FLAG_Z;
            }
            
            // Negative flag
            if(cpu->A & 0x80) {
                cpu->P |= FLAG_N;
            } else {
                cpu->P &= ~FLAG_N;
            }
            break;
        }

        case OP_EOR: {
            cpu->A = cpu->A ^ num;
            
            // Zero flag
            if(cpu->A == 0) {
                cpu->P |= FLAG_Z;
            } else {
                cpu->P &= ~FLAG_Z;
            }
            
            // Negative flag
            if(cpu->A & 0x80) {
                cpu->P |= FLAG_N;
            } else {
                cpu->P &= ~FLAG_N;
            }
            break;
        }

        case OP_ORA: {
            cpu->A = cpu->A | num;
            
            // Zero flag
            if(cpu->A == 0) {
                cpu->P |= FLAG_Z;
            } else {
                cpu->P &= ~FLAG_Z;
            }
            
            // Negative flag
            if(cpu->A & 0x80) {
                cpu->P |= FLAG_N;
            } else {
                cpu->P &= ~FLAG_N;
            }
            break;
        }

        case OP_BIT: {
            u8 result = cpu->A & num;
            cpu->P = (cpu->P & ~FLAG_N) | (num & FLAG_N);  
            cpu->P = (cpu->P & ~FLAG_V) | (num & FLAG_V);  
            
            // Zero flag 基于AND结果
            if(result == 0) {
                cpu->P |= FLAG_Z;
            } else {
                cpu->P &= ~FLAG_Z;
            }
            break;
        }

//移位
        case OP_ASL: {
            /*算术左移 - 左移一位，最高位进入C标志*/
            u8 result = num << 1;
            
            // Carry flag - 原最高位进入进位标志
            cpu->P = (cpu->P & ~FLAG_C) | ((num & 0x80) ? FLAG_C : 0);
            
            // Zero flag
            if(result == 0) {
                cpu->P |= FLAG_Z;
            } else {
                cpu->P &= ~FLAG_Z;
            }
            
            // Negative flag
            if(result & 0x80) {
                cpu->P |= FLAG_N;
            } else {
                cpu->P &= ~FLAG_N;
            }
            
            // 写回结果
            if(addr == 0) {
                cpu->A = result;  // 累加器寻址
            } else {
                memory_write(addr, result);  // 内存寻址
            }
            break;
        }

        case OP_LSR: {
            /*逻辑右移 - 右移一位，最低位进入C标志*/
            u8 result = num >> 1;
            
            // Carry flag - 原最低位进入进位标志
            cpu->P = (cpu->P & ~FLAG_C) | (num & 0x01);
            
            // Zero flag
            if(result == 0) {
                cpu->P |= FLAG_Z;
            } else {
                cpu->P &= ~FLAG_Z;
            }
            
            // Negative flag - 逻辑右移后最高位总是0
            cpu->P &= ~FLAG_N;
            
            // 写回结果
            if(addr == 0) {
                cpu->A = result;  // 累加器寻址
            } else {
                memory_write(addr, result);  // 内存寻址
            }
            break;
        }

        case OP_ROL: {
            /*循环左移 - 左移一位，C标志进入最低位，最高位进入C标志*/
            u8 old_carry = (cpu->P & FLAG_C) ? 1 : 0;
            u8 result = (num << 1) | old_carry;
            
            // Carry flag - 原最高位进入进位标志
            cpu->P = (cpu->P & ~FLAG_C) | ((num & 0x80) ? FLAG_C : 0);
            
            // Zero flag
            if(result == 0) {
                cpu->P |= FLAG_Z;
            } else {
                cpu->P &= ~FLAG_Z;
            }
            
            // Negative flag
            if(result & 0x80) {
                cpu->P |= FLAG_N;
            } else {
                cpu->P &= ~FLAG_N;
            }
            
            // 写回结果
            if(addr == 0) {
                cpu->A = result;  // 累加器寻址
            } else {
                memory_write(addr, result);  // 内存寻址
            }
            break;
        }

        case OP_ROR: {
            /*循环右移 - 右移一位，C标志进入最高位，最低位进入C标志*/
            u8 old_carry = (cpu->P & FLAG_C) ? 0x80 : 0;
            u8 result = (num >> 1) | old_carry;
            
            // Carry flag - 原最低位进入进位标志
            cpu->P = (cpu->P & ~FLAG_C) | (num & 0x01);
            
            // Zero flag
            if(result == 0) {
                cpu->P |= FLAG_Z;
            } else {
                cpu->P &= ~FLAG_Z;
            }
            
            // Negative flag
            if(result & 0x80) {
                cpu->P |= FLAG_N;
            } else {
                cpu->P &= ~FLAG_N;
            }
            
            // 写回结果
            if(addr == 0) {
                cpu->A = result;  // 累加器寻址
            } else {
                memory_write(addr, result);  // 内存寻址
            }
            break;
        }
//条件分支
        case OP_BCC: {
            if((cpu->P & FLAG_C) == 0) {
                cpu->PC += num;
            }
            break;
        }
        case OP_BCS: {
            if((cpu->P & FLAG_C) != 0) {
                cpu->PC += num;
            }
            break;
        }
        case OP_BEQ: {  
            if((cpu->P & FLAG_Z) != 0) {
                cpu->PC += num;
            }
            break;
        }
        case OP_BMI: {  
            if((cpu->P & FLAG_N) != 0) {
                cpu->PC += num;
            }
            break;
        }
        case OP_BNE: {  
            if((cpu->P & FLAG_Z) == 0) {
                cpu->PC += num;
            }
            break;
        }
        case OP_BPL: {      
            if((cpu->P & FLAG_N) == 0) {
                cpu->PC += num;
            }
            break;
        }
        case OP_BVC: {  
            if((cpu->P & FLAG_V) == 0) {
                cpu->PC += num;
            }
            break;
        }
        case OP_BVS: {
            if((cpu->P & FLAG_V) != 0) {
                cpu->PC += num;
            }
            break;
        }

//强制转移指令
        case OP_BRK: {
            cpu->PC += 2;
            push_stack(cpu, (cpu->PC >> 8) & 0xFF);
            push_stack(cpu, cpu->PC & 0xFF);
            push_stack(cpu, (cpu->P & ~FLAG_B) | FLAG_I);
            cpu->PC = memory_read(0xFFFE) | (memory_read(0xFFFF) << 8);
            break;
        }
        case OP_JMP: {
            cpu->PC = addr;
            break;
        }

        case OP_LDA:
            cpu->A = num;
            break;

        default:
            break;
    }
}

OpInfo get_op_type(u8 instruction) {
    OpInfo op = op_info[(instruction & 0xFF)];
    return op;
}


/*模拟CPU的执行过程*/
void cpu_run(CPU *cpu) {
    // 检查中断
    check_interrupts(cpu);
    
    // 正常的指令执行
    OpInfo op = get_op(cpu);    //取指令
    u16 addr = get_operand_address(cpu, op.addr_mode);  //译码
    u8 num = fetch_op_num(addr);
    run_instruction(cpu, op.op, addr, num);  //执行
    cpu->cycle = cpu->cycle + op.cycles + page_acrossed;    //同步时钟周期
    page_acrossed = 0;
}

/*初始化cpu*/
void cpu_init(CPU *cpu) {
    cpu->P = FLAG_U;  // 未使用位总是1
    cpu->A = 0;
    cpu->X = 0;
    cpu->Y = 0;
    cpu->S = 0xFD;    // 初始化堆栈指针
    cpu->PC = 0;
    cpu->cycle = 0;
    cpu->nmi_pending = false;
    cpu->irq_pending = false;
    reset(cpu);
}