# Font Rendering Library

This library provides and efficient way to render scalable text direcly onto a
framebuffer. It is completely abstract from the hardware.
Using Signed Distance Field (SDF) and Run-Lenght Encoding (RLE) compression,
it minimizes memory usage and computational overhead while supporting runtime
scaling.

## Features
- **Scalable Font:** Render text at any size using SDF-based fonts.
- **RLE Compression:** Compact font data storage with efficient decompression
- **Lightweight Design:** Optimized for systems with limited resources
- **Customizable Colors and Alignment:** Specify text color, size and alignment
- **Framebuffer Inetgration:** Designed to work seamlessly with pixel-based framebuffers

## File Structure
```
sw-lib-font/
├── src/
│   ├── text.c             # Utilities and font render function
│   ├── font.c             # Font SDF data (generated)
│   └── main.c             # Demo application to display capabilities
├── inc/
│   ├── text.h             # Header file for text.c
│   └── font.h             # Definitions for font creation (generated)
├── generated/
│   ├── sdf_generator.py   # SDF Font generator
│   ├── font.ttf           # Font to use in the library
│   ├── bitmaps/           # Folder to contain bitmaps used
│   └── sdf_fonts/         # Folder to contain generated SDFs
├── build/                 # Compiled object files
└── app                    # Final compiled application
```

## Usage

1. **Include the Header:**
Include `text.h` in your source file to access the functions to use.
```c
#include "text.h"
```

2. **Initialize Graphical API:**
The library require access to a function used to draw a pixel (the user defines it).
```c
draw_pixel = <your_draw_pixel_function>
```

3. **Render Text:**
Setup and render whichever text you want to.
```c
uint16_t x = 100;                // x position in framebuffer
uint16_t y = 50;                 // y position in framebuffer
enum FontAlign align = CENTER;   // text centering on the point
char *text = "Hello, World!";    // text to render
uint32_t color = 0xffffffff;     // which color to use to draw the text
float size = 1.0f;               // scale factor

draw_text(x, y, align, text, color, size);
```

## Compilation

To compile the project and experiment with the test, run `make` inside the project root.
