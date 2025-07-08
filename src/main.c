#include "../include/cpu.h"
#include "../include/memory.h"

CPU cpu;

int main() {
    cpu_init(&cpu);
    init_op_table();
    memory_init();
    return 0;
}