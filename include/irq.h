#ifndef IRQ_H
#define IRQ_H

#include "../include/cpu.h"

void reset(CPU *cpu);
void irq(CPU *cpu);
void nmi(CPU *cpu);
void check_interrupts(CPU *cpu);

#endif