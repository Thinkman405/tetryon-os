#ifndef TETRYON_IO_H
#define TETRYON_IO_H

#include "tetryon_math.h"

// Device Node IDs
#define NODE_SCREEN 14
#define NODE_KEYBOARD 15

// Function Prototypes
void device_io(int node_idx, TetryonNode input);

#endif
