# SDF font atlas generation tool

Atlas generation tool for the [font rendering demo](https://github.com/astiopin/webgl_fonts).

The algorithm described here.

Mostly for educational purposes since the algorithm is performant enough for generating font atlases at runtime.

# Dependencies

GLFW
    
# Usage

```sdf_atlas -f font_file.ttf [options]
Options:
    -h              this help
    -o 'filename'   output file name (without extension)
    -tw 'size'      atlas image width in pixels, default 1024
    -th 'size'      atlas image height in pixels (optional)
    -ur 'ranges'    unicode ranges 'start1:end1,start:end2,single_codepoint' without spaces,
                    default: 31:126,0xffff
    -bs 'size'      SDF distance in pixels, default 16
    -rh 'size'      row height in pixels (without SDF border), default 96
Example:
    sdf_atlas -f Roboto-Regular.ttf -o roboto -tw 2048 -th 2048 -bs 22 -rh 70 -ur 31:126,0xA0:0xFF,0x400:0x4FF,0xFFFF```

    