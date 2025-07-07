#include "include/cpu.h"

void cpu_init(CPU *cpu) {
    cpu->P = 0;
    cpu->A = 0;
    cpu->X = 0;
    cpu->Y = 0;
    cpu->S = 0;
    cpu->PC = 0;
}
