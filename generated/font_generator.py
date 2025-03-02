import os
import argparse
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

        image = Image.new("L", (char_width, total_height), 0)
        draw = ImageDraw.Draw(image)

        x_offset = -bbox[0]
        y_offset = ascent
        draw.text((x_offset, y_offset), char, fill=255, font=font, anchor="ls")

        output_path = os.path.join(output_folder, f"char_{char_code}.png")
        image.save(output_path)
        

def generate_sdf(bitmap_folder, output_folder, edge_down, edge_up):
    os.makedirs(output_folder, exist_ok=True)

    def smoothstep(edge0, edge1, x):
        t = np.clip((x - edge0) / (edge1 - edge0), 0, 1)
        return t * t * (3 - 2 * t)


    def compute_sdf(bitmap):
        inside = distance_transform_edt(bitmap)
        outside = distance_transform_edt(1 - bitmap)
        sdf = inside - outside
        normalized_sdf = np.clip((sdf + 3) / 6, 0, 1)
        normalized_sdf = smoothstep(edge_down, edge_up, normalized_sdf) * 255
        return normalized_sdf.astype(np.uint8)

    for file_name in os.listdir(bitmap_folder):
        if file_name.endswith(".png"):
            image = Image.open(os.path.join(bitmap_folder, file_name)).convert("L")
            bitmap = np.array(image) > 128
            sdf = compute_sdf(bitmap)
            sdf_image = Image.fromarray(sdf)
            sdf_image.save(os.path.join(output_folder, file_name.replace("char_", "sdf_")))


def compress_rle_4bit_paired(data):
    compressed = []
    i = 0

    while i < len(data):
        sdf1 = data[i] // 16
        count1 = 1
        i += 1

        while i < len(data) and data[i] // 16 == sdf1 and count1 < 255:
            count1 += 1
            i += 1

        if i < len(data):
            sdf2 = data[i] // 16
            count2 = 1
            i += 1

            while i < len(data) and data[i] // 16 == sdf2 and count2 < 255:
                count2 += 1
                i += 1
        else:
            count2 = 0

        compressed.append(((sdf1 << 4) | sdf2, count1, count2))

    return compressed


def generate_c_files(sdf_folder, output_c_folder, output_inc_folder, font_name, size):
    os.makedirs(output_c_folder, exist_ok=True)
    os.makedirs(output_inc_folder, exist_ok=True)

    c_file_path = os.path.join(output_c_folder, f"font_{font_name}_{size}.c")
    h_file_path = os.path.join(output_inc_folder, f"font_{font_name}_{size}.h")

    with open(c_file_path, "w") as c_file, open(h_file_path, "w") as h_file:
        # Header file content
        h_file.write(f"#ifndef FONT_{font_name.upper()}_{size}_H\n#define FONT_{font_name.upper()}_{size}_H\n\n#include <stdint.h>\n#include \"glyph.h\"\n\n")
        h_file.write(f"extern const uint8_t sdf_data_{font_name}_{size}[];\n")
        h_file.write(f"extern const Glyph glyphs_{font_name}_{size}[];\n\n")
        h_file.write(f"#endif // FONT_{font_name.upper()}_{size}_H\n")

        # C file content
        c_file.write(f"#include \"font_{font_name}_{size}.h\"\n\n")

        sdf_data = []
        glyph_metadata = []

        for file_name in sorted(os.listdir(sdf_folder), key=extract_number):
            if file_name.endswith(".png"):
                image = Image.open(os.path.join(sdf_folder, file_name)).convert("L")
                width, height = image.size
                pixels = list(image.getdata())
                compressed = compress_rle_4bit_paired(pixels)
                offset = len(sdf_data)

                # Append SDF data
                sdf_data.extend([item for pair in compressed for item in pair])
                glyph_metadata.append((offset, len(compressed) * 2, width, height))

        # Write SDF data as a single array
        c_file.write(f"const uint8_t sdf_data_{font_name}_{size}[] = {{\n")
        for i, value in enumerate(sdf_data):
            c_file.write(f"{value}, ")
            if (i + 1) % 12 == 0:
                c_file.write("\n")
        c_file.write("};\n\n")

        c_file.write(f"const Glyph glyphs_{font_name}_{size}[] = {{\n")
        for offset, size, width, height in glyph_metadata:
            c_file.write(f"    {{ {offset}, {size}, {width}, {height} }},\n")
        c_file.write("};\n")


def generate_font_h(output_inc_folder):
    font_definitions = []
    font_entries = []
    enum_entries = []

    for file_name in os.listdir(output_inc_folder):
        match = re.match(r'font_(\w+)_(\d+)\.h', file_name)
        if match:
            font_name = match.group(1)
            font_size = match.group(2)
            font_definitions.append(f"#include \"{file_name}\"")
            font_entries.append(f"    {{ {font_size}, sdf_data_{font_name}_{font_size}, glyphs_{font_name}_{font_size} }},")
            enum_entries.append(f"    {font_name.upper()}_{font_size},\n")

    h_file_path = os.path.join(output_inc_folder, "font.h")
    with open(h_file_path, "w") as h_file:
        h_file.write("#ifndef FONT_H\n#define FONT_H\n\n#include <stdint.h>\n\n")
        h_file.write("\n".join(font_definitions) + "\n\n")

        h_file.write("typedef struct {\n")
        h_file.write("    uint8_t size;\n")
        h_file.write("    const uint8_t* sdf_data;\n")
        h_file.write("    const Glyph* glyphs;\n")
        h_file.write("} Font;\n\n")

        h_file.write("typedef enum {\n")
        h_file.write("".join(enum_entries))
        h_file.write("} FontName;\n\n")

        h_file.write("static const Font fonts[] = {\n")
        h_file.write("\n".join(font_entries) + "\n")
        h_file.write("};\n\n")

        h_file.write("#endif // FONT_H\n")


def main():
    parser = argparse.ArgumentParser(description="Generate bitmap, SDF file, C file and Header file.")
    parser.add_argument("--font", required=True, help="TTF file.")
    parser.add_argument("--size", type=int, default=64, help="Size of bitmaps (default: 64).")
    parser.add_argument("--font_name", required=True, help="Name of the font")
    parser.add_argument("--edge_down", default=0.35, help="Lower edge of the smoothstep function.")
    parser.add_argument("--edge_up", default=0.75, help="Higher edge of the smoothstep function.")
    parser.add_argument("--bitmap_folder", default="bitmaps", help="Folder which contains the generated bitmaps.")
    parser.add_argument("--sdf_folder", default="sdf_fonts", help="Folder which contains the generated SDF.")
    parser.add_argument("--c_folder", default="../src", help="Folder which contains the C file.")
    parser.add_argument("--inc_folder", default="../include", help="Folder which contains the include file.")
    args = parser.parse_args()

    print("Bitmap generation...")
    generate_bitmaps_fixed(args.font, args.size, args.bitmap_folder)

    print("SDF generation...")
    generate_sdf(args.bitmap_folder, args.sdf_folder, float(args.edge_down), float(args.edge_up))

    print("C files generation...")
    generate_c_files(args.sdf_folder, args.c_folder, args.inc_folder, args.font_name, args.size)

    print("Updating font.h")
    generate_font_h(args.inc_folder)

    print("Process completed!")

if __name__ == "__main__":
    main()

