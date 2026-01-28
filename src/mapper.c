#include "../include/mapper.h"

void mapper_init(Mapper *mapper, u8 prg_banks, u8 chr_banks) {
    if (!mapper) return;
    mapper->prg_banks = prg_banks;
    mapper->chr_banks = chr_banks;
    mapper_reset(mapper);
}

void mapper_reset(Mapper *mapper) {
    (void)mapper;
}
