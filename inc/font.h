#ifndef FONT_H
#define FONT_H

#include <stdint.h>

typedef struct {
    uint32_t offset;
    uint16_t size;
    uint8_t width;
    uint8_t height;
} Glyph;

extern const uint8_t sdf_data[];
extern const Glyph glyphs[];

#endif // FONT_H
