#ifndef FONT_H
#define FONT_H

#include <stdint.h>

typedef struct {
    const uint8_t *sdf;
    uint8_t width;
    uint8_t height;
} Glyph;

extern const Glyph glyphs[];

#endif // FONT_H
