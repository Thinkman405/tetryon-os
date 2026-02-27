#include <stdio.h>
#include "tetryon_math.h"

// Fixed Device Nodes (Singularities)
#define NODE_SCREEN 14
#define NODE_KEYBOARD 15

// Device IO Handler
void device_io(int node_idx, TetryonNode input) {
    if (node_idx == NODE_SCREEN) {
        // Output to host stdout
        // We use the raw_value as the "character" or data point
        printf("[TERM] >> %.4f\n", input.real_coords[0]);
    } else if (node_idx == NODE_KEYBOARD) {
        // Mock input
        printf("[DEV] Keyboard input requested (Mocked).\n");
    } else {
        printf("[DEV] Unknown device node: %d\n", node_idx);
    }
}
