#ifndef GLYPH_H
#define GLYPH_H

#include <stdint.h>

typedef struct {
    uint32_t offset;
    uint16_t size;
    uint8_t width;
    uint8_t height;
} Glyph;

#endif // GLYPH_H
