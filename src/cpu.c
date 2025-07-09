#include <stdio.h>
#include <string.h>

#include "../include/cpu.h"
#include "../include/memory.h"

OpInfo op_info[256];
u8 fetched = 0x00;


void cpu_init(CPU *cpu) {
    cpu->P = 0;
    cpu->A = 0;
    cpu->X = 0;
    cpu->Y = 0;
    cpu->S = 0;
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

OpInfo get_op(CPU *cpu) {
    u8 opcode = fetch(cpu);
    OpInfo of  = op_info[opcode];
    return of;
}

u16 get_operand_address(CPU *cpu, AddrMode mode) {
    switch (mode) {
        case ADDR_IMM:
            return cpu->PC++; // 立即数，操作数在下一字节
        case ADDR_ZP:
            return memory_read(cpu->PC++) & 0x00FF;//前两位是页号，后两位是页内偏移。
        case ADDR_ABS: {
            u8 lo = memory_read(cpu->PC++);
            u8 hi = memory_read(cpu->PC++);
            return (hi << 8) | lo;
        }

        default:
            return 0;
    }
}



/*读取rom到内存中*/
void load_rom(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if(fp == NULL) {
        printf("failed to open rom:%s\n",filename);
        return;
    }

    //rom头
    u8 rom_head[16];
    fread(rom_head, 1, 16, fp);
    int prg_size = rom_head[4] * 16 * 1024; // PRG-ROM大小
    fseek(fp, 16, SEEK_SET);
    fread(&memory[0x8000], 1, prg_size, fp);
    if(prg_size == 16 * 1024) {
        memcpy(&memory[0xC000], &memory[0x8000], 16 * 1024);
    }
    fclose(fp);

}
