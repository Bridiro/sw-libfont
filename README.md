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
│   ├── libfont.c                # Utilities and font render function
│   └── font_<name_of_font>.c    # Font SDF data (generated)
├── inc/
│   ├── libfont.h                # Header file for libfont.c
│   ├── font_<name_of_font>.h    # Definition for single font (generated)
│   └── font.h                   # Definitions for using all the generated fonts (generated)
├── test/
│   └── main.c                   # Demo application to display capabilities
├── generated/
│   ├── font_generator.py        # SDF Font generator
│   ├── font.ttf                 # Font to use in the library
│   ├── bitmaps/                 # Folder to contain bitmaps used
│   └── sdf_fonts/               # Folder to contain generated SDFs
├── build/                       # Compiled object files
├── library.json                 # File used by PlatformIO to identify library
└── app                          # Final compiled application
```

## Usage
1. **Generate Font:**
Go inside the `generated/` folder and run the python script passing all the needed parameters. Run `python font_generator.py --help` for more informations.

2. **Include the Header:**
Include `libfont.h` in your source file to access the functions to use.
```c
#include "libfont.h"
```

3. **Render Text:**
Setup and render whichever text you want to.
```c
uint16_t x = 100;                       // x position in framebuffer
uint16_t y = 50;                        // y position in framebuffer
FontAlign align = FONT_ALIGN_CENTER;    // text centering on the point
FontName font = KONEXY_100;             // Font name (check font.c)
char *text = "Hello, World!";           // text to render
uint32_t color = 0xffffffff;            // which color to use to draw the text
float size = 1.0f;                      // scale factor

draw_text(x, y, align, font, text, color, size, draw_line_callback);
```

## Compilation

To compile the project and experiment with the test, run `make` inside the project root.
