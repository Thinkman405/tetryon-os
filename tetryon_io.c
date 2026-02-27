#include <stdio.h>
#include "tetryon_io.h"

void device_io(int node_idx, TetryonNode input) {
    if (node_idx == NODE_SCREEN) {
        // Output to standard out for now, representing the screen
        // In a real implementation, this might write to a framebuffer or serial port
        printf("[SCREEN] %.4f\n", input.real_coords[0]);
    } else if (node_idx == NODE_KEYBOARD) {
        // Input handling would go here
        // For now, we just acknowledge the request
        printf("[KEYBOARD] Input requested (Not implemented)\n");
    } else {
        printf("[IO] Unknown Device Node: %d\n", node_idx);
    }
}
