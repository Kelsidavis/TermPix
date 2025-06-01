// main.c - TermPix: High-resolution terminal image viewer
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <locale.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "image.h"
#include "render.h"
#include "terminal.h"
#include "../lib/stb_image.h"

extern int enable_dithering;
extern int render_mode; // 0 = auto, 1 = half-blocks, 2 = braille

void setup_console_utf8() {
#ifdef _WIN32
    // Set console code pages to UTF-8
    SetConsoleOutputCP(CP_UTF8);
    
    // Enable ANSI escape sequence processing
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif
    
    // Set locale for UTF-8
    setlocale(LC_ALL, "C.UTF-8");
}

void print_banner() {
    printf("\x1b[1;36m"); // Bold cyan
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║  ████████╗███████╗██████╗ ███╗   ███╗██████╗ ██╗██╗  ██╗     ║\n");
    printf("║  ╚══██╔══╝██╔════╝██╔══██╗████╗ ████║██╔══██╗██║╚██╗██╔╝     ║\n");
    printf("║     ██║   █████╗  ██████╔╝██╔████╔██║██████╔╝██║ ╚███╔╝      ║\n");
    printf("║     ██║   ██╔══╝  ██╔══██╗██║╚██╔╝██║██╔═══╝ ██║ ██╔██╗      ║\n");
    printf("║     ██║   ███████╗██║  ██║██║ ╚═╝ ██║██║     ██║██╔╝ ██╗     ║\n");
    printf("║     ╚═╝   ╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝     ║\n");
    printf("║                                                              ║\n");
    printf("║           High-Resolution Terminal Image Viewer              ║\n");
    printf("║                 v2.0 • Smart Mode Selection                  ║\n");
    printf("║   Developed by Kelsi Davis  •  https://geekastro.dev         ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\x1b[0m\n\n");
}


void print_usage(const char *program_name) {
    print_banner();
    printf("Transform images into stunning terminal art with smart rendering technology.\n");
    printf("Automatically detects the best mode for your image - color or high-detail.\n\n");
    
    printf("\x1b[1;33m⚡ Quick Start:\x1b[0m\n");
    printf("   %s image.jpg                    \x1b[90m# Let TermPix choose the best mode\x1b[0m\n\n", program_name);
    
    printf("\x1b[1mUsage:\x1b[0m %s [OPTIONS] <image_file>\n\n", program_name);
    
    printf("\x1b[1;32m🎛️  Options:\x1b[0m\n");
    printf("   \x1b[36m--width N\x1b[0m      Set maximum width in characters (default: terminal width)\n");
    printf("   \x1b[36m--height N\x1b[0m     Set maximum height in characters (default: terminal height)\n");
    printf("   \x1b[36m--mode MODE\x1b[0m    Rendering mode: auto, color, detail (default: auto)\n");
    printf("   \x1b[36m--dither\x1b[0m       Enable Floyd-Steinberg dithering for smoother gradients\n");
    printf("   \x1b[36m--fit\x1b[0m          Force image to fit exactly in specified dimensions\n");
    printf("   \x1b[36m-h, --help\x1b[0m     Show this help message\n");
    printf("   \x1b[36m--version\x1b[0m      Show detailed version information\n\n");
    
    printf("\x1b[1;34m📁 Supported Formats:\x1b[0m\n");
    printf("   JPEG • PNG • BMP • TGA • GIF • PSD • HDR • PIC\n\n");
    
    printf("\x1b[1;35m🎨 Rendering Modes:\x1b[0m\n");
    printf("   \x1b[33mauto\x1b[0m     🧠 Smart detection - analyzes image and picks best mode\n");
    printf("   \x1b[31mcolor\x1b[0m    🌈 Half-blocks with rich colors (perfect for photos)\n");
    printf("   \x1b[37mdetail\x1b[0m   🔍 Braille dots for sharp lines (ideal for diagrams)\n\n");
    
    printf("\x1b[1;36m📚 Examples:\x1b[0m\n");
    printf("   %s vacation.jpg\n", program_name);
    printf("   %s --width 80 --height 40 portrait.png\n", program_name);
    printf("   %s --mode color --dither sunset.jpg\n", program_name);
    printf("   %s --mode detail flowchart.png\n", program_name);
    printf("   %s --width 120 --mode auto screenshot.png\n\n", program_name);
    
    printf("\x1b[1;33m💡 Pro Tips:\x1b[0m\n");
    printf("   • Use \x1b[32m--dither\x1b[0m with photos for smoother color transitions\n");
    printf("   • Try \x1b[32m--mode detail\x1b[0m for text, diagrams, and line art\n");
    printf("   • Adjust terminal font size for optimal viewing experience\n");
    printf("   • Larger \x1b[32m--width\x1b[0m values show more detail, smaller ones give overview\n\n");
    
    printf("\x1b[90m═══════════════════════════════════════════════════════════════\x1b[0m\n");
    printf("\x1b[1;37mTermPix\x1b[0m automatically preserves aspect ratios and optimizes quality.\n");
    printf("No configuration needed - just point it at an image and enjoy! 🚀\n");
}

void print_compact_banner() {
    printf("\x1b[1;36m"); // Bold cyan
    printf("┌─────────────────────────────────────────┐\n");
    printf("│ \x1b[1;37m▀█▀ █▀▀ █▀█ █▀▄▀█ █▀█ █ ▀▄▀\x1b[1;36m │\n");
    printf("│ \x1b[1;37m █  █▄▄ █▀▄ █ ▀ █ █▀▀ █ █ █\x1b[1;36m │\n");
    printf("│         \x1b[1;33mv2.0\x1b[1;36m • \x1b[0;36mSmart Rendering\x1b[1;36m         │\n");
    printf("└─────────────────────────────────────────┘\x1b[0m\n\n");
}

void print_version() {
    print_compact_banner();
    printf("\x1b[1;32m🎯 Features:\x1b[0m\n");
    printf("   • \x1b[36mSmart mode detection\x1b[0m - Auto-selects best rendering\n");
    printf("   • \x1b[35mColor mode\x1b[0m - Rich colors with half-blocks (2× resolution)\n");
    printf("   • \x1b[37mDetail mode\x1b[0m - Sharp lines with braille dots (4× resolution)\n");
    printf("   • \x1b[33mDithering support\x1b[0m - Floyd-Steinberg for smooth gradients\n");
    printf("   • \x1b[34mUTF-8 rendering\x1b[0m - Full Unicode character support\n\n");
    
    printf("\x1b[1;34m📊 Build Info:\x1b[0m\n");
    printf("   Version: 2.0\n");
    printf("   Compiled: %s %s\n", __DATE__, __TIME__);
#ifdef _WIN32
    printf("   Platform: Windows (UTF-8 enabled)\n");
#else
    printf("   Platform: Unix/Linux\n");
#endif
    printf("   Modes: Auto, Color, Detail\n\n");
    
    printf("\x1b[1;35m💡 Quick Start:\x1b[0m\n");
    printf("   \x1b[32mtermpix image.jpg\x1b[0m              # Auto-detect best mode\n");
    printf("   \x1b[32mtermpix --mode color photo.png\x1b[0m   # Force color mode\n");
    printf("   \x1b[32mtermpix --mode detail diagram.svg\x1b[0m # Force detail mode\n\n");
    
    printf("Made with ❤️  for terminal enthusiasts\n");
}

int validate_dimensions(int width, int height) {
    if (width <= 0 || height <= 0) {
        printf("\x1b[31mError:\x1b[0m Dimensions must be positive integers.\n");
        return 0;
    }
    if (width > 500 || height > 200) {
        printf("\x1b[33mWarning:\x1b[0m Large dimensions may cause display issues.\n");
        printf("Recommended maximum: 500×200\n");
        printf("Continue anyway? (y/N): ");
        char response;
        if (scanf(" %c", &response) == 1 && (response == 'y' || response == 'Y')) {
            return 1;
        }
        return 0;
    }
    return 1;
}

const char* get_file_extension(const char* filename) {
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

// Case-insensitive string comparison (Windows compatibility)
int str_case_cmp(const char* a, const char* b) {
    while (*a && *b) {
        char ca = (*a >= 'A' && *a <= 'Z') ? (*a + 32) : *a;
        char cb = (*b >= 'A' && *b <= 'Z') ? (*b + 32) : *b;
        if (ca != cb) return ca - cb;
        a++; b++;
    }
    return *a - *b;
}

int is_supported_format(const char* filename) {
    const char* ext = get_file_extension(filename);
    const char* supported[] = {"jpg", "jpeg", "png", "bmp", "tga", "gif", "psd", "hdr", "pic", NULL};
    
    for (int i = 0; supported[i]; i++) {
        if (str_case_cmp(ext, supported[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    // Set up console and UTF-8 support
    setup_console_utf8();
    
    // Parse command line arguments
    const char *filename = NULL;
    int max_width = 0, max_height = 0;
    int force_fit = 0;
    int show_help = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--width") == 0 && i + 1 < argc) {
            max_width = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--height") == 0 && i + 1 < argc) {
            max_height = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            char *mode = argv[++i];
            if (strcmp(mode, "auto") == 0) {
                render_mode = 0;
            } else if (strcmp(mode, "color") == 0) {
                render_mode = 1;
            } else if (strcmp(mode, "detail") == 0) {
                render_mode = 2;
            } else {
                printf("\x1b[31mError:\x1b[0m Unknown mode '%s'. Use: auto, color, or detail\n", mode);
                return 1;
            }
        } else if (strcmp(argv[i], "--dither") == 0) {
            enable_dithering = 1;
        } else if (strcmp(argv[i], "--fit") == 0) {
            force_fit = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            show_help = 1;
        } else if (strcmp(argv[i], "--version") == 0) {
            print_version();
            return 0;
        } else if (argv[i][0] == '-') {
            printf("\x1b[31mError:\x1b[0m Unknown option '%s'\n", argv[i]);
            printf("Use --help for usage information.\n");
            return 1;
        } else {
            if (filename != NULL) {
                printf("\x1b[31mError:\x1b[0m Multiple image files specified.\n");
                printf("Please specify only one image file.\n");
                return 1;
            }
            filename = argv[i];
        }
    }

    if (show_help || !filename) {
        print_usage(argv[0]);
        return show_help ? 0 : 1;
    }

    // Validate dimensions if specified
    if ((max_width > 0 || max_height > 0) && !validate_dimensions(max_width, max_height)) {
        return 1;
    }

    // Check file format
    if (!is_supported_format(filename)) {
        printf("\x1b[33mWarning:\x1b[0m '%s' may not be a supported image format.\n", filename);
        printf("Supported: JPEG, PNG, BMP, TGA, GIF, PSD, HDR, PIC\n");
        printf("Continue anyway? (y/N): ");
        char response;
        if (scanf(" %c", &response) != 1 || (response != 'y' && response != 'Y')) {
            return 1;
        }
    }

    // Check if file exists and is readable
    FILE *test_file = fopen(filename, "rb");
    if (!test_file) {
        printf("\x1b[31mError:\x1b[0m Cannot open file '%s'\n", filename);
        printf("Please check the file path and permissions.\n");
        return 1;
    }
    fclose(test_file);

    printf("\x1b[1;34m⚡ Loading:\x1b[0m %s\n", filename);
    
    clock_t start = clock();

    // Load the image
    Image img;
    if (!load_image(filename, &img)) {
        printf("\x1b[31mError:\x1b[0m Failed to load image '%s'\n", filename);
        printf("The file may be corrupted or in an unsupported format.\n");
        return 1;
    }

    clock_t load_time = clock();
    double load_duration = ((double)(load_time - start)) / CLOCKS_PER_SEC;

    printf("\x1b[1;32m✓ Loaded:\x1b[0m %dx%d pixels, %d channels (%.2fs)\n", 
           img.width, img.height, img.channels, load_duration);

    // Get terminal size if not specified
    if (max_width == 0 || max_height == 0) {
        int term_rows, term_cols;
        get_terminal_size(&term_rows, &term_cols);
        
        if (max_width == 0) max_width = term_cols;
        if (max_height == 0) max_height = term_rows * 4; // 4 pixels per character height
        
        printf("\x1b[1;36m📐 Terminal:\x1b[0m %d×%d characters\n", term_cols, term_rows);
    }

    printf("\x1b[1;35m🎨 Target:\x1b[0m %d×%d pixels", max_width, max_height);
    if (enable_dithering) printf(" (dithered)");
    if (force_fit) printf(" (forced fit)");
    printf("\n");

    // Render the image
    printf("\n");
    clock_t render_start = clock();
    render_image(&img, max_width, max_height);
    clock_t render_time = clock();
    
    double render_duration = ((double)(render_time - render_start)) / CLOCKS_PER_SEC;
    double total_duration = ((double)(render_time - start)) / CLOCKS_PER_SEC;

    // Statistics
    printf("\n\x1b[90m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\x1b[0m\n");
    printf("\x1b[90mRender: %.2fs | Total: %.2fs | %s\x1b[0m\n", 
           render_duration, total_duration, filename);

    // Clean up
    stbi_image_free(img.data);
    
    return 0;
}