#ifndef CPU_H
#define CPU_H

#include "type.h"
#include <stdbool.h>

/* cpu内存分布
$0000-$07FF  2KB RAM
$0800-$1FFF  RAM镜像
$2000-$2007  PPU寄存器
$2008-$3FFF  PPU寄存器镜像
$4000-$4017  APU/I/O寄存器
$4018-$401F  测试用途
$4020-$5FFF  扩展
$6000-$7FFF  SRAM
$8000-$FFFF  PRG-ROM
*/

// 中断向量地址
#define NMI_VECTOR    0xFFFA  // NMI中断向量地址
#define RESET_VECTOR  0xFFFC  // RESET中断向量地址
#define IRQ_VECTOR    0xFFFE  // IRQ中断向量地址

// CPU状态寄存器标志位
#define FLAG_C  (1 << 0)  // 进位标志
#define FLAG_Z  (1 << 1)  // 零标志
#define FLAG_I  (1 << 2)  // 中断禁止标志
#define FLAG_D  (1 << 3)  // 十进制模式标志
#define FLAG_B  (1 << 4)  // Break指令标志
#define FLAG_U  (1 << 5)  // 未使用（总是1）
#define FLAG_V  (1 << 6)  // 溢出标志
#define FLAG_N  (1 << 7)  // 负数标志

typedef struct {
    u8 P;    // 程序状态字
    u8 A;    // 累加器
    u8 X, Y; // 索引寄存器
    u8 S;    // 堆栈指针
    u16 PC;  // 程序计数器
    u32 cycle; // 时钟周期
    
    // 中断相关标志
    bool nmi_pending;   // NMI不可屏蔽中断等待处理
    bool irq_pending;   // IRQ中断等待处理
    
    // 临时/译码工作区
    u16 operand_addr; // 寻址计算出的操作数地址
    u8 fetched;       // 当前指令读取到的操作数（立即数或从内存读出）
    u8 page_crossed;  // 寻址过程中是否跨页（用于周期补偿）
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
    const char *name;    
    // 新增：寻址函数与操作函数指针（用于函数指针驱动调度）
    u8 (*addr_func)(CPU *cpu); // 计算操作数地址/填充 cpu->fetched；返回是否跨页（0/1）
    void (*op_func)(CPU *cpu, AddrMode mode); // 执行操作
} OpInfo;   //代表一条6502CPU指令

extern OpInfo op_info[256];

typedef void (*OpFunc)(CPU *cpu, AddrMode mode);

void cpu_init(CPU *cpu);
void set_flag(CPU *cpu, u8 flag, bool value);
bool get_flag(CPU *cpu, u8 flag);
void init_op_table();
OpInfo get_op_type(u8 instruction);
void clock_tick();   //模拟时钟周期
void reset(CPU *cpu);
void irq(CPU *cpu);
void nmi(CPU *cpu);
void check_interrupts(CPU *cpu);

void push_stack(CPU *cpu, u8 value);
void push_stack16(CPU *cpu, u16 value);
u8 pull_stack(CPU *cpu);
u16 pull_stack16(CPU *cpu);


u8 fetch(CPU *cpu);     //取指  
OpInfo get_op(CPU *cpu);
void run_instruction(CPU *cpu, OpType op, u16 addr, u8 num);//执行
void cpu_run(CPU *cpu);

extern u8 cpuRam[0x0800]; // 2KB cpuRAM

#endif
