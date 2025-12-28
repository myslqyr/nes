#ifndef BUS_H
#define BUS_H

#include "memory.h"

void bus_write(u16 addr, u8 data);
u8 bus_read(u16 addr);

#endif