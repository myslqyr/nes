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
/*加法运算*/
    /*带进位加法*/
    op_info[0x69] = (OpInfo){ OP_ADC, ADDR_IMM, 2 };
    op_info[0x65] = (OpInfo){ OP_ADC, ADDR_ZP, 3 };
    op_info[0x6D] = (OpInfo){ OP_ADC, ADDR_ABS, 4 };
    op_info[0x75] = (OpInfo){ OP_ADC, ADDR_ZPX, 4 };
    op_info[0x7D] = (OpInfo){ OP_ADC, ADDR_ABX, 4 };
    op_info[0x79] = (OpInfo){ OP_ADC, ADDR_ABY, 4 };
    op_info[0x61] = (OpInfo){ OP_ADC, ADDR_INDX, 6 };
    op_info[0x71] = (OpInfo){ OP_ADC, ADDR_INDY, 5 };

    /*存储器+1*/
    op_info[0xE6] = (OpInfo){ OP_INC, ADDR_ZP, 5 };
    op_info[0xF6] = (OpInfo){ OP_INC, ADDR_ZPX, 6 };
    op_info[0xEE] = (OpInfo){ OP_INC, ADDR_ABS, 6 };
    op_info[0xFE] = (OpInfo){ OP_INC, ADDR_ABX, 7 };

    /*X+1*/
    op_info[0xE8] = (OpInfo){ OP_INX, ADDR_IMPL, 2 };

    /*Y+1*/
    op_info[0xC8] = (OpInfo){ OP_INY, ADDR_IMPL, 2 };

//减法操作
    /*带借位减法*/
    op_info[0xE1] = (OpInfo){ OP_SBC, ADDR_INDX, 6 };
    op_info[0xF1] = (OpInfo){ OP_SBC, ADDR_INDY, 5 };
    op_info[0xE9] = (OpInfo){ OP_SBC, ADDR_IMM, 2 };   
    op_info[0xE5] = (OpInfo){ OP_SBC, ADDR_ZP, 3 };    
    op_info[0xF5] = (OpInfo){ OP_SBC, ADDR_ZPX, 4 };   
    op_info[0xED] = (OpInfo){ OP_SBC, ADDR_ABS, 4 }; 
    op_info[0xFD] = (OpInfo){ OP_SBC, ADDR_ABX, 4 };   
    op_info[0xF9] = (OpInfo){ OP_SBC, ADDR_ABY, 4 };   

    /*存储器-1*/
    op_info[0xC6] = (OpInfo){ OP_DEC, ADDR_ZP, 5 };
    op_info[0xD6] = (OpInfo){ OP_DEC, ADDR_ZPX, 6 };
    op_info[0xCE] = (OpInfo){ OP_DEC, ADDR_ABS, 6 };
    op_info[0xDE] = (OpInfo){ OP_DEC, ADDR_ABX, 7 };

    /*X-1*/
    op_info[0xCA] = (OpInfo){ OP_DEX, ADDR_IMPL, 2 };

    /*Y-1*/
    op_info[0x88] = (OpInfo){ OP_DEY, ADDR_IMPL, 2 };

//逻辑运算
    /*逻辑与*/
    op_info[0x29] = (OpInfo){ OP_AND, ADDR_IMM, 2 };
    op_info[0x25] = (OpInfo){ OP_AND, ADDR_ZP, 3 };
    op_info[0x2D] = (OpInfo){ OP_AND, ADDR_ABS, 4 };
    op_info[0x35] = (OpInfo){ OP_AND, ADDR_ZPX, 4 };
    op_info[0x3D] = (OpInfo){ OP_AND, ADDR_ABX, 4 };
    op_info[0x39] = (OpInfo){ OP_AND, ADDR_ABY, 4 };
    op_info[0x21] = (OpInfo){ OP_AND, ADDR_INDX, 6 };
    op_info[0x31] = (OpInfo){ OP_AND, ADDR_INDY, 5 };

    /*逻辑或*/
    op_info[0x09] = (OpInfo){ OP_ORA, ADDR_IMM, 2 };
    op_info[0x05] = (OpInfo){ OP_ORA, ADDR_ZP, 3 };
    op_info[0x0D] = (OpInfo){ OP_ORA, ADDR_ABS, 4 };
    op_info[0x15] = (OpInfo){ OP_ORA, ADDR_ZPX, 4 };
    op_info[0x1D] = (OpInfo){ OP_ORA, ADDR_ABX, 4 };
    op_info[0x19] = (OpInfo){ OP_ORA, ADDR_ABY, 4 };
    op_info[0x01] = (OpInfo){ OP_ORA, ADDR_INDX, 6 };
    op_info[0x11] = (OpInfo){ OP_ORA, ADDR_INDY, 5 };

    /*逻辑异或*/
    op_info[0x49] = (OpInfo){ OP_EOR, ADDR_IMM, 2 };
    op_info[0x45] = (OpInfo){ OP_EOR, ADDR_ZP, 3 };
    op_info[0x4D] = (OpInfo){ OP_EOR, ADDR_ABS, 4 };
    op_info[0x55] = (OpInfo){ OP_EOR, ADDR_ZPX, 4 };
    op_info[0x5D] = (OpInfo){ OP_EOR, ADDR_ABX, 4 };
    op_info[0x59] = (OpInfo){ OP_EOR, ADDR_ABY, 4 };
    op_info[0x41] = (OpInfo){ OP_EOR, ADDR_INDX, 6 };
    op_info[0x51] = (OpInfo){ OP_EOR, ADDR_INDY, 5 };

    /*位测试*/
    op_info[0x24] = (OpInfo){ OP_BIT, ADDR_ZP, 3 };
    op_info[0x2C] = (OpInfo){ OP_BIT, ADDR_ABS, 4 };

//移位
    /*左移*/
    op_info[0x0A] = (OpInfo){ OP_ASL, ADDR_ACC, 2 };
    op_info[0x06] = (OpInfo){ OP_ASL, ADDR_ZP, 5 };
    op_info[0x0E] = (OpInfo){ OP_ASL, ADDR_ABS, 6 };
    op_info[0x16] = (OpInfo){ OP_ASL, ADDR_ZPX, 6 };
    op_info[0x1E] = (OpInfo){ OP_ASL, ADDR_ABX, 7 };

    /*右移*/
    op_info[0x4A] = (OpInfo){ OP_LSR, ADDR_ACC, 2 };
    op_info[0x46] = (OpInfo){ OP_LSR, ADDR_ZP, 5 };
    op_info[0x4E] = (OpInfo){ OP_LSR, ADDR_ABS, 6 };
    op_info[0x56] = (OpInfo){ OP_LSR, ADDR_ZPX, 6 };
    op_info[0x5E] = (OpInfo){ OP_LSR, ADDR_ABX, 7 };

    /*循环左移*/
    op_info[0x2A] = (OpInfo){ OP_ROL, ADDR_ACC, 2 };
    op_info[0x26] = (OpInfo){ OP_ROL, ADDR_ZP, 5 };
    op_info[0x2E] = (OpInfo){ OP_ROL, ADDR_ABS, 6 };
    op_info[0x36] = (OpInfo){ OP_ROL, ADDR_ZPX, 6 };
    op_info[0x3E] = (OpInfo){ OP_ROL, ADDR_ABX, 7 };

    /*循环右移*/
    op_info[0x6A] = (OpInfo){ OP_ROR, ADDR_ACC, 2 };
    op_info[0x66] = (OpInfo){ OP_ROR, ADDR_ZP, 5 };
    op_info[0x6E] = (OpInfo){ OP_ROR, ADDR_ABS, 6 };
    op_info[0x76] = (OpInfo){ OP_ROR, ADDR_ZPX, 6 };
    op_info[0x7E] = (OpInfo){ OP_ROR, ADDR_ABX, 7 };

//条件分支
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
            cpu->A = num;
            break;

        default:
            break;
    }
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