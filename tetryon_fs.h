#ifndef TETRYON_FS_H
#define TETRYON_FS_H

#include <stdint.h>
#include "tetryon_math.h" // For TetryonNode

// The Crystal Header: Metadata for a crystallized file
typedef struct {
    uint64_t file_id;
    uint64_t start_node_sig; // Signature/Pointer to the first CrystalNode
    uint64_t size;           // Number of nodes in the crystal
} CrystalHeader;

// The Crystal Node: A single facet of the file geometry
typedef struct CrystalNode {
    TetryonNode payload;        // The actual data
    struct CrystalNode* next_facet; // Pointer to the next node in the chain
} CrystalNode;

// Function Prototypes
void fs_init();
uint64_t crystallize(uint64_t file_id, TetryonNode* data, int count);
int sublimate(uint64_t file_id, TetryonNode* buffer, int max_count);

#endif // TETRYON_FS_H
