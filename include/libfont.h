/**
 * @file libfont.h
 * @date 2024-12-30
 * @author Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * @brief Text rendering functions
 */

#ifndef TEXT_H
#define TEXT_H

#include <stdint.h>

#define UTILITIES 0

/**
 * @brief Callback used to draw pixel
 */
typedef void (*draw_line_callback_t)(uint16_t x, uint16_t y, uint16_t lenght, uint32_t color);

/**
 * @brief Represents the alignment of the rendering of the text
 *
 * @details
 *     - LEFT means that the text rendering will start from Coords.x to the left
 *     - CENTER means that the text rendering will be centered to Coords.x
 *     - RIGHT means that the text rendering will end at Coords.x
 */
enum FontAlign {
    LEFT,
    CENTER,
    RIGHT
};

/**
 * @brief Draws text
 *
 * @param x Position on x axis in buffer
 * @param y Position on y axis in buffer
 * @param align Alignment on x axis
 * @param text Pointer to text to draw
 * @param color ARGB color value (alpha is ignored)
 * @param size Scale value
 */
void draw_text(uint16_t x, uint16_t y, enum FontAlign align, char *text, uint32_t color, uint16_t pixel_size, draw_line_callback_t line_callback);

/**
 * @brief Calcaulate the lenght on the text in pixel
 */
uint16_t text_lenght(char *text, uint16_t pixel_size);

#if UTILITIES
/**
 * @brief Utility to extract 8 bit alpha value from ARGB8888 format
 *
 * @param color Color value
 * @return alpha value
 */
uint8_t get_alpha(uint32_t color);

/**
 * @brief Utility to extract 8 bit red value from ARGB8888 format
 *
 * @param color Color value
 * @return red value
 */
uint8_t get_red(uint32_t color);

/**
 * @brief Utility to extract 8 bit green value from ARGB8888 format
 *
 * @param color Color value
 * @return green value
 */
uint8_t get_green(uint32_t color);

/**
 * @brief Utility to extract 8 bit blue value from ARGB8888 format
 *
 * @param color Color value
 * @return blue value
 */
uint8_t get_blue(uint32_t color);
#endif // UTILITIES

#endif // TEXT_H
