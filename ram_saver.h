#ifndef INCLUDE_RAM_SAVER_H
#define INCLUDE_RAM_SAVER_H

#include <stdint.h>
#include <stddef.h>

extern void saveRam(uint8_t* ram, size_t len, char* path, const uint8_t &flags);

#endif
