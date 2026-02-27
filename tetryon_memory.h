#ifndef TETRYON_MEMORY_H
#define TETRYON_MEMORY_H

#include "tetryon_math.h"

// Spiral Sector Structure (Linked List Node)
typedef struct SpiralSector {
    TetryonNode data;
    double theta;
    double radius;
    struct SpiralSector* next;
} SpiralSector;

// Global Head of the Spiral Memory
extern SpiralSector* SpiralMemory;

// Function Declarations
void init_spiral_memory();
void spiral_alloc(TetryonNode n);
int spiral_retrieve(uint64_t harmonic_sig, TetryonNode* out_node);
void visualize_spiral();

#endif // TETRYON_MEMORY_H
