#ifndef MAPPER_H
#define MAPPER_H

#include "type.h"

typedef struct {
    u8 prg_banks;
    u8 chr_banks;
} Mapper;

void mapper_init(Mapper *mapper, u8 prg_banks, u8 chr_banks);
void mapper_reset(Mapper *mapper);

#endif
