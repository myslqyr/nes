#include "../include/cpu.h"

CPU cpu;

int main() {
    cpu_init(&cpu);
    init_op_table();
    return 0;
}