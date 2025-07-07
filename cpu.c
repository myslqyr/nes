#include "include/cpu.h"

void cpu_init(CPU *cpu) {
    cpu->P = 0;
    cpu->A = 0;
    cpu->X = 0;
    cpu->Y = 0;
    cpu->S = 0;
    cpu->PC = 0;
}

/*
    void set_flag(CPU *cpu, u8 flag, bool value)
    设置标志位
    flag: 标志位
    value: bool值,true为设置,false为清除
*/

void set_flag(CPU *cpu, u8 flag, bool value) {
    if (value) {
        cpu->P |= flag;
    } else {
        cpu->P &= ~flag;
    }
}