#include <stdio.h>

#include "../include/cpu.h"
#include "../include/memory.h"

CPU *cpu;

int main() {
    cpu_init(cpu);
    init_op_table();
    memory_init();

    load_rom("../nes-test/cpu_dummy_reads/cpu_dummy_reads.nes");
    // for(int i = 0;i < sizeof(memory) / sizeof(memory[0]); i++) {
    //     printf("%x ",memory[i]);
    // }
    while(1) {
        cpu_run(cpu);
    }
    return 0;
}