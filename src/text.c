#include "text.h"
#include "font.h"
#include <stdio.h>

float _get_sdf_value_bilinear(const Glyph *glyph, float x, float y)
{
    int x0 = (int)x;
    int y0 = (int)y;
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    x0 = x0 < 0 ? 0 : (x0 >= glyph->width ? glyph->width - 1 : x0);
    y0 = y0 < 0 ? 0 : (y0 >= glyph->height ? glyph->height - 1 : y0);
    x1 = x1 < 0 ? 0 : (x1 >= glyph->width ? glyph->width - 1 : x1);
    y1 = y1 < 0 ? 0 : (y1 >= glyph->height ? glyph->height - 1 : y1);

    float q00 = glyph->sdf[y0 * glyph->width + x0];
    float q01 = glyph->sdf[y0 * glyph->width + x1];
    float q10 = glyph->sdf[y1 * glyph->width + x0];
    float q11 = glyph->sdf[y1 * glyph->width + x1];

    float tx = x - x0;
    float ty = y - y0;

    float value = (1 - tx) * (1 - ty) * q00 +
                  tx * (1 - ty) * q01 +
                  (1 - tx) * ty * q10 +
                  tx * ty * q11;

    return value;
}

float _smoothstep(float edge0, float edge1, float x)
{
    x = (x - edge0) / (edge1 - edge0);
    x = x < 0 ? 0 : (x > 1 ? 1 : x);
    return x * x * (3 - 2 * x);
}

void _render_glyph(const Glyph *glyph, int x, int y, uint32_t color, float size)
{
    for (int j = 0; j < glyph->height * size; j++)
    {
        for (int i = 0; i < glyph->width * size; i++)
        {
            int sdf_x = i / size;
            int sdf_y = j / size;

            float sdf_value = _get_sdf_value_bilinear(glyph, sdf_x, sdf_y);
            
            float alpha = _smoothstep(0.2, 0.75, sdf_value / 255.0f);
            if (alpha > 0.0f)
            {
                uint8_t to_pass = (uint8_t)(alpha * 255);
                color &= 0x00ffffff;
                color |= ((uint32_t) to_pass << 24);
                draw_pixel(x + i, y + j, color);
            }
        }
    }
}

void draw_text(uint16_t x, uint16_t y, enum FontAlign align, char *text, uint32_t color, float size)
{
    while (*text)
    {
        int char_code = *text++;
        if (char_code >= 32 && char_code <= 126)
        {
            const Glyph *glyph = &glyphs[char_code - 32];
            _render_glyph(glyph, x, y, color, size);
            x += glyph->width * size;
        }
    }
}

#if UTILITIES
uint8_t get_alpha(uint32_t color)
{
    return (color >> 24) & 0xff;
}

uint8_t get_red(uint32_t color)
{
    return (color >> 16) & 0xff;
}

uint8_t get_green(uint32_t color)
{
    return (color >> 8) & 0xff;
}

uint8_t get_blue(uint32_t color)
{
    return color & 0xff;
}
#endif // UTILITIES
