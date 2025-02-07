/**
 * @file text.c
 * @date 2024-12-30
 * @author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * @brief Text rendering functions
 */

#include "text.h"
#include "font.h"


void _draw_rle_series(uint8_t count, uint8_t value, int x, int y, float size, int glyph_width, int glyph_height, int *current_x, int *current_y, uint32_t color, draw_pixel_callback_t pixel_callback) {
    for (uint8_t k = 0; k < count; ++k) {
        // Render the decompressed pixel
        if (value > 0) { // Only render non-transparent pixels
            int scaled_x = (int)(x + *current_x * size);
            int scaled_y = (int)(y + *current_y * size);

            for (int j = 0; j < size; ++j) {
                for (int i = 0; i < size; ++i) {
                    if (value > 0) {
                        uint32_t blended_color = (color & 0x00ffffff) | ((uint32_t)value << 24);
                        pixel_callback(scaled_x + i, scaled_y + j, blended_color);
                    }
                }
            }
        }

        // Advance to the next pixel
        (*current_x)++;
        if (*current_x >= glyph_width) {
            *current_x = 0;
            (*current_y)++;
            if (*current_y >= glyph_height)
                break;
        }
    }
}


void _render_glyph(const Glyph *glyph, int x, int y, uint32_t color, float size, draw_pixel_callback_t pixel_callback) {
    const uint8_t *data = &sdf_data[glyph->offset];
    uint16_t remaining_size = glyph->size;

    int glyph_width = glyph->width;
    int glyph_height = glyph->height;

    int current_x = 0;
    int current_y = 0;

    while (remaining_size > 0 && current_y < glyph_height) {
        // Decode the value and run length
        uint8_t value_raw = *data++;
        uint8_t value1 = ((value_raw >> 4) & 0xF) * 16;
        uint8_t value2 = (value_raw & 0xF) * 16;
        uint8_t count1 = *data++;
        uint8_t count2 = *data++;
        remaining_size -= 2;

        _draw_rle_series(count1, value1, x, y, size, glyph_width, glyph_height, &current_x, &current_y, color, pixel_callback);
        _draw_rle_series(count2, value2, x, y, size, glyph_width, glyph_height, &current_x, &current_y, color, pixel_callback);
    }
}


void draw_text(uint16_t x, uint16_t y, enum FontAlign align, char *text, uint32_t color, float size, draw_pixel_callback_t pixel_callback) {
    if (align == CENTER) {
        // To center it just shift x half the text lenght back
        uint16_t len = text_lenght(text, size);
        x -= len / 2;
    } else if (align == RIGHT) {
        // To align it to the right, just shift x the text lenght back
        uint16_t len = text_lenght(text, size);
        x -= len;
    }
    while (*text) {
        int char_code = *text++;
        if (char_code >= 32 && char_code <= 126) {
            // Get glyph, print it and then move x for next character
            const Glyph *glyph = &glyphs[char_code - 32];
            _render_glyph(glyph, x, y, color, size, pixel_callback);
            x += glyph->width * size;
        }
    }
}

uint16_t text_lenght(char *text, float size) {
    float tot = 0;
    while (*text) {
        int char_code = *text++;
        if (char_code >= 32 && char_code <= 126) {
            // Get width from every glyph descriptor, multiply it with size and add to tot
            tot += (float) (&glyphs[char_code - 32])->width * size;
        }
    }
    return (uint16_t) tot;
}

#if UTILITIES
uint8_t get_alpha(uint32_t color) {
    return (color >> 24) & 0xff;
}

uint8_t get_red(uint32_t color) {
    return (color >> 16) & 0xff;
}

uint8_t get_green(uint32_t color) {
    return (color >> 8) & 0xff;
}

uint8_t get_blue(uint32_t color) {
    return color & 0xff;
}
#endif // UTILITIES
