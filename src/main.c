#include <stdio.h>

#include "../include/cpu.h"
#include "../include/memory.h"

CPU cpu;

int main() {
    cpu_init(&cpu);
    init_op_table();
    memory_init();

    load_rom("./cpu_dummy_reads.nes");
    for(int i = 0;i < sizeof(memory) / sizeof(memory[0]); i++) {
        printf("%x ",memory[i]);
    }
    return 0;
}