#!/usr/bin/env python3
"""
Convert a Conway's Game of Life plaintext pattern file (.cells) to a PNG image.

Supports:
- Comments: lines starting with '!'
- Dead cells: '.'
- Alive cells: 'O' or '*' (case-insensitive)
- Variable row lengths (shorter rows are padded with dead cells)
"""

import argparse
from PIL import Image

def read_plaintext(file_path):
    """Parse a plaintext .cells file and return a 2D boolean grid (True = alive)."""
    with open(file_path, 'r') as f:
        lines = f.readlines()

    # Remove comment lines and trailing newlines
    data_lines = [line.rstrip('\n') for line in lines if not line.startswith('!')]
    if not data_lines:
        raise ValueError("No pattern data found in file.")

    # Determine max width (for padding shorter rows)
    max_width = max(len(line) for line in data_lines)

    grid = []
    for line in data_lines:
        # Pad line to max width with '.' (dead cells)
        padded = line.ljust(max_width, '.')
        row = []
        for ch in padded:
            if ch in ('O', 'o', '*'):
                row.append(True)     # alive
            else:
                row.append(False)    # dead (including '.' or any other char)
        grid.append(row)

    return grid

def grid_to_image(grid, cell_size, dead_color, alive_color):
    """Create a Pillow Image from the boolean grid."""
    height = len(grid)
    width = len(grid[0]) if grid else 0
    if width == 0 or height == 0:
        raise ValueError("Empty grid.")

    img = Image.new('RGB', (width * cell_size, height * cell_size), dead_color)

    # Draw alive cells as rectangles
    for y, row in enumerate(grid):
        for x, alive in enumerate(row):
            if alive:
                x0 = x * cell_size
                y0 = y * cell_size
                x1 = x0 + cell_size
                y1 = y0 + cell_size
                # Draw a filled rectangle for the cell
                for dy in range(cell_size):
                    for dx in range(cell_size):
                        img.putpixel((x0 + dx, y0 + dy), alive_color)

    # Alternatively, use ImageDraw for better performance, but putpixel is fine for small images.
    return img

def main():
    parser = argparse.ArgumentParser(
        description="Convert Game of Life plaintext pattern to a PNG image."
    )
    parser.add_argument("input", help="Path to input .cells file")
    parser.add_argument("output", help="Path to output PNG file")
    parser.add_argument("--cell-size", type=int, default=10,
                        help="Size of each cell in pixels (default: 10)")
    parser.add_argument("--dead-color", default=(0, 0, 0),
                        help="Color for dead cells (default: white)")
    parser.add_argument("--alive-color", default=(255, 255, 255),
                        help="Color for alive cells (default: black)")

    args = parser.parse_args()

    grid = read_plaintext(args.input)
    img = grid_to_image(grid, args.cell_size, args.dead_color, args.alive_color)
    img.save(args.output)
    print(f"Saved image to {args.output}")

if __name__ == "__main__":
    main()
