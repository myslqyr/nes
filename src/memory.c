#include <stdio.h>
#include <string.h>

#include "../include/memory.h"
#include <assert.h>

u8 memory[NES_MEM_SIZE];

void memory_init() {
    for(int i = 0; i < NES_MEM_SIZE; i++) {
        memory[i] = (u8)0x00;
    }
}


