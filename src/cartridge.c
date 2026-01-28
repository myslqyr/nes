#include "../include/cartridge.h"
#include "../include/mapper0.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>


static u8 *prg_rom = NULL;
static u8 *chr_rom = NULL;
static int prg_rom_size = 0;
static int chr_rom_size = 0;
static bool chr_is_ram = false;
static u8 mapper_id = 0;
static MirrorMode mirror_mode = MIRROR_HORIZONTAL;

MirrorMode cartridge_get_mirror(void) {
    return mirror_mode;
}

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

    mapper_id = (header.mapper2 & 0xF0) | (header.mapper1 >> 4);
    if (mapper_id != 0) {
        printf("Mapper %d not supported (only Mapper 0)\n", mapper_id);
        fclose(fp);
        return false;
    }

    //跳过 Trainer
    if (header.mapper1 & 0x04) {
        fseek(fp, 512, SEEK_CUR);
    }

    prg_rom_size = header.prg_rom_chunks * 16 * 1024;
    chr_rom_size = header.chr_rom_chunks * 8 * 1024;
    mirror_mode = (header.mapper1 & 0x01) ? MIRROR_VERTICAL : MIRROR_HORIZONTAL;

    prg_rom = malloc(prg_rom_size);
    fread(prg_rom, 1, prg_rom_size, fp);

    if (chr_rom_size > 0) {
        chr_rom = malloc(chr_rom_size);
        fread(chr_rom, 1, chr_rom_size, fp);
        chr_is_ram = false;
    } else {
        chr_rom_size = 8 * 1024;
        chr_rom = malloc(chr_rom_size);
        memset(chr_rom, 0, chr_rom_size);
        chr_is_ram = true;
    }

    fclose(fp);

    printf("ROM loaded: PRG=%dKB CHR=%dKB\n",
           prg_rom_size / 1024,
           chr_rom_size / 1024);

    return true;
}


u8 cartridge_cpu_read(u16 addr) {
    u32 mapped_addr = 0;
    if (mapper0_cpu_map_read(addr, prg_rom_size, &mapped_addr)) {
        return prg_rom[mapped_addr];
    }
    return 0;
}

void cartridge_cpu_write(u16 addr, u8 data) {
    u32 mapped_addr = 0;
    if (mapper0_cpu_map_write(addr, prg_rom_size, &mapped_addr)) {
        (void)data;
    }
}

u8 cartridge_ppu_read(u16 addr) {
    u32 mapped_addr = 0;
    if (mapper0_ppu_map_read(addr, chr_rom_size, &mapped_addr)) {
        return chr_rom[mapped_addr];
    }
    return 0;
}

void cartridge_ppu_write(u16 addr, u8 data) {
    u32 mapped_addr = 0;
    if (mapper0_ppu_map_write(addr, chr_rom_size, chr_is_ram, &mapped_addr)) {
        chr_rom[mapped_addr] = data;
    }
}
