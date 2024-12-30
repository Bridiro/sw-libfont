import os
import sys
import argparse
import freetype
from PIL import Image, ImageDraw, ImageFont
import numpy as np
from scipy.ndimage import distance_transform_edt
import re

def extract_number(filename):
    match = re.search(r'(\d+)', filename)
    return int(match.group(1)) if match else 0

def generate_bitmaps_fixed(font_path, size, output_folder):
    font = ImageFont.truetype(font_path, size)
    
    os.makedirs(output_folder, exist_ok=True)
    
    ascent, descent = font.getmetrics()
    total_height = ascent + descent

    for char_code in range(32, 127):
        char = chr(char_code)

        bbox = font.getbbox(char, anchor="ls")  # "ls" = left baseline
        char_width = bbox[2] - bbox[0]
        char_height = bbox[3] - bbox[1]

        image = Image.new("L", (char_width, total_height), 0)
        draw = ImageDraw.Draw(image)

        x_offset = -bbox[0]
        y_offset = ascent
        draw.text((x_offset, y_offset), char, fill=255, font=font, anchor="ls")

        output_path = os.path.join(output_folder, f"char_{char_code}.png")
        image.save(output_path)
        

def generate_bitmaps(font_path, size, output_folder):
    def render_bitmap(char, font_path, size=32):
        face = freetype.Face(font_path)
        face.set_pixel_sizes(0, size)
        face.load_char(char)
        bitmap = face.glyph.bitmap

        width, height = bitmap.width, bitmap.rows
        if width == 0 or height == 0:
            print(f"Carattere '{char}' non ha bitmap (probabilmente spazio).")
            return np.zeros((size // 4, size // 4), dtype=np.uint8)

        array = np.array(bitmap.buffer, dtype=np.uint8).reshape(height, width)
        return array

    os.makedirs(output_folder, exist_ok=True)

    for char_code in range(32, 127):
        bitmap = render_bitmap(char_code, font_path, size)
        image = Image.fromarray(bitmap)
        image.save(os.path.join(output_folder, f"char_{char_code}.png"))

def generate_sdf(bitmap_folder, output_folder):
    os.makedirs(output_folder, exist_ok=True)

    def compute_sdf(bitmap):
        inside = distance_transform_edt(bitmap)
        outside = distance_transform_edt(1 - bitmap)
        sdf = inside - outside
        normalized_sdf = np.clip((sdf + 3) / 6, 0, 1) * 255
        return normalized_sdf.astype(np.uint8)

    for file_name in os.listdir(bitmap_folder):
        if file_name.endswith(".png"):
            image = Image.open(os.path.join(bitmap_folder, file_name)).convert("L")
            bitmap = np.array(image) > 128
            sdf = compute_sdf(bitmap)
            sdf_image = Image.fromarray(sdf)
            sdf_image.save(os.path.join(output_folder, file_name.replace("char_", "sdf_")))


def generate_c_files(sdf_folder, output_c_folder, output_inc_folder):
    os.makedirs(output_c_folder, exist_ok=True)
    os.makedirs(output_inc_folder, exist_ok=True)
    c_file_path = os.path.join(output_c_folder, "font.c")
    h_file_path = os.path.join(output_inc_folder, "font.h")

    with open(c_file_path, "w") as c_file, open(h_file_path, "w") as h_file:
        c_file.write("#include <stdint.h>\n")
        c_file.write("#include \"font.h\"\n\n")
        h_file.write("#ifndef FONT_H\n#define FONT_H\n\n#include <stdint.h>\n\n")

        glyph_metadata = []

        for file_name in sorted(os.listdir(sdf_folder), key=extract_number):
            if file_name.endswith(".png"):
                char_code = int(file_name.split("_")[1].split(".")[0])
                array_name = f"glyph_{char_code}"
                image = Image.open(os.path.join(sdf_folder, file_name)).convert("L")
                width, height = image.size
                pixels = list(image.getdata())

                c_file.write(f"static const uint8_t {array_name}[{width} * {height}] = {{\n")
                for i, p in enumerate(pixels):
                    c_file.write(f"{p}, ")
                    if (i + 1) % width == 0:
                        c_file.write("\n")
                c_file.write("};\n\n")

                glyph_metadata.append((char_code, array_name, width, height))

        c_file.write("\nconst Glyph glyphs[] = {\n")

        for char_code, array_name, width, height in glyph_metadata:
            c_file.write(f"    {{ {array_name}, {width}, {height} }}, // Char \"{chr(char_code)}\"\n")

        c_file.write("};\n")

        h_file.write("typedef struct {\n")
        h_file.write("    const uint8_t *sdf;\n    uint8_t width;\n    uint8_t height;\n} Glyph;\n\n")
        h_file.write("extern const Glyph glyphs[];\n\n")
        h_file.write("#endif // FONT_H\n")


def main():
    parser = argparse.ArgumentParser(description="Generate bitmap, SDF file, C file and Header file.")
    parser.add_argument("--font", required=True, help="TTF file.")
    parser.add_argument("--size", type=int, default=64, help="Size of bitmaps (default: 64).")
    parser.add_argument("--bitmap_folder", default="bitmaps", help="Folder which contains the generated bitmaps.")
    parser.add_argument("--sdf_folder", default="sdf_fonts", help="Folder which contains the generated SDF.")
    parser.add_argument("--c_folder", default="../src", help="Folder which contains the C file.")
    parser.add_argument("--inc_folder", default="../inc", help="Folder which contains the include file.")
    args = parser.parse_args()

    print("Bitmap generation...")
    generate_bitmaps_fixed(args.font, args.size, args.bitmap_folder)

    print("SDF generation...")
    generate_sdf(args.bitmap_folder, args.sdf_folder)

    print("C files generation...")
    generate_c_files(args.sdf_folder, args.c_folder, args.inc_folder)

    print("Process completed!")

if __name__ == "__main__":
    main()

