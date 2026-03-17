# FFmpeg Pre-built Binaries

FFmpeg command-line tools for audio/video processing, screen capture, and GIF generation.

## Version

- **Version**: 6.1
- **Source**: [ffbinaries.com](https://ffbinaries.com)

## Platforms

| Platform | Architecture | Path |
|----------|--------------|------|
| macOS | Intel (x86_64) | `macosx/intel/` |
| macOS | ARM64 (Apple Silicon) | `macosx/arm/` (Intel via Rosetta) |
| Windows | x64 | `windows/x64/` |
| Linux | x64 | `linux/x64/` |

> **Note**: The macOS ARM directory currently contains Intel binaries that run via Rosetta 2.
> Native ARM binaries will be added when officially available.

## Included Executables

- `ffmpeg` - Audio/video encoder, decoder, transcoder
- `ffprobe` - Media file analyzer

## Usage Examples

### Screen Recording (macOS)
```bash
ffmpeg -f avfoundation -i "1:" -r 30 -t 10 output.mp4
```

### Screen Recording (Windows)
```bash
ffmpeg -f gdigrab -framerate 30 -i desktop -t 10 output.mp4
```

### GIF Generation
```bash
# High quality GIF with palette
ffmpeg -i input.mp4 -vf "fps=15,scale=640:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse" output.gif
```

### Region Capture
```bash
ffmpeg -f avfoundation -i "1:" -vf "crop=800:600:100:100" output.mp4
```

## License

FFmpeg is licensed under LGPL/GPL. See https://ffmpeg.org/legal.html
