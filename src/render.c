#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "image.h"
#include "terminal.h"
#include "render.h"

int enable_dithering = 0;
int render_mode = 0; // 0 = auto, 1 = half-blocks (color), 2 = braille (detail)
extern int silent_mode;

// Braille dot positions (2x4 grid):
// 1 4
// 2 5
// 3 6
// 7 8
static const int braille_map[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

typedef struct {
    int r, g, b;
} Color;

static int rgb_to_gray(int r, int g, int b) {
    return (int)(0.299 * r + 0.587 * g + 0.114 * b);
}

// Calculate color variance to determine if image is colorful
static double calculate_color_variance(const Image *img) {
    long long r_sum = 0, g_sum = 0, b_sum = 0;
    int pixel_count = img->width * img->height;
    
    // Calculate averages
    for (int i = 0; i < pixel_count * 3; i += 3) {
        r_sum += img->data[i];
        g_sum += img->data[i + 1];
        b_sum += img->data[i + 2];
    }
    
    double r_avg = (double)r_sum / pixel_count;
    double g_avg = (double)g_sum / pixel_count;
    double b_avg = (double)b_sum / pixel_count;
    
    // Calculate variance between R, G, B channels
    double rg_diff = fabs(r_avg - g_avg);
    double rb_diff = fabs(r_avg - b_avg);
    double gb_diff = fabs(g_avg - b_avg);
    
    return (rg_diff + rb_diff + gb_diff) / 3.0;
}

// High-quality half-block renderer (better for color images)
static void render_half_blocks(const Image *img, int max_width, int max_height) {
    if (!silent_mode) {
        printf("Using half-block mode (optimized for color)\n");
    }
    
    // Get terminal size
    int term_rows, term_cols;
    get_terminal_size(&term_rows, &term_cols);
    
    if (max_width > term_cols) max_width = term_cols;
    if (max_height > term_rows * 2) max_height = term_rows * 2;
    
    // Calculate aspect-preserving dimensions
    double img_aspect = (double)img->width / img->height;
    double terminal_aspect = (double)max_width / (max_height * 0.5);
    
    int out_cols, out_rows;
    
    if (img_aspect > terminal_aspect) {
        out_cols = max_width;
        out_rows = (int)(max_width / img_aspect / 2.0);
        if (out_rows > max_height / 2) out_rows = max_height / 2;
    } else {
        out_rows = max_height / 2;
        out_cols = (int)(out_rows * img_aspect * 2.0);
        if (out_cols > max_width) out_cols = max_width;
    }
    
    if (out_cols < 1) out_cols = 1;
    if (out_rows < 1) out_rows = 1;
    
    double scale_x = (double)img->width / out_cols;
    double scale_y = (double)img->height / (out_rows * 2);
    
    if (!silent_mode) {
        printf("Half-blocks: %d×%d chars (%d×%d pixels) from %d×%d\n", 
               out_cols, out_rows * 2, out_cols, out_rows * 2, img->width, img->height);
    }
    
    for (int y = 0; y < out_rows; ++y) {
        for (int x = 0; x < out_cols; ++x) {
            // Sample top and bottom pixels
            int src_x = (int)(x * scale_x);
            int src_y_top = (int)(y * 2 * scale_y);
            int src_y_bot = (int)((y * 2 + 1) * scale_y);
            
            // Clamp to boundaries
            if (src_x >= img->width) src_x = img->width - 1;
            if (src_y_top >= img->height) src_y_top = img->height - 1;
            if (src_y_bot >= img->height) src_y_bot = img->height - 1;
            
            int top_r = 0, top_g = 0, top_b = 0;
            int bot_r = 0, bot_g = 0, bot_b = 0;
            
            // Sample top pixel
            if (src_x >= 0 && src_y_top >= 0) {
                int idx = (src_y_top * img->width + src_x) * 3;
                top_r = img->data[idx + 0];
                top_g = img->data[idx + 1];
                top_b = img->data[idx + 2];
            }
            
            // Sample bottom pixel
            if (src_x >= 0 && src_y_bot >= 0) {
                int idx = (src_y_bot * img->width + src_x) * 3;
                bot_r = img->data[idx + 0];
                bot_g = img->data[idx + 1];
                bot_b = img->data[idx + 2];
            }
            
            // Output with both foreground and background colors
            printf("\x1b[38;2;%d;%d;%dm\x1b[48;2;%d;%d;%dm▀", 
                   top_r, top_g, top_b, bot_r, bot_g, bot_b);
        }
        printf("\x1b[0m\n");
        fflush(stdout);
    }
}

// High-detail braille renderer (better for line art and B&W)
static void render_braille(const Image *img, int max_width, int max_height) {
    if (!silent_mode) {
        printf("Using braille mode (optimized for detail)\n");
    }
    
    // Get terminal size
    int term_rows, term_cols;
    get_terminal_size(&term_rows, &term_cols);
    
    if (max_width > term_cols) max_width = term_cols;
    if (max_height > term_rows * 4) max_height = term_rows * 4;
    
    // Calculate dimensions for 2x4 braille
    double img_aspect = (double)img->width / img->height;
    double char_aspect = 2.0 / 4.0;
    double terminal_aspect = (double)max_width / max_height * char_aspect;
    
    int out_cols, out_rows;
    
    if (img_aspect > terminal_aspect) {
        out_cols = max_width / 2;
        out_rows = (int)((out_cols * 2.0) / img_aspect / 4.0);
        if (out_rows > max_height / 4) out_rows = max_height / 4;
    } else {
        out_rows = max_height / 4;
        out_cols = (int)(out_rows * 4.0 * img_aspect / 2.0);
        if (out_cols > max_width / 2) out_cols = max_width / 2;
    }
    
    if (out_cols < 1) out_cols = 1;
    if (out_rows < 1) out_rows = 1;
    
    int render_width = out_cols * 2;
    int render_height = out_rows * 4;
    
    double scale_x = (double)img->width / render_width;
    double scale_y = (double)img->height / render_height;
    
    if (!silent_mode) {
        printf("Braille: %d×%d chars (%d×%d pixels) from %d×%d\n", 
               out_cols, out_rows, render_width, render_height, img->width, img->height);
    }
    
    // Create grayscale version for thresholding
    int *gray_image = malloc(render_width * render_height * sizeof(int));
    Color *color_image = malloc(render_width * render_height * sizeof(Color));
    
    if (!gray_image || !color_image) {
        printf("Error: Memory allocation failed\n");
        free(gray_image);
        free(color_image);
        return;
    }
    
    // Downsample
    for (int y = 0; y < render_height; y++) {
        for (int x = 0; x < render_width; x++) {
            int src_x = (int)(x * scale_x);
            int src_y = (int)(y * scale_y);
            
            if (src_x >= img->width) src_x = img->width - 1;
            if (src_y >= img->height) src_y = img->height - 1;
            
            if (src_x >= 0 && src_y >= 0) {
                int idx = (src_y * img->width + src_x) * 3;
                int r = img->data[idx + 0];
                int g = img->data[idx + 1];
                int b = img->data[idx + 2];
                
                gray_image[y * render_width + x] = rgb_to_gray(r, g, b);
                color_image[y * render_width + x] = (Color){r, g, b};
            } else {
                gray_image[y * render_width + x] = 0;
                color_image[y * render_width + x] = (Color){0, 0, 0};
            }
        }
    }
    
    // Calculate threshold
    long long sum = 0;
    for (int i = 0; i < render_width * render_height; i++) {
        sum += gray_image[i];
    }
    int threshold = (int)(sum / (render_width * render_height));
    
    // Render braille
    for (int char_y = 0; char_y < out_rows; char_y++) {
        for (int char_x = 0; char_x < out_cols; char_x++) {
            int braille_code = 0x2800;
            int total_r = 0, total_g = 0, total_b = 0, on_count = 0;
            
            // Sample 2x4 grid
            for (int dy = 0; dy < 4; dy++) {
                for (int dx = 0; dx < 2; dx++) {
                    int px = char_x * 2 + dx;
                    int py = char_y * 4 + dy;
                    
                    if (px < render_width && py < render_height) {
                        int gray = gray_image[py * render_width + px];
                        if (gray > threshold) {
                            braille_code |= braille_map[dy * 2 + dx];
                            Color c = color_image[py * render_width + px];
                            total_r += c.r; total_g += c.g; total_b += c.b;
                            on_count++;
                        }
                    }
                }
            }
            
            // Average color
            if (on_count > 0) {
                printf("\x1b[38;2;%d;%d;%dm", total_r/on_count, total_g/on_count, total_b/on_count);
            }
            
            // Output braille
            printf("%c%c%c", 
                   (char)(0xE0 | (braille_code >> 12)),
                   (char)(0x80 | ((braille_code >> 6) & 0x3F)),
                   (char)(0x80 | (braille_code & 0x3F)));
        }
        printf("\x1b[0m\n");
        fflush(stdout);
    }
    
    free(gray_image);
    free(color_image);
}

void render_image(const Image *img, int max_width, int max_height) {
    int selected_mode = render_mode;
    
    // Auto-detect best mode if not specified
    if (selected_mode == 0) {
        double color_variance = calculate_color_variance(img);
        
        if (!silent_mode) {
            printf("Color variance: %.1f ", color_variance);
        }
        
        if (color_variance > 15.0) {
            selected_mode = 1; // Half-blocks for colorful images
            if (!silent_mode) printf("(colorful - using half-blocks)\n");
        } else {
            selected_mode = 2; // Braille for B&W/line art
            if (!silent_mode) printf("(monochrome - using braille)\n");
        }
    }
    
    if (selected_mode == 1) {
        render_half_blocks(img, max_width, max_height);
    } else {
        render_braille(img, max_width, max_height);
    }
    
    printf("\x1b[0m");
    fflush(stdout);
}