#include "../include/cartridge.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>


static u8 *prg_rom = NULL;
static u8 *chr_rom = NULL;
static int prg_rom_size = 0;
static int chr_rom_size = 0;

bool cartridge_load(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf("Failed to open ROM: %s\n", filename);
        return false;
    }

    iNESHeader header;
    fread(&header, 1, sizeof(header), fp);

    if (memcmp(header.name, "NES\x1A", 4) != 0) {
        printf("Not a valid iNES file\n");
        fclose(fp);
        return false;
    }

    //跳过 Trainer
    if (header.mapper1 & 0x04) {
        fseek(fp, 512, SEEK_CUR);
    }

    prg_rom_size = header.prg_rom_chunks * 16 * 1024;
    chr_rom_size = header.chr_rom_chunks * 8 * 1024;

    prg_rom = malloc(prg_rom_size);
    fread(prg_rom, 1, prg_rom_size, fp);

    if (chr_rom_size > 0) {
        chr_rom = malloc(chr_rom_size);
        fread(chr_rom, 1, chr_rom_size, fp);
    }

    fclose(fp);

    printf("ROM loaded: PRG=%dKB CHR=%dKB\n",
           prg_rom_size / 1024,
           chr_rom_size / 1024);

    return true;
}


u8 cartridge_cpu_read(u16 addr) {
    if (addr >= 0x8000) {
        if (prg_rom_size == 16 * 1024) //16KB
            return prg_rom[(addr - 0x8000) & 0x3FFF];
        else
            return prg_rom[addr - 0x8000];
    }
    return 0;
}