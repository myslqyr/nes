/*
 * opcodes.c
 * 把指令实现与寻址从 cpu 核心拆出，集中管理 opcode 表与函数指针映射
 */

#include "../include/cpu.h"
#include "../include/bus.h"

extern OpInfo op_info[256];

/* 设置 Z/N 标志 */
void set_zn_flags(CPU *cpu, u8 v) {
    if (v == 0) cpu->P |= FLAG_Z; else cpu->P &= ~FLAG_Z;
    if (v & 0x80) cpu->P |= FLAG_N; else cpu->P &= ~FLAG_N;
}

/* 寻址函数 */
u8 addr_impl(CPU *cpu) {
    cpu->operand_addr = 0;
    cpu->fetched = 0;
    return 0;
}

u8 addr_acc(CPU *cpu) {
    cpu->fetched = cpu->A;
    cpu->operand_addr = 0;  // 累加器模式，不使用地址
    return 0;
}

u8 addr_imm(CPU *cpu) {
    cpu->operand_addr = cpu->PC;
    cpu->fetched = cpu_read(cpu->PC++);
    return 0;
}

u8 addr_abs(CPU *cpu) {
    u8 lo = cpu_read(cpu->PC++);
    u8 hi = cpu_read(cpu->PC++);
    u16 addr = (hi << 8) | lo;
    cpu->operand_addr = addr;
    cpu->fetched = cpu_read(addr);
    cpu->page_crossed = 0;
    return 0;
}

u8 addr_abx(CPU *cpu) {
    u8 lo = cpu_read(cpu->PC++);
    u8 hi = cpu_read(cpu->PC++);
    u16 abs = (hi << 8) | lo;
    u16 final = abs + cpu->X;
    cpu->operand_addr = final;
    cpu->fetched = cpu_read(final);
    cpu->page_crossed = ((final & 0xFF00) != (abs & 0xFF00)) ? 1 : 0;
    return cpu->page_crossed;
}

u8 addr_aby(CPU *cpu) {
    u8 lo = cpu_read(cpu->PC++);
    u8 hi = cpu_read(cpu->PC++);
    u16 abs = (hi << 8) | lo;
    u16 final = abs + cpu->Y;
    cpu->operand_addr = final;
    cpu->fetched = cpu_read(final);
    cpu->page_crossed = ((final & 0xFF00) != (abs & 0xFF00)) ? 1 : 0;
    return cpu->page_crossed;
}

u8 addr_zp(CPU *cpu) {
    u8 a = cpu_read(cpu->PC++);
    cpu->operand_addr = a & 0xFF;
    cpu->fetched = cpu_read(cpu->operand_addr);
    cpu->page_crossed = 0;
    return 0;
}

u8 addr_zpx(CPU *cpu) {
    u8 base = cpu_read(cpu->PC++);
    u8 addr = (base + cpu->X) & 0xFF;
    cpu->operand_addr = addr;
    cpu->fetched = cpu_read(addr);
    cpu->page_crossed = 0;
    return 0;
}

u8 addr_zpy(CPU *cpu) {
    u8 base = cpu_read(cpu->PC++);
    u8 addr = (base + cpu->Y) & 0xFF;
    cpu->operand_addr = addr;
    cpu->fetched = cpu_read(addr);
    cpu->page_crossed = 0;
    return 0;
}

u8 addr_ind(CPU *cpu) {
    u8 lo = cpu_read(cpu->PC++);
    u8 hi = cpu_read(cpu->PC++);
    u16 ptr = (hi << 8) | lo;
    u8 nlo = cpu_read(ptr);
    u8 nhi = ((ptr & 0x00FF) == 0x00FF) ? cpu_read(ptr & 0xFF00) : cpu_read(ptr + 1);
    u16 addr = (nhi << 8) | nlo;
    cpu->operand_addr = addr;
    cpu->fetched = cpu_read(addr);
    cpu->page_crossed = 0;
    return 0;
}

u8 addr_indx(CPU *cpu) {
    u8 zp = cpu_read(cpu->PC++);
    u8 ptr = (zp + cpu->X) & 0xFF;
    u8 lo = cpu_read(ptr);
    u8 hi = cpu_read((ptr + 1) & 0xFF);
    u16 addr = (hi << 8) | lo;
    cpu->operand_addr = addr;
    cpu->fetched = cpu_read(addr);
    cpu->page_crossed = 0;
    return 0;
}

u8 addr_indy(CPU *cpu) {
    u8 zp = cpu_read(cpu->PC++);
    u8 lo = cpu_read(zp);
    u8 hi = cpu_read((zp + 1) & 0xFF);
    u16 base = (hi << 8) | lo;
    u16 final = base + cpu->Y;
    cpu->operand_addr = final;
    cpu->fetched = cpu_read(final);
    cpu->page_crossed = ((final & 0xFF00) != (base & 0xFF00)) ? 1 : 0;
    return cpu->page_crossed;
}

u8 addr_rel(CPU *cpu) { //相对寻址
    u8 offset = cpu_read(cpu->PC++);
    u16 base_pc = cpu->PC;
    if (offset & 0x80) {
        base_pc = cpu->PC + (offset | 0xFF00);
    } else {
        base_pc = cpu->PC + offset;
    }
    cpu->operand_addr = base_pc;
    cpu->page_crossed = ((base_pc & 0xFF00) != (cpu->PC & 0xFF00)) ? 1 : 0;
    cpu->fetched = 0;
    return cpu->page_crossed;
}

/* 操作函数（简化实现） */
void op_unimpl(CPU *cpu, AddrMode mode) {
    (void)cpu;
    (void)mode;
}

void op_nop(CPU *cpu, AddrMode mode) {
    (void)cpu;
    (void)mode;
}

void op_lda(CPU *cpu, AddrMode mode) {
    (void)mode;  // 不使用mode参数，因为LDA总是使用fetched值
    cpu->A = cpu->fetched;
    set_zn_flags(cpu, cpu->A);
}

void op_sta(CPU *cpu, AddrMode mode) {
    (void)mode;  // STA总是写到内存
    cpu_write(cpu->operand_addr, cpu->A);
}

void op_adc(CPU *cpu, AddrMode mode) {
    (void)mode;  // ADC总是使用fetched值
    u8 m = cpu->fetched;
    u8 carry = (cpu->P & FLAG_C) ? 1 : 0;
    u16 res = (u16)cpu->A + (u16)m + carry;
    u8 old = cpu->A;
    cpu->A = res & 0xFF;
    if (res > 0xFF) {
        cpu->P |= FLAG_C;
    } else {
        cpu->P &= ~FLAG_C;
    }
    set_zn_flags(cpu, cpu->A);
    if (((old ^ m) & 0x80) == 0 && ((old ^ cpu->A) & 0x80) != 0) {
        cpu->P |= FLAG_V;
    } else {
        cpu->P &= ~FLAG_V;
    }
}

void op_sbc(CPU *cpu, AddrMode mode) {
    (void)mode;  // SBC总是使用fetched值
    u8 m = cpu->fetched;
    u8 carry = (cpu->P & FLAG_C) ? 1 : 0;
    u16 value = m ^ 0xFF;
    u16 res = (u16)cpu->A + value + carry;
    u8 old = cpu->A;
    cpu->A = res & 0xFF;
    if (res > 0xFF) {
        cpu->P |= FLAG_C;
    } else {
        cpu->P &= ~FLAG_C;
    }
    set_zn_flags(cpu, cpu->A);
    if (((old ^ value) & 0x80) == 0 && ((old ^ cpu->A) & 0x80) != 0) {
        cpu->P |= FLAG_V;
    } else {
        cpu->P &= ~FLAG_V;
    }
}

void op_jmp(CPU *cpu, AddrMode mode) {
    (void)mode;  // JMP总是跳转到operand_addr
    cpu->PC = cpu->operand_addr;
}

void op_brk(CPU *cpu, AddrMode mode) {
    (void)mode;  // BRK是隐含寻址
    u16 ret = cpu->PC + 1;
    push_stack(cpu, (ret >> 8) & 0xFF);
    push_stack(cpu, ret & 0xFF);
    push_stack(cpu, cpu->P | FLAG_B);
    cpu->P |= FLAG_I;
    u8 lo = cpu_read(IRQ_VECTOR);
    u8 hi = cpu_read(IRQ_VECTOR + 1);
    cpu->PC = (hi << 8) | lo;
}

void op_inc(CPU *cpu, AddrMode mode) {
    (void)mode;  // INC总是操作内存
    u8 v = cpu_read(cpu->operand_addr);
    v++;
    cpu_write(cpu->operand_addr, v);
    set_zn_flags(cpu, v);
}

void op_inx(CPU *cpu, AddrMode mode) {
    (void)mode;  // INX是隐含寻址
    cpu->X++;
    set_zn_flags(cpu, cpu->X);
}

void op_iny(CPU *cpu, AddrMode mode) {
    (void)mode;  // INY是隐含寻址
    cpu->Y++;
    set_zn_flags(cpu, cpu->Y);
}

void op_dec(CPU *cpu, AddrMode mode) {
    (void)mode;  // DEC总是操作内存
    u8 v = cpu_read(cpu->operand_addr);
    v--;
    cpu_write(cpu->operand_addr, v);
    set_zn_flags(cpu, v);
}

void op_dex(CPU *cpu, AddrMode mode) {
    (void)mode;  // DEX是隐含寻址
    cpu->X--;
    set_zn_flags(cpu, cpu->X);
}

void op_dey(CPU *cpu, AddrMode mode) {
    (void)mode;  // DEY是隐含寻址
    cpu->Y--;
    set_zn_flags(cpu, cpu->Y);
}

void op_and(CPU *cpu, AddrMode mode) {
    (void)mode;  // AND总是使用fetched值
    cpu->A &= cpu->fetched;
    set_zn_flags(cpu, cpu->A);
}

void op_ora(CPU *cpu, AddrMode mode) {
    (void)mode;  // ORA总是使用fetched值
    cpu->A |= cpu->fetched;
    set_zn_flags(cpu, cpu->A);
}

void op_eor(CPU *cpu, AddrMode mode) {
    (void)mode;  // EOR总是使用fetched值
    cpu->A ^= cpu->fetched;
    set_zn_flags(cpu, cpu->A);
}

void op_bit(CPU *cpu, AddrMode mode) {
    (void)mode;  // BIT总是使用fetched值
    u8 r = cpu->A & cpu->fetched;
    if (r == 0) {
        cpu->P |= FLAG_Z;
    } else {
        cpu->P &= ~FLAG_Z;
    }
    cpu->P = (cpu->P & ~FLAG_N) | (cpu->fetched & FLAG_N);
    cpu->P = (cpu->P & ~FLAG_V) | (cpu->fetched & FLAG_V);
}

// BIT指令的绝对寻址版本
void op_bit_abs(CPU *cpu, AddrMode mode) {
    (void)mode;  // BIT总是使用fetched值
    op_bit(cpu, mode);  // 逻辑相同，只是寻址方式不同
}

void op_asl(CPU *cpu, AddrMode mode) {
    u8 val = (mode == ADDR_ACC) ? cpu->A : cpu->fetched;
    u8 res = val << 1;
    if (val & 0x80) {
        cpu->P |= FLAG_C;
    } else {
        cpu->P &= ~FLAG_C;
    }
    set_zn_flags(cpu, res);
    if (mode == ADDR_ACC) {
        cpu->A = res;
    } else {
        cpu_write(cpu->operand_addr, res);
    }
}

void op_lsr(CPU *cpu, AddrMode mode) {
    u8 val = (mode == ADDR_ACC) ? cpu->A : cpu->fetched;
    u8 res = val >> 1;
    if (val & 0x01) {
        cpu->P |= FLAG_C;
    } else {
        cpu->P &= ~FLAG_C;
    }
    cpu->P &= ~FLAG_N;
    set_zn_flags(cpu, res);
    if (mode == ADDR_ACC) {
        cpu->A = res;
    } else {
        cpu_write(cpu->operand_addr, res);
    }
}

void op_rol(CPU *cpu, AddrMode mode) {
    u8 val = (mode == ADDR_ACC) ? cpu->A : cpu->fetched;
    u8 oldc = (cpu->P & FLAG_C) ? 1 : 0;
    u8 res = (val << 1) | oldc;
    if (val & 0x80) {
        cpu->P |= FLAG_C;
    } else {
        cpu->P &= ~FLAG_C;
    }
    set_zn_flags(cpu, res);
    if (mode == ADDR_ACC) {
        cpu->A = res;
    } else {
        cpu_write(cpu->operand_addr, res);
    }
}

void op_ror(CPU *cpu, AddrMode mode) {
    u8 val = (mode == ADDR_ACC) ? cpu->A : cpu->fetched;
    u8 oldc = (cpu->P & FLAG_C) ? 0x80 : 0;
    u8 res = (val >> 1) | oldc;
    if (val & 0x01) {
        cpu->P |= FLAG_C;
    } else {
        cpu->P &= ~FLAG_C;
    }
    set_zn_flags(cpu, res);
    if (mode == ADDR_ACC) {
        cpu->A = res;
    } else {
        cpu_write(cpu->operand_addr, res);
    }
}

/* 分支函数 */
void op_bcc(CPU *cpu, AddrMode mode) {
    (void)mode;  // 分支指令都是相对寻址
    if ((cpu->P & FLAG_C) == 0) {
        cpu->PC = cpu->operand_addr;
        cpu->cycle += 1;
        if (cpu->page_crossed) {
            cpu->cycle += 1;
        }
    }
}

void op_bcs(CPU *cpu, AddrMode mode) {
    (void)mode;  // 分支指令都是相对寻址
    if ((cpu->P & FLAG_C) != 0) {
        cpu->PC = cpu->operand_addr;
        cpu->cycle += 1;
        if (cpu->page_crossed) {
            cpu->cycle += 1;
        }
    }
}

void op_beq(CPU *cpu, AddrMode mode) {
    (void)mode;  // 分支指令都是相对寻址
    if ((cpu->P & FLAG_Z) != 0) {
        cpu->PC = cpu->operand_addr;
        cpu->cycle += 1;
        if (cpu->page_crossed) {
            cpu->cycle += 1;
        }
    }
}

void op_bne(CPU *cpu, AddrMode mode) {
    (void)mode;  // 分支指令都是相对寻址
    if ((cpu->P & FLAG_Z) == 0) {
        cpu->PC = cpu->operand_addr;
        cpu->cycle += 1;
        if (cpu->page_crossed) {
            cpu->cycle += 1;
        }
    }
}

void op_bpl(CPU *cpu, AddrMode mode) {
    (void)mode;  // 分支指令都是相对寻址
    if ((cpu->P & FLAG_N) == 0) {
        cpu->PC = cpu->operand_addr;
        cpu->cycle += 1;
        if (cpu->page_crossed) {
            cpu->cycle += 1;
        }
    }
}

void op_bmi(CPU *cpu, AddrMode mode) {
    (void)mode;  // 分支指令都是相对寻址
    if ((cpu->P & FLAG_N) != 0) {
        cpu->PC = cpu->operand_addr;
        cpu->cycle += 1;
        if (cpu->page_crossed) {
            cpu->cycle += 1;
        }
    }
}

void op_bvc(CPU *cpu, AddrMode mode) {
    (void)mode;  // 分支指令都是相对寻址
    if ((cpu->P & FLAG_V) == 0) {
        cpu->PC = cpu->operand_addr;
        cpu->cycle += 1;
        if (cpu->page_crossed) {
            cpu->cycle += 1;
        }
    }
}

void op_bvs(CPU *cpu, AddrMode mode) {
    (void)mode;  // 分支指令都是相对寻址
    if ((cpu->P & FLAG_V) != 0) {
        cpu->PC = cpu->operand_addr;
        cpu->cycle += 1;
        if (cpu->page_crossed) {
            cpu->cycle += 1;
        }
    }
}

/* 堆栈/子程序/中断相关 */
void op_pha(CPU *cpu, AddrMode mode) {
    (void)mode;  // PHA是隐含寻址
    push_stack(cpu, cpu->A);
}

void op_pla(CPU *cpu, AddrMode mode) {
    (void)mode;  // PLA是隐含寻址
    cpu->A = pull_stack(cpu);
    set_zn_flags(cpu, cpu->A);
}

void op_php(CPU *cpu, AddrMode mode) {
    (void)mode;  // PHP是隐含寻址
    // PHP总是设置B和U标志
    push_stack(cpu, cpu->P | FLAG_B | FLAG_U);
}

void op_plp(CPU *cpu, AddrMode mode) {
    (void)mode;  // PLP是隐含寻址
    cpu->P = pull_stack(cpu);
    // PLP清除B标志（因为这不是从BRK返回）
    cpu->P &= ~FLAG_B;
    // U标志总是设置
    cpu->P |= FLAG_U;
}

void op_jsr(CPU *cpu, AddrMode mode) {
    (void)mode;  // JSR是绝对寻址
    u16 ret = cpu->PC - 1;
    push_stack(cpu, (ret >> 8) & 0xFF);
    push_stack(cpu, ret & 0xFF);
    cpu->PC = cpu->operand_addr;
}

void op_rts(CPU *cpu, AddrMode mode) {
    (void)mode;  // RTS是隐含寻址
    u8 lo = pull_stack(cpu);
    u8 hi = pull_stack(cpu);
    u16 addr = (hi << 8) | lo;
    cpu->PC = addr + 1;
}

void op_rti(CPU *cpu, AddrMode mode) {
    (void)mode;  // RTI是隐含寻址
    cpu->P = pull_stack(cpu);
    // RTI时清除B标志，因为这不是BRK中断返回
    cpu->P &= ~FLAG_B;
    // U标志在6502中未使用，RTI时保持不变或清除
    cpu->P &= ~FLAG_U;

    // 2. 弹出PC低字节
    u8 lo = pull_stack(cpu);
    // 3. 弹出PC高字节
    u8 hi = pull_stack(cpu);
    // 组合成完整的PC
    cpu->PC = (hi << 8) | lo;
}

/* 更多的操作函数 */
void op_ldx(CPU *cpu, AddrMode mode) {
    (void)mode;  // LDX总是使用fetched值
    cpu->X = cpu->fetched;
    set_zn_flags(cpu, cpu->X);
}

void op_ldy(CPU *cpu, AddrMode mode) {
    (void)mode;  // LDY总是使用fetched值
    cpu->Y = cpu->fetched;
    set_zn_flags(cpu, cpu->Y);
}

void op_stx(CPU *cpu, AddrMode mode) {
    (void)mode;  // STX总是写到内存
    cpu_write(cpu->operand_addr, cpu->X);
}

void op_sty(CPU *cpu, AddrMode mode) {
    (void)mode;  // STY总是写到内存
    cpu_write(cpu->operand_addr, cpu->Y);
}

void op_cmp(CPU *cpu, AddrMode mode) {
    (void)mode;  // CMP总是使用fetched值
    u16 res = (u16)cpu->A - (u16)cpu->fetched;
    if (cpu->A >= cpu->fetched) {
        cpu->P |= FLAG_C;
    } else {
        cpu->P &= ~FLAG_C;
    }
    set_zn_flags(cpu, res & 0xFF);
}

void op_cpx(CPU *cpu, AddrMode mode) {
    (void)mode;  // CPX总是使用fetched值
    u16 res = (u16)cpu->X - (u16)cpu->fetched;
    if (cpu->X >= cpu->fetched) {
        cpu->P |= FLAG_C;
    } else {
        cpu->P &= ~FLAG_C;
    }
    set_zn_flags(cpu, res & 0xFF);
}

void op_cpy(CPU *cpu, AddrMode mode) {
    (void)mode;  // CPY总是使用fetched值
    u16 res = (u16)cpu->Y - (u16)cpu->fetched;
    if (cpu->Y >= cpu->fetched) {
        cpu->P |= FLAG_C;
    } else {
        cpu->P &= ~FLAG_C;
    }
    set_zn_flags(cpu, res & 0xFF);
}

void op_tax(CPU *cpu, AddrMode mode) {
    (void)mode;  // TAX是隐含寻址
    cpu->X = cpu->A;
    set_zn_flags(cpu, cpu->X);
}

void op_txa(CPU *cpu, AddrMode mode) {
    (void)mode;  // TXA是隐含寻址
    cpu->A = cpu->X;
    set_zn_flags(cpu, cpu->A);
}

void op_tay(CPU *cpu, AddrMode mode) {
    (void)mode;  // TAY是隐含寻址
    cpu->Y = cpu->A;
    set_zn_flags(cpu, cpu->Y);
}

void op_tya(CPU *cpu, AddrMode mode) {
    (void)mode;  // TYA是隐含寻址
    cpu->A = cpu->Y;
    set_zn_flags(cpu, cpu->A);
}

void op_tsx(CPU *cpu, AddrMode mode) {
    (void)mode;  // TSX是隐含寻址
    cpu->X = cpu->S;
    set_zn_flags(cpu, cpu->X);
}

void op_txs(CPU *cpu, AddrMode mode) {
    (void)mode;  // TXS是隐含寻址
    cpu->S = cpu->X;
}

void op_clc(CPU *cpu, AddrMode mode) {
    (void)mode;  // CLC是隐含寻址
    cpu->P &= ~FLAG_C;
}

void op_sec(CPU *cpu, AddrMode mode) {
    (void)mode;  // SEC是隐含寻址
    cpu->P |= FLAG_C;
}

void op_cli(CPU *cpu, AddrMode mode) {
    (void)mode;  // CLI是隐含寻址
    cpu->P &= ~FLAG_I;
}

void op_sei(CPU *cpu, AddrMode mode) {
    (void)mode;  // SEI是隐含寻址
    cpu->P |= FLAG_I;
}

void op_cld(CPU *cpu, AddrMode mode) {
    (void)mode;  // CLD是隐含寻址
    cpu->P &= ~FLAG_D;
}

void op_sed(CPU *cpu, AddrMode mode) {
    (void)mode;  // SED是隐含寻址
    cpu->P |= FLAG_D;
}

void op_clv(CPU *cpu, AddrMode mode) {
    (void)mode;  // CLV是隐含寻址
    cpu->P &= ~FLAG_V;
}

/* 初始化 opcode 表并绑定函数指针 */
void init_op_table() {
    for (int i = 0; i < 256; i++) {
        op_info[i] = (OpInfo){ OP_NOP, ADDR_IMPL, 2, "NOP", addr_impl, op_nop };
    }

    // 这里按需替换具体 opcode 的 op 和 addr_mode（复制原有表）
    op_info[0x69] = (OpInfo){ OP_ADC, ADDR_IMM, 2, "ADC", addr_imm, op_adc };
    op_info[0x65] = (OpInfo){ OP_ADC, ADDR_ZP, 3, "ADC", addr_zp, op_adc };
    op_info[0x6D] = (OpInfo){ OP_ADC, ADDR_ABS, 4, "ADC", addr_abs, op_adc };
    op_info[0x75] = (OpInfo){ OP_ADC, ADDR_ZPX, 4, "ADC", addr_zpx, op_adc };
    op_info[0x7D] = (OpInfo){ OP_ADC, ADDR_ABX, 4, "ADC", addr_abx, op_adc };
    op_info[0x79] = (OpInfo){ OP_ADC, ADDR_ABY, 4, "ADC", addr_aby, op_adc };
    op_info[0x61] = (OpInfo){ OP_ADC, ADDR_INDX, 6, "ADC", addr_indx, op_adc };
    op_info[0x71] = (OpInfo){ OP_ADC, ADDR_INDY, 5, "ADC", addr_indy, op_adc };

    // SBC指令 (减法)
    op_info[0xE9] = (OpInfo){ OP_SBC, ADDR_IMM, 2, "SBC", addr_imm, op_sbc };
    op_info[0xE5] = (OpInfo){ OP_SBC, ADDR_ZP, 3, "SBC", addr_zp, op_sbc };
    op_info[0xED] = (OpInfo){ OP_SBC, ADDR_ABS, 4, "SBC", addr_abs, op_sbc };
    op_info[0xF5] = (OpInfo){ OP_SBC, ADDR_ZPX, 4, "SBC", addr_zpx, op_sbc };
    op_info[0xFD] = (OpInfo){ OP_SBC, ADDR_ABX, 4, "SBC", addr_abx, op_sbc };
    op_info[0xF9] = (OpInfo){ OP_SBC, ADDR_ABY, 4, "SBC", addr_aby, op_sbc };
    op_info[0xE1] = (OpInfo){ OP_SBC, ADDR_INDX, 6, "SBC", addr_indx, op_sbc };
    op_info[0xF1] = (OpInfo){ OP_SBC, ADDR_INDY, 5, "SBC", addr_indy, op_sbc };

    // 分支
    op_info[0x90] = (OpInfo){ OP_BCC, ADDR_REL, 2, "BCC", addr_rel, op_bcc };
    op_info[0xB0] = (OpInfo){ OP_BCS, ADDR_REL, 2, "BCS", addr_rel, op_bcs };
    op_info[0xF0] = (OpInfo){ OP_BEQ, ADDR_REL, 2, "BEQ", addr_rel, op_beq };
    op_info[0xD0] = (OpInfo){ OP_BNE, ADDR_REL, 2, "BNE", addr_rel, op_bne };
    op_info[0x10] = (OpInfo){ OP_BPL, ADDR_REL, 2, "BPL", addr_rel, op_bpl };
    op_info[0x30] = (OpInfo){ OP_BMI, ADDR_REL, 2, "BMI", addr_rel, op_bmi };
    op_info[0x50] = (OpInfo){ OP_BVC, ADDR_REL, 2, "BVC", addr_rel, op_bvc };
    op_info[0x70] = (OpInfo){ OP_BVS, ADDR_REL, 2, "BVS", addr_rel, op_bvs };

    // JMP/BRK/JSR/RTS/RTI 示例
    op_info[0x4C] = (OpInfo){ OP_JMP, ADDR_ABS, 3, "JMP", addr_abs, op_jmp };
    op_info[0x6C] = (OpInfo){ OP_JMP, ADDR_IND, 5, "JMP", addr_ind, op_jmp };
    op_info[0x00] = (OpInfo){ OP_BRK, ADDR_IMPL, 7, "BRK", addr_impl, op_brk };
    op_info[0x20] = (OpInfo){ OP_JSR, ADDR_ABS, 6, "JSR", addr_abs, op_jsr };
    op_info[0x60] = (OpInfo){ OP_RTS, ADDR_IMPL, 6, "RTS", addr_impl, op_rts };
    op_info[0x40] = (OpInfo){ OP_RTI, ADDR_IMPL, 6, "RTI", addr_impl, op_rti };

    // LDA/STA - 更多变体
    op_info[0xA9] = (OpInfo){ OP_LDA, ADDR_IMM, 2, "LDA", addr_imm, op_lda };
    op_info[0xA5] = (OpInfo){ OP_LDA, ADDR_ZP, 3, "LDA", addr_zp, op_lda };
    op_info[0xAD] = (OpInfo){ OP_LDA, ADDR_ABS, 4, "LDA", addr_abs, op_lda };
    op_info[0xB5] = (OpInfo){ OP_LDA, ADDR_ZPX, 4, "LDA", addr_zpx, op_lda };
    op_info[0xBD] = (OpInfo){ OP_LDA, ADDR_ABX, 4, "LDA", addr_abx, op_lda };
    op_info[0xB9] = (OpInfo){ OP_LDA, ADDR_ABY, 4, "LDA", addr_aby, op_lda };
    op_info[0xA1] = (OpInfo){ OP_LDA, ADDR_INDX, 6, "LDA", addr_indx, op_lda };
    op_info[0xB1] = (OpInfo){ OP_LDA, ADDR_INDY, 5, "LDA", addr_indy, op_lda };

    op_info[0x85] = (OpInfo){ OP_STA, ADDR_ZP, 3, "STA", addr_zp, op_sta };
    op_info[0x8D] = (OpInfo){ OP_STA, ADDR_ABS, 4, "STA", addr_abs, op_sta };
    op_info[0x95] = (OpInfo){ OP_STA, ADDR_ZPX, 4, "STA", addr_zpx, op_sta };
    op_info[0x9D] = (OpInfo){ OP_STA, ADDR_ABX, 5, "STA", addr_abx, op_sta };
    op_info[0x99] = (OpInfo){ OP_STA, ADDR_ABY, 5, "STA", addr_aby, op_sta };
    op_info[0x81] = (OpInfo){ OP_STA, ADDR_INDX, 6, "STA", addr_indx, op_sta };
    op_info[0x91] = (OpInfo){ OP_STA, ADDR_INDY, 6, "STA", addr_indy, op_sta };

    // LDX/LDY
    op_info[0xA2] = (OpInfo){ OP_LDX, ADDR_IMM, 2, "LDX", addr_imm, op_ldx };
    op_info[0xA6] = (OpInfo){ OP_LDX, ADDR_ZP, 3, "LDX", addr_zp, op_ldx };
    op_info[0xAE] = (OpInfo){ OP_LDX, ADDR_ABS, 4, "LDX", addr_abs, op_ldx };
    op_info[0xB6] = (OpInfo){ OP_LDX, ADDR_ZPY, 4, "LDX", addr_zpy, op_ldx };
    op_info[0xBE] = (OpInfo){ OP_LDX, ADDR_ABY, 4, "LDX", addr_aby, op_ldx };

    op_info[0xA0] = (OpInfo){ OP_LDY, ADDR_IMM, 2, "LDY", addr_imm, op_ldy };
    op_info[0xA4] = (OpInfo){ OP_LDY, ADDR_ZP, 3, "LDY", addr_zp, op_ldy };
    op_info[0xAC] = (OpInfo){ OP_LDY, ADDR_ABS, 4, "LDY", addr_abs, op_ldy };
    op_info[0xB4] = (OpInfo){ OP_LDY, ADDR_ZPX, 4, "LDY", addr_zpx, op_ldy };
    op_info[0xBC] = (OpInfo){ OP_LDY, ADDR_ABX, 4, "LDY", addr_abx, op_ldy };

    // STX/STY
    op_info[0x86] = (OpInfo){ OP_STX, ADDR_ZP, 3, "STX", addr_zp, op_stx };
    op_info[0x8E] = (OpInfo){ OP_STX, ADDR_ABS, 4, "STX", addr_abs, op_stx };
    op_info[0x96] = (OpInfo){ OP_STX, ADDR_ZPY, 4, "STX", addr_zpy, op_stx };

    op_info[0x84] = (OpInfo){ OP_STY, ADDR_ZP, 3, "STY", addr_zp, op_sty };
    op_info[0x8C] = (OpInfo){ OP_STY, ADDR_ABS, 4, "STY", addr_abs, op_sty };
    op_info[0x94] = (OpInfo){ OP_STY, ADDR_ZPX, 4, "STY", addr_zpx, op_sty };

    // 比较指令 CMP/CPX/CPY
    op_info[0xC9] = (OpInfo){ OP_CMP, ADDR_IMM, 2, "CMP", addr_imm, op_cmp };
    op_info[0xC5] = (OpInfo){ OP_CMP, ADDR_ZP, 3, "CMP", addr_zp, op_cmp };
    op_info[0xCD] = (OpInfo){ OP_CMP, ADDR_ABS, 4, "CMP", addr_abs, op_cmp };
    op_info[0xD5] = (OpInfo){ OP_CMP, ADDR_ZPX, 4, "CMP", addr_zpx, op_cmp };
    op_info[0xDD] = (OpInfo){ OP_CMP, ADDR_ABX, 4, "CMP", addr_abx, op_cmp };
    op_info[0xD9] = (OpInfo){ OP_CMP, ADDR_ABY, 4, "CMP", addr_aby, op_cmp };
    op_info[0xC1] = (OpInfo){ OP_CMP, ADDR_INDX, 6, "CMP", addr_indx, op_cmp };
    op_info[0xD1] = (OpInfo){ OP_CMP, ADDR_INDY, 5, "CMP", addr_indy, op_cmp };

    op_info[0xE0] = (OpInfo){ OP_CPX, ADDR_IMM, 2, "CPX", addr_imm, op_cpx };
    op_info[0xE4] = (OpInfo){ OP_CPX, ADDR_ZP, 3, "CPX", addr_zp, op_cpx };
    op_info[0xEC] = (OpInfo){ OP_CPX, ADDR_ABS, 4, "CPX", addr_abs, op_cpx };

    op_info[0xC0] = (OpInfo){ OP_CPY, ADDR_IMM, 2, "CPY", addr_imm, op_cpy };
    op_info[0xC4] = (OpInfo){ OP_CPY, ADDR_ZP, 3, "CPY", addr_zp, op_cpy };
    op_info[0xCC] = (OpInfo){ OP_CPY, ADDR_ABS, 4, "CPY", addr_abs, op_cpy };

    // AND指令 (逻辑与)
    op_info[0x29] = (OpInfo){ OP_AND, ADDR_IMM, 2, "AND", addr_imm, op_and };
    op_info[0x25] = (OpInfo){ OP_AND, ADDR_ZP, 3, "AND", addr_zp, op_and };
    op_info[0x2D] = (OpInfo){ OP_AND, ADDR_ABS, 4, "AND", addr_abs, op_and };
    op_info[0x35] = (OpInfo){ OP_AND, ADDR_ZPX, 4, "AND", addr_zpx, op_and };
    op_info[0x3D] = (OpInfo){ OP_AND, ADDR_ABX, 4, "AND", addr_abx, op_and };
    op_info[0x39] = (OpInfo){ OP_AND, ADDR_ABY, 4, "AND", addr_aby, op_and };
    op_info[0x21] = (OpInfo){ OP_AND, ADDR_INDX, 6, "AND", addr_indx, op_and };
    op_info[0x31] = (OpInfo){ OP_AND, ADDR_INDY, 5, "AND", addr_indy, op_and };

    // BIT指令
    op_info[0x24] = (OpInfo){ OP_BIT, ADDR_ZP, 3, "BIT", addr_zp, op_bit };
    op_info[0x2C] = (OpInfo){ OP_BIT, ADDR_ABS, 4, "BIT", addr_abs, op_bit_abs };

    // ORA指令 (逻辑或)
    op_info[0x09] = (OpInfo){ OP_ORA, ADDR_IMM, 2, "ORA", addr_imm, op_ora };
    op_info[0x05] = (OpInfo){ OP_ORA, ADDR_ZP, 3, "ORA", addr_zp, op_ora };
    op_info[0x0D] = (OpInfo){ OP_ORA, ADDR_ABS, 4, "ORA", addr_abs, op_ora };
    op_info[0x15] = (OpInfo){ OP_ORA, ADDR_ZPX, 4, "ORA", addr_zpx, op_ora };
    op_info[0x1D] = (OpInfo){ OP_ORA, ADDR_ABX, 4, "ORA", addr_abx, op_ora };
    op_info[0x19] = (OpInfo){ OP_ORA, ADDR_ABY, 4, "ORA", addr_aby, op_ora };
    op_info[0x01] = (OpInfo){ OP_ORA, ADDR_INDX, 6, "ORA", addr_indx, op_ora };
    op_info[0x11] = (OpInfo){ OP_ORA, ADDR_INDY, 5, "ORA", addr_indy, op_ora };

    // EOR指令 (异或)
    op_info[0x49] = (OpInfo){ OP_EOR, ADDR_IMM, 2, "EOR", addr_imm, op_eor };
    op_info[0x45] = (OpInfo){ OP_EOR, ADDR_ZP, 3, "EOR", addr_zp, op_eor };
    op_info[0x4D] = (OpInfo){ OP_EOR, ADDR_ABS, 4, "EOR", addr_abs, op_eor };
    op_info[0x55] = (OpInfo){ OP_EOR, ADDR_ZPX, 4, "EOR", addr_zpx, op_eor };
    op_info[0x5D] = (OpInfo){ OP_EOR, ADDR_ABX, 4, "EOR", addr_abx, op_eor };
    op_info[0x59] = (OpInfo){ OP_EOR, ADDR_ABY, 4, "EOR", addr_aby, op_eor };
    op_info[0x41] = (OpInfo){ OP_EOR, ADDR_INDX, 6, "EOR", addr_indx, op_eor };
    op_info[0x51] = (OpInfo){ OP_EOR, ADDR_INDY, 5, "EOR", addr_indy, op_eor };

    // 堆栈操作
    op_info[0x48] = (OpInfo){ OP_PHA, ADDR_IMPL, 3, "PHA", addr_impl, op_pha };
    op_info[0x68] = (OpInfo){ OP_PLA, ADDR_IMPL, 4, "PLA", addr_impl, op_pla };
    op_info[0x08] = (OpInfo){ OP_PHP, ADDR_IMPL, 3, "PHP", addr_impl, op_php };
    op_info[0x28] = (OpInfo){ OP_PLP, ADDR_IMPL, 4, "PLP", addr_impl, op_plp };

    // 移位指令 (累加器版本)
    op_info[0x0A] = (OpInfo){ OP_ASL, ADDR_ACC, 2, "ASL", addr_acc, op_asl };
    op_info[0x4A] = (OpInfo){ OP_LSR, ADDR_ACC, 2, "LSR", addr_acc, op_lsr };
    op_info[0x2A] = (OpInfo){ OP_ROL, ADDR_ACC, 2, "ROL", addr_acc, op_rol };
    op_info[0x6A] = (OpInfo){ OP_ROR, ADDR_ACC, 2, "ROR", addr_acc, op_ror };

    // 传输指令
    op_info[0xAA] = (OpInfo){ OP_TAX, ADDR_IMPL, 2, "TAX", addr_impl, op_tax };
    op_info[0x8A] = (OpInfo){ OP_TXA, ADDR_IMPL, 2, "TXA", addr_impl, op_txa };
    op_info[0xA8] = (OpInfo){ OP_TAY, ADDR_IMPL, 2, "TAY", addr_impl, op_tay };
    op_info[0x98] = (OpInfo){ OP_TYA, ADDR_IMPL, 2, "TYA", addr_impl, op_tya };
    op_info[0xBA] = (OpInfo){ OP_TSX, ADDR_IMPL, 2, "TSX", addr_impl, op_tsx };
    op_info[0x9A] = (OpInfo){ OP_TXS, ADDR_IMPL, 2, "TXS", addr_impl, op_txs };

    // NOP指令
    op_info[0xEA] = (OpInfo){ OP_NOP, ADDR_IMPL, 2, "NOP", addr_impl, op_nop };

    // 标志位操作
    op_info[0x18] = (OpInfo){ OP_CLC, ADDR_IMPL, 2, "CLC", addr_impl, op_clc };
    op_info[0x38] = (OpInfo){ OP_SEC, ADDR_IMPL, 2, "SEC", addr_impl, op_sec };
    op_info[0x58] = (OpInfo){ OP_CLI, ADDR_IMPL, 2, "CLI", addr_impl, op_cli };
    op_info[0x78] = (OpInfo){ OP_SEI, ADDR_IMPL, 2, "SEI", addr_impl, op_sei };
    op_info[0xD8] = (OpInfo){ OP_CLD, ADDR_IMPL, 2, "CLD", addr_impl, op_cld };
    op_info[0xF8] = (OpInfo){ OP_SED, ADDR_IMPL, 2, "SED", addr_impl, op_sed };
    op_info[0xB8] = (OpInfo){ OP_CLV, ADDR_IMPL, 2, "CLV", addr_impl, op_clv };

    // ASL/LSR/ROL/ROR 内存版本 (之前只注册了累加器版本)
    op_info[0x06] = (OpInfo){ OP_ASL, ADDR_ZP, 5, "ASL", addr_zp, op_asl };
    op_info[0x0E] = (OpInfo){ OP_ASL, ADDR_ABS, 6, "ASL", addr_abs, op_asl };
    op_info[0x16] = (OpInfo){ OP_ASL, ADDR_ZPX, 6, "ASL", addr_zpx, op_asl };
    op_info[0x1E] = (OpInfo){ OP_ASL, ADDR_ABX, 7, "ASL", addr_abx, op_asl };

    op_info[0x46] = (OpInfo){ OP_LSR, ADDR_ZP, 5, "LSR", addr_zp, op_lsr };
    op_info[0x4E] = (OpInfo){ OP_LSR, ADDR_ABS, 6, "LSR", addr_abs, op_lsr };
    op_info[0x56] = (OpInfo){ OP_LSR, ADDR_ZPX, 6, "LSR", addr_zpx, op_lsr };
    op_info[0x5E] = (OpInfo){ OP_LSR, ADDR_ABX, 7, "LSR", addr_abx, op_lsr };

    op_info[0x26] = (OpInfo){ OP_ROL, ADDR_ZP, 5, "ROL", addr_zp, op_rol };
    op_info[0x2E] = (OpInfo){ OP_ROL, ADDR_ABS, 6, "ROL", addr_abs, op_rol };
    op_info[0x36] = (OpInfo){ OP_ROL, ADDR_ZPX, 6, "ROL", addr_zpx, op_rol };
    op_info[0x3E] = (OpInfo){ OP_ROL, ADDR_ABX, 7, "ROL", addr_abx, op_rol };

    op_info[0x66] = (OpInfo){ OP_ROR, ADDR_ZP, 5, "ROR", addr_zp, op_ror };
    op_info[0x6E] = (OpInfo){ OP_ROR, ADDR_ABS, 6, "ROR", addr_abs, op_ror };
    op_info[0x76] = (OpInfo){ OP_ROR, ADDR_ZPX, 6, "ROR", addr_zpx, op_ror };
    op_info[0x7E] = (OpInfo){ OP_ROR, ADDR_ABX, 7, "ROR", addr_abx, op_ror };

    // INC/DEC
    op_info[0xE6] = (OpInfo){ OP_INC, ADDR_ZP, 5, "INC", addr_zp, op_inc };
    op_info[0xEE] = (OpInfo){ OP_INC, ADDR_ABS, 6, "INC", addr_abs, op_inc };
    op_info[0xF6] = (OpInfo){ OP_INC, ADDR_ZPX, 6, "INC", addr_zpx, op_inc };
    op_info[0xFE] = (OpInfo){ OP_INC, ADDR_ABX, 7, "INC", addr_abx, op_inc };

    op_info[0xC6] = (OpInfo){ OP_DEC, ADDR_ZP, 5, "DEC", addr_zp, op_dec };
    op_info[0xCE] = (OpInfo){ OP_DEC, ADDR_ABS, 6, "DEC", addr_abs, op_dec };
    op_info[0xD6] = (OpInfo){ OP_DEC, ADDR_ZPX, 6, "DEC", addr_zpx, op_dec };
    op_info[0xDE] = (OpInfo){ OP_DEC, ADDR_ABX, 7, "DEC", addr_abx, op_dec };

    // INX/INY/DEX/DEY
    op_info[0xE8] = (OpInfo){ OP_INX, ADDR_IMPL, 2, "INX", addr_impl, op_inx };
    op_info[0xC8] = (OpInfo){ OP_INY, ADDR_IMPL, 2, "INY", addr_impl, op_iny };
    op_info[0xCA] = (OpInfo){ OP_DEX, ADDR_IMPL, 2, "DEX", addr_impl, op_dex };
    op_info[0x88] = (OpInfo){ OP_DEY, ADDR_IMPL, 2, "DEY", addr_impl, op_dey };
}
