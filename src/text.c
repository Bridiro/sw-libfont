#include "text.h"
#include "font.h"

float _get_sdf_value_bilinear(const Glyph *glyph, float x, float y)
{
    const uint8_t *sdf = &sdf_data[glyph->offset];

    int x0 = (int)x;
    int y0 = (int)y;
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    x0 = x0 < 0 ? 0 : (x0 >= glyph->width ? glyph->width - 1 : x0);
    y0 = y0 < 0 ? 0 : (y0 >= glyph->height ? glyph->height - 1 : y0);
    x1 = x1 < 0 ? 0 : (x1 >= glyph->width ? glyph->width - 1 : x1);
    y1 = y1 < 0 ? 0 : (y1 >= glyph->height ? glyph->height - 1 : y1);

    float q00 = sdf[y0 * glyph->width + x0];
    float q01 = sdf[y0 * glyph->width + x1];
    float q10 = sdf[y1 * glyph->width + x0];
    float q11 = sdf[y1 * glyph->width + x1];

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
            
            float alpha = _smoothstep(0.2, 0.80, sdf_value / 255.0f);
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


void _render_glyph_rle(const Glyph *glyph, int x, int y, uint32_t color, float size)
{
    const uint8_t *data = &sdf_data[glyph->offset];
    uint16_t remaining_size = glyph->size;

    int glyph_width = glyph->width;
    int glyph_height = glyph->height;

    int current_x = 0;
    int current_y = 0;

    while (remaining_size > 0 && current_y < glyph_height)
    {
        // Decode the value and run length
        uint8_t value = *data++;
        uint8_t count = *data++;
        remaining_size -= 2;

        for (uint8_t k = 0; k < count; ++k)
        {
            // Render the decompressed pixel
            if (value > 0) // Only render non-transparent pixels
            {
                int scaled_x = (int)(x + current_x * size);
                int scaled_y = (int)(y + current_y * size);

                for (int j = 0; j < size; ++j)
                {
                    for (int i = 0; i < size; ++i)
                    {
                        // Calculate alpha using value and blend it with the color
                        float alpha = _smoothstep(0.2, 0.80, value / 255.0f);
                        if (alpha > 0.0f)
                        {
                            uint8_t to_pass = (uint8_t)(alpha * 255);
                            uint32_t blended_color = (color & 0x00ffffff) | ((uint32_t)to_pass << 24);
                            draw_pixel(scaled_x + i, scaled_y + j, blended_color);
                        }
                    }
                }
            }

            // Advance to the next pixel
            current_x++;
            if (current_x >= glyph_width)
            {
                current_x = 0;
                current_y++;
                if (current_y >= glyph_height)
                    break;
            }
        }
    }
}


void draw_text(uint16_t x, uint16_t y, enum FontAlign align, char *text, uint32_t color, float size)
{
    if (align == CENTER)
    {
        uint16_t len = text_lenght(text, size);
        x -= len / 2;
    } else if (align == RIGHT)
    {
        uint16_t len = text_lenght(text, size);
        x -= len;
    }
    while (*text)
    {
        int char_code = *text++;
        if (char_code >= 32 && char_code <= 126)
        {
            const Glyph *glyph = &glyphs[char_code - 32];
            _render_glyph_rle(glyph, x, y, color, size);
            x += glyph->width * size;
        }
    }
}

uint16_t text_lenght(char *text, float size)
{
    float tot = 0;
    while (*text)
    {
        int char_code = *text++;
        if (char_code >= 32 && char_code <= 126)
        {
            tot += (float) (&glyphs[char_code - 32])->width * size;
        }
    }
    return (uint16_t) tot;
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
