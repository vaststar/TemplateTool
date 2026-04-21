# FFmpeg Pre-built Binaries

FFmpeg command-line tools for audio/video processing, screen capture, and GIF generation.

## Version

- **Default version**: 8.1
- **Sources**:
  - Windows / Linux: [BtbN/FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds) (daily auto-builds)
  - macOS: [Homebrew](https://formulae.brew.sh/formula/ffmpeg) (`brew install ffmpeg`)

### Triggering a Build

On the GitHub repository page, go to **Actions** → **Download FFmpeg All Platforms** → **Run workflow**, where you can specify:
- `ffmpeg_version`: Version to download (e.g., 8.1, 7.1)
- `platforms`: Platforms to download (windows, macos, linux)

## Platforms

| Platform | Architecture | Path | Source |
|----------|--------------|------|--------|
| Windows | x64 | `windows/x64/` | BtbN |
| macOS | Intel (x86_64) | `macosx/intel/` | Homebrew |
| macOS | ARM64 (Apple Silicon) | `macosx/arm/` | Homebrew (native arm64) |
| Linux | x64 | `linux/x64/` | BtbN |

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
