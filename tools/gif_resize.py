#!/usr/bin/env python3
##################################################################################
#                                                                                #
## Resize (crop or letterbox) a GIF to fit the Lilka display.                   ##
## Default target is Lilka v2 in landscape: 280x240.                            ##
#                                                                                #
## Requires Pillow: pip install Pillow                                          ##
#                                                                                #
## Usage:                                                                       ##
##   tools/gif_resize.py input.gif                     -> input_280x240.gif     ##
##   tools/gif_resize.py input.gif -o out.gif --mode fit                        ##
##   tools/gif_resize.py input.gif --size 216x216                               ##
#                                                                                #
##################################################################################
import argparse
import sys
from pathlib import Path

try:
    from PIL import Image, ImageSequence
except ImportError:
    print("Error: Pillow is required, install it with: pip install Pillow")
    sys.exit(1)

LILKA_WIDTH = 280
LILKA_HEIGHT = 240

# AnimatedGIF on ESP32 limits canvas width to 320 px
GIF_MAX_WIDTH = 320


def parse_size(value):
    try:
        w, h = value.lower().split("x")
        return int(w), int(h)
    except ValueError:
        raise argparse.ArgumentTypeError(f"invalid size '{value}', expected WxH, e.g. 280x240")


def cover(frame, width, height):
    # Scale to fill the target and crop the overflow around the center
    scale = max(width / frame.width, height / frame.height)
    scaled = frame.resize((max(width, round(frame.width * scale)), max(height, round(frame.height * scale))), Image.LANCZOS)
    left = (scaled.width - width) // 2
    top = (scaled.height - height) // 2
    return scaled.crop((left, top, left + width, top + height))


def fit(frame, width, height, background):
    # Scale to fit inside the target and pad the rest with background color
    scale = min(width / frame.width, height / frame.height)
    scaled = frame.resize((max(1, round(frame.width * scale)), max(1, round(frame.height * scale))), Image.LANCZOS)
    canvas = Image.new("RGBA", (width, height), background)
    canvas.paste(scaled, ((width - scaled.width) // 2, (height - scaled.height) // 2), scaled)
    return canvas


def crop(frame, width, height, background):
    # No scaling, just cut the center (or pad if the source is smaller)
    canvas = Image.new("RGBA", (width, height), background)
    canvas.paste(frame, ((width - frame.width) // 2, (height - frame.height) // 2), frame)
    return canvas


def main():
    parser = argparse.ArgumentParser(description="Resize a GIF to the Lilka display size")
    parser.add_argument("input", type=Path, help="source GIF")
    parser.add_argument("-o", "--output", type=Path, help="output GIF (default: <input>_<W>x<H>.gif)")
    parser.add_argument(
        "-s",
        "--size",
        type=parse_size,
        default=(LILKA_WIDTH, LILKA_HEIGHT),
        help=f"target size WxH (default: {LILKA_WIDTH}x{LILKA_HEIGHT})",
    )
    parser.add_argument(
        "-m",
        "--mode",
        choices=["cover", "fit", "crop"],
        default="cover",
        help="cover: scale and crop to fill (default), fit: scale and letterbox, crop: cut center without scaling",
    )
    parser.add_argument("-b", "--background", default="black", help="letterbox color for fit/crop (default: black)")
    parser.add_argument("--colors", type=int, default=256, help="palette size per frame, 2..256 (default: 256)")
    parser.add_argument("--skip", type=int, default=1, help="keep every N-th frame to reduce file size (default: 1)")
    args = parser.parse_args()

    width, height = args.size
    if width > GIF_MAX_WIDTH:
        print(f"Warning: width {width} exceeds AnimatedGIF limit of {GIF_MAX_WIDTH} px, Lilka won't open it")
    if not 2 <= args.colors <= 256:
        parser.error("--colors must be in range 2..256")
    if args.skip < 1:
        parser.error("--skip must be >= 1")

    output = args.output or args.input.with_name(f"{args.input.stem}_{width}x{height}.gif")

    with Image.open(args.input) as src:
        loop = src.info.get("loop", 0)
        frames = []
        durations = []
        pending_duration = 0
        for index, frame in enumerate(ImageSequence.Iterator(src)):
            # Pillow composites GIF frames with disposal applied, so each frame is a full image
            pending_duration += frame.info.get("duration", 100)
            if index % args.skip != 0:
                continue
            rgba = frame.convert("RGBA")
            if args.mode == "cover":
                out = cover(rgba, width, height)
            elif args.mode == "fit":
                out = fit(rgba, width, height, args.background)
            else:
                out = crop(rgba, width, height, args.background)
            frames.append(out.convert("RGB").quantize(colors=args.colors, method=Image.MEDIANCUT, dither=Image.FLOYDSTEINBERG))
            durations.append(pending_duration)
            pending_duration = 0

        if not frames:
            print("Error: no frames found")
            sys.exit(1)
        # Frames dropped after the last kept one still contribute to its duration
        durations[-1] += pending_duration

    frames[0].save(
        output,
        save_all=True,
        append_images=frames[1:],
        duration=durations,
        loop=loop,
        optimize=True,
        disposal=1,
    )
    print(f"{args.input} ({src.width}x{src.height}) -> {output} ({width}x{height}, {len(frames)} frames, {output.stat().st_size} bytes)")


if __name__ == "__main__":
    main()
