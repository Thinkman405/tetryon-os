#ifndef TETRYON_GPU_H
#define TETRYON_GPU_H

#include "tetryon_math.h"
#include "tetryon_memory.h"

// FluxLine: Represents a connection (BIND) between two nodes
typedef struct {
    int source_id;
    int target_id;
    double strength;
} FluxLine;

// SceneFrame: A snapshot of the entire register set and active memory
// Note: In a real GPU this would be a VRAM buffer. Here it's a logical grouping.
typedef struct {
    TetryonNode* registers; // Pointer to the VM's register array
    SpiralSector* memory;   // Pointer to the VM's spiral memory
    int register_count;
} SceneFrame;

// Function Prototypes
void gpu_snapshot(char* filename, TetryonNode* registers, int reg_count, SpiralSector* memory);

#endif // TETRYON_GPU_H
