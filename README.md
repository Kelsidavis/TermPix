# TermPix

**TermPix** is a high-resolution terminal image renderer built in C. It transforms images into rich, colorful terminal graphics using half-block and braille rendering modes with automatic optimization based on image content.

---

## Features

* ✅ Smart mode detection: auto-selects best rendering (photo vs. diagram)
* 🎨 Half-block color rendering (2× vertical resolution)
* 🔍 Braille detail rendering (4× resolution for line work)
* 🖼️ Dithering support with Floyd–Steinberg algorithm
* 🧱 Terminal-aware sizing with aspect ratio preservation
* 💻 Cross-platform: Fully compatible with Windows (PowerShell/cmd) and Unix terminals

---

## Screenshot (Windows Terminal Example)

> Running `termpix.exe windows.png` on PowerShell:

![TermPix rendering screenshot](screenshot.jpg)

---

## Usage

### Basic

```powershell
termpix image.jpg
```

TermPix auto-detects terminal size and rendering mode.

### Command-Line Options

| Option         | Description                                                       |
| -------------- | ----------------------------------------------------------------- |
| `--width N`    | Set maximum output width in characters                            |
| `--height N`   | Set maximum output height in characters                           |
| `--mode MODE`  | Set rendering mode: `auto`, `color`, or `detail`                  |
| `--dither`     | Enable dithering for smoother gradients                           |
| `--fit`        | Force exact dimension scaling (disable aspect ratio preservation) |
| `--version`    | Show version and feature information                              |
| `--help`, `-h` | Show usage instructions                                           |

---

## PowerShell Module

TermPix can also be used as an installable PowerShell module:

### Module Layout

```
TermPix/
├── TermPix.psm1
├── TermPix.psd1
├── termpix.exe
└── screenshot.jpg
```

### Example PowerShell Function

```powershell
Show-TermPixImage -Path "image.jpg" -Width 100 -Mode color -Dither
```

### Installation

```powershell
# Manual installation
$dest = "$HOME\Documents\PowerShell\Modules\TermPix"
New-Item -ItemType Directory -Path $dest -Force | Out-Null
Copy-Item -Recurse -Path .\* -Destination $dest
Import-Module TermPix
```

---

## Build Instructions

### Windows (PowerShell + TDM-GCC / MinGW-w64)

```powershell
./build_windows.ps1
```

Requires `stb_image.h` in the `lib/` directory.

### Linux / macOS

```bash
gcc -o termpix main.c render.c terminal.c image.c -lm
```

---

## Examples

```powershell
termpix photo.jpg
termpix --mode color --dither portrait.png
termpix --mode detail flowchart.tga
termpix --width 100 --height 40 --fit diagram.bmp
```

---

## Author

**Kelsi Davis**
🌐 [https://geekastro.dev](https://geekastro.dev)

TermPix is part of a suite of tools focused on accessible software for data visualization and astrophotography workflows.

---

## License

MIT License
