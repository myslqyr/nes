#include "../include/cpu.h"
#include "../include/irq.h"
#include "../include/bus.h"
#include "../include/disassembly.h"

OpInfo op_info[256];


// 堆栈操作辅助函数,堆栈固定在第一个页中
void push_stack(CPU *cpu, u8 value) {
    bus_write(0x100 + cpu->S, value);
    cpu->S--;
}

void push_stack16(CPU *cpu, u16 value) {
    push_stack(cpu, (value >> 8) & 0xFF);  // 高字节
    push_stack(cpu, value & 0xFF);         // 低字节
}

u8 pull_stack(CPU *cpu) {
    cpu->S++;
    return bus_read(0x100 + cpu->S);
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

/* init_op_table moved to src/opcodes.c */

u8 fetch(CPU *cpu) {
    return bus_read(cpu->PC++);
}


/*获取二进制指令的操作码*/
OpInfo get_op(CPU *cpu) {
    u8 opcode = fetch(cpu);
    OpInfo op  = op_info[opcode];
    return op;
}


u8 fetch_op_num(u16 addr) {
    if(addr == (u16)0) {
        return (u8)0; // 旧逻辑用全局 fetched；现在应使用 cpu->fetched，函数签名无法取到 cpu 指针
    } else {
        return bus_read(addr);
    }
}

//TODO:集中处理各个标志位
void set_cpu_p(CPU *cpu, u8 num) {
    return ;
}


// 简化的 run_instruction 保持存在以兼容外部调用（现在多数逻辑在 cpu_run 中执行）
void run_instruction(CPU *cpu, OpType op, u16 addr, u8 num) {
    // 兼容性占位：调用未实现的泛用处理
    (void)cpu; (void)op; (void)addr; (void)num;
}

OpInfo get_op_type(u8 instruction) {
    OpInfo op = op_info[(instruction & 0xFF)];
    return op;
}




/*模拟CPU的执行过程*/
void cpu_run(CPU *cpu) {
    // 检查中断
    check_interrupts(cpu);
    
    // 基于函数指针的取指-译码-执行流程
    u16 current_pc = cpu->PC;
    u8 opcode = fetch(cpu);
    OpInfo entry = op_info[opcode];

    // 调试输出：显示指令信息和当前CPU状态
    debug_print_instruction(cpu, opcode, entry, current_pc);

    // 调用寻址函数，寻址函数应填充 cpu->operand_addr 和 cpu->fetched，返回是否跨页
    u8 page = 0;
    if (entry.addr_func) {
        page = entry.addr_func(cpu);
    }

    // 分支指令的周期补偿需要在操作执行时处理（仅当分支被采纳时）
    bool is_branch = (entry.op == OP_BCC || entry.op == OP_BCS || entry.op == OP_BEQ || entry.op == OP_BNE || entry.op == OP_BPL || entry.op == OP_BMI || entry.op == OP_BVC || entry.op == OP_BVS);

    if (is_branch) {
        cpu->cycle += entry.cycles; // 基本周期
        if (entry.op_func) entry.op_func(cpu, entry.addr_mode);
        // 分支操作函数负责在分支被采纳时增加额外的周期（包括跨页）
    } else {
        // 对非分支指令，根据寻址跨页加1周期（若寻址返回 1）
        cpu->cycle += entry.cycles + page;
        if (entry.op_func) entry.op_func(cpu, entry.addr_mode);
    }
    cpu->page_crossed = 0;
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