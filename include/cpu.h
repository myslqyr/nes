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
    u32 cycle; // 时钟周期
} CPU;

typedef enum {
    ADDR_ACC,      // A (累加器)
    ADDR_ABS,      // absolute
    ADDR_ABX,      // absolute,X
    ADDR_ABY,      // absolute,Y
    ADDR_IMM,      // #immediate
    ADDR_IMPL,     // implied
    ADDR_IND,      // (absolute) 间接
    ADDR_INDX,     // (zero page,X) X-indexed, indirect
    ADDR_INDY,     // (zero page),Y indirect, Y-indexed
    ADDR_REL,      // relative
    ADDR_ZP,       // zero page
    ADDR_ZPX,      // zero page,X
    ADDR_ZPY,      // zero page,Y
} AddrMode;

typedef enum {
    OP_ADC,
    OP_AND,
    OP_ASL,
    OP_BCC,
    OP_BCS,
    OP_BEQ,
    OP_BIT,
    OP_BMI,
    OP_BNE,
    OP_BPL,
    OP_BRK,
    OP_BVC,
    OP_BVS,
    OP_CLC,
    OP_CLD,
    OP_CLI,
    OP_CLV,
    OP_CMP,
    OP_CPX,
    OP_CPY,
    OP_DEC,
    OP_DEX,
    OP_DEY,
    OP_EOR,
    OP_INC,
    OP_INX,
    OP_INY,
    OP_JMP,
    OP_JSR,
    OP_LDA,
    OP_LDX,
    OP_LDY,
    OP_LSR,
    OP_NOP,
    OP_ORA,
    OP_PHA,
    OP_PHP,
    OP_PLA,
    OP_PLP,
    OP_ROL,
    OP_ROR,
    OP_RTI,
    OP_RTS,
    OP_SBC,
    OP_SEC,
    OP_SED,
    OP_SEI,
    OP_STA,
    OP_STX,
    OP_STY,
    OP_TAX,
    OP_TAY,
    OP_TSX,
    OP_TXA,
    OP_TXS,
    OP_TYA,
    OP_UNK // 未知/非法指令
} OpType;       //https://www.masswerk.at/6502/6502_instruction_set.html#ORA
                //https://github.com/tcarmelveilleux/dcc6502#  6502反汇编工具  在线网站：https://www.masswerk.at/6502/disassembler.html

typedef struct {
    OpType op;
    AddrMode addr_mode;
    u8 cycles;
} OpInfo;   //代表一条6502CPU指令

extern OpInfo op_info[256];

typedef void (*OpFunc)(CPU *cpu, AddrMode mode);

void cpu_init(CPU *cpu);
void set_flag(CPU *cpu, u8 flag);
u8 get_flag(CPU *cpu);
void init_op_table();

void clock();   //模拟时钟周期
void reset();   // 复位信号
void irq();     //中断信号
void nmi();     //不可屏蔽中断

u8 fetch(CPU *cpu);     //取指  
OpInfo get_op(CPU *cpu);
u16 get_operand_address(CPU *cpu, AddrMode mode); //译码
u8 fetch_op_num(u16 addr);// 取数
void run_instruction(CPU *cpu, OpType op, u16 addr, u8 num);//执行
void cpu_run(CPU *cpu);

//寻址方式

#endif
