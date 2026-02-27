#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tetryon_fb.h"

// Global Framebuffer State
static Framebuffer fb = {0};
static int fb_active = 0;

#ifdef __linux__
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

void fb_init() {
    int fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
        printf("[GeoFB] Warning: Cannot open /dev/fb0. Running in Headless Mode.\n");
        fb_active = 0;
        return;
    }

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        printf("[GeoFB] Error reading fixed information.\n");
        close(fbfd);
        return;
    }

    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        printf("[GeoFB] Error reading variable information.\n");
        close(fbfd);
        return;
    }

    fb.width = vinfo.xres;
    fb.height = vinfo.yres;
    fb.bpp = vinfo.bits_per_pixel;
    fb.xoffset = vinfo.xoffset;
    fb.yoffset = vinfo.yoffset;
    fb.screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
    fb.pitch = finfo.line_length;
    fb.fd = fbfd;

    // Map the device to memory
    fb.buffer = (uint32_t*)mmap(0, fb.screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if ((long)fb.buffer == -1) {
        printf("[GeoFB] Error: Failed to map framebuffer device to memory.\n");
        close(fbfd);
        return;
    }

    fb_active = 1;
    printf("[GeoFB] Initialized: %dx%d, %dbpp\n", fb.width, fb.height, fb.bpp);
    
    // Clear Screen
    memset(fb.buffer, 0, fb.screensize);
}

void fb_cleanup() {
    if (fb_active) {
        munmap(fb.buffer, fb.screensize);
        close(fb.fd);
    }
}

// Helper: Draw Pixel
void draw_pixel(int x, int y, uint32_t color) {
    if (!fb_active || x < 0 || x >= fb.width || y < 0 || y >= fb.height) return;
    
    // Calculate offset
    // Calculate offset
    long location = (x + fb.xoffset) * (fb.bpp / 8) + (y + fb.yoffset) * fb.pitch;
    // Wait, vinfo is local to init. We need to store offsets if we want to use them.
    // For simplicity, assume offset is 0 or just use linear index for 32bpp
    // location = x * 4 + y * fb.pitch; 
    
    // Pointer arithmetic on uint32_t* moves by 4 bytes. 
    // pitch is in bytes. So y * (pitch / 4)
    int index = x + y * (fb.pitch / 4);
    fb.buffer[index] = color;
}

// Helper: Draw Rect
void draw_rect(int x, int y, int w, int h, uint32_t color) {
    for (int i = 0; i < w; i++) {
        draw_pixel(x + i, y, color);
        draw_pixel(x + i, y + h - 1, color);
    }
    for (int j = 0; j < h; j++) {
        draw_pixel(x, y + j, color);
        draw_pixel(x + w - 1, y + j, color);
    }
}

// Helper: Draw Filled Rect
void draw_rect_filled(int x, int y, int w, int h, uint32_t color) {
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            draw_pixel(x + i, y + j, color);
        }
    }
}

#else
// Windows / Non-Linux Fallback
// Using ANSI Escape Codes for Headless Visualization

int ansi_active = 0;

void fb_init() {
    printf("[GeoFB] System is not Linux. Activating ANSI Headless Mode...\n");
    ansi_active = 1;
    
    // Clear Screen, Hide Cursor
    printf("\033[2J"); // Clear
    printf("\033[?25l"); // Hide Cursor
    
    // Draw Border
    printf("\033[1;1H"); // Home
    printf("+--------------------------------+");
    printf("\033[2;2H[ Tetryon OS - ANSI Mode ]");
}

void fb_cleanup() {
    if (ansi_active) {
        printf("\033[?25h"); // Show Cursor
        printf("\033[0m");   // Reset attributes
    }
}

void draw_pixel(int x, int y, uint32_t color) {
    if (ansi_active && color != 0) {
        // Scale: x/10, y/20
        int term_x = (x / 10) + 1; 
        int term_y = (y / 20) + 3; // Offset for header
        printf("\033[%d;%dH#", term_y, term_x);
    }
}

void draw_rect(int x, int y, int w, int h, uint32_t color) {
    if (ansi_active) {
        int term_x = (x / 10) + 1;
        int term_y = (y / 20) + 3;
        printf("\033[%d;%dH+", term_y, term_x);
    }
}

void draw_rect_filled(int x, int y, int w, int h, uint32_t color) {
     if (ansi_active) {
        int term_x = (x / 10) + 1;
        int term_y = (y / 20) + 3;
        printf("\033[%d;%dHO", term_y, term_x);
     }
}
#endif

// Shared Rendering Logic
void render_hud() {
    if (!fb_active) return;

    // Draw Borders
    // Left Panel (Hex Dump): 25% width
    // Center (Radar): 50% width
    // Right Panel (Stats): 25% width
    
    int w = fb.width;
    int h = fb.height;
    
    int left_w = w / 4;
    int right_x = w - (w / 4);
    
    // Frame
    draw_rect(0, 0, w, h, COLOR_LATTICE);
    
    // Dividers
    for (int y = 0; y < h; y++) {
        draw_pixel(left_w, y, COLOR_LATTICE);
        draw_pixel(right_x, y, COLOR_LATTICE);
    }
}

void render_lattice(TetryonNode* registers, int reg_count) {
    if (fb_active) {
        // ... (Existing FB logic) ...
        // 1. Clear Radar Area (Center)
        // Optimization: Don't clear everything, just the radar box
        int w = fb.width;
        int h = fb.height;
        int left_w = w / 4;
        int center_w = w / 2;
        int radar_x = left_w + 1;
        int radar_y = 1;
        int radar_w = center_w - 2;
        int radar_h = h - 2;
        
        render_hud();

        // 2. Plot Nodes
        int cx = w / 2;
        int cy = h / 2;
        double scale = 20.0; // Pixels per unit
        
        for (int i = 0; i < reg_count; i++) {
            double rx = registers[i].real_coords[0];
            double ry = registers[i].real_coords[1];
            
            // Project to Screen
            int sx = cx + (int)(rx * scale);
            int sy = cy - (int)(ry * scale); // Y flip
            
            // Draw Node (3x3 dot)
            uint32_t color = (registers[i].r > 0.001) ? COLOR_NODE_ACTIVE : COLOR_NODE_IDLE;
            if (i == 0) color = 0xFFFFFFFF; // Origin is White
            
            draw_rect_filled(sx - 2, sy - 2, 5, 5, color);
        }
    } 
    #ifndef __linux__
    else if (ansi_active) {
        // ANSI Rendering
        // Draw Nodes
        // We assume 80x24 terminal roughly
        
        for (int i = 0; i < reg_count; i++) {
            if (registers[i].r > 0.001 || i == 0) {
                int sx = (int)(registers[i].real_coords[0] * 2) + 40; // Center X=40
                int sy = 12 - (int)(registers[i].real_coords[1]);     // Center Y=12
                
                if (sx >= 1 && sx < 80 && sy >= 3 && sy < 24) {
                    char symbol = (i == 0) ? '+' : 'o';
                    printf("\033[%d;%dH%c", sy, sx, symbol);
                }
            }
        }
        
        // Draw HUD Text
        printf("\033[23;2HActive Nodes: %d", reg_count);
        fflush(stdout);
    }
    #endif
}

void fb_panic(char* message) {
    if (!fb_active) return;
    
    // Fill screen with Red
    int size = fb.width * fb.height;
    for (int i = 0; i < size; i++) {
        fb.buffer[i] = COLOR_CRITICAL;
    }
}
