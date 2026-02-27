#ifndef TETRYON_FB_H
#define TETRYON_FB_H

#include <stdint.h>
#include "tetryon_math.h"

// Framebuffer Structure
typedef struct {
    uint32_t* buffer;
    int width;
    int height;
    int pitch;     // Bytes per line
    int bpp;       // Bits per pixel
    int xoffset;   // [NEW] X Offset
    int yoffset;   // [NEW] Y Offset
    long screensize;
    int fd;        // File descriptor
} Framebuffer;

// Colors (ARGB 8888)
#define COLOR_VOID        0x00000000 // Black
#define COLOR_LATTICE     0x0000FF00 // Green
#define COLOR_NODE_ACTIVE 0x00FF00FF // Magenta
#define COLOR_NODE_IDLE   0x000000FF // Blue
#define COLOR_CRITICAL    0x00FF0000 // Red
#define COLOR_TEXT        0x00FFFFFF // White

// Function Prototypes
void fb_init();
void fb_cleanup();
void render_lattice(TetryonNode* registers, int reg_count);
void fb_panic(char* message);
void draw_pixel(int x, int y, uint32_t color);
void draw_rect(int x, int y, int w, int h, uint32_t color);
void draw_rect_filled(int x, int y, int w, int h, uint32_t color);

#endif // TETRYON_FB_H
