#include <stdio.h>
#include <stdlib.h>
#include "tetryon_gpu.h"

void gpu_snapshot(char* filename, TetryonNode* registers, int reg_count, SpiralSector* memory) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("[GeoGPU] Error: Could not open %s for writing.\n", filename);
        return;
    }

    fprintf(f, "{\n");
    fprintf(f, "  \"nodes\": [\n");

    // 1. Export Registers (N0-N15)
    for (int i = 0; i < reg_count; i++) {
        fprintf(f, "    {\"id\": %d, \"x\": %.4f, \"y\": %.4f, \"z\": %.4f, \"val\": %.4f, \"type\": \"register\"}", 
                i, 
                registers[i].real_coords[0], 
                registers[i].real_coords[1], 
                registers[i].real_coords[2], 
                registers[i].r); // Using r as 'val' or magnitude
        
        if (i < reg_count - 1 || memory != NULL) {
            fprintf(f, ",\n");
        } else {
            fprintf(f, "\n");
        }
    }

    // 2. Export Spiral Memory Nodes
    // We assign them IDs starting from reg_count (16+)
    int mem_id = reg_count;
    SpiralSector* current = memory;
    while (current != NULL) {
        fprintf(f, "    {\"id\": %d, \"x\": %.4f, \"y\": %.4f, \"z\": %.4f, \"val\": %.4f, \"type\": \"memory\"}", 
                mem_id, 
                current->data.real_coords[0], 
                current->data.real_coords[1], 
                current->data.real_coords[2], 
                current->data.r);
        
        current = current->next;
        mem_id++;
        
        if (current != NULL) {
            fprintf(f, ",\n");
        } else {
            fprintf(f, "\n");
        }
    }

    fprintf(f, "  ],\n");
    fprintf(f, "  \"links\": [\n");

    // 3. Export Links (Flux Lines)
    // For now, we don't have an explicit 'link' list in the VM state other than implicit relations.
    // However, the prompt mentions "FluxLine: A struct representing a BIND connection".
    // The VM doesn't currently track BINDs persistently as a list, it just fuses coordinates.
    // BUT, if we want to visualize connections, we might need to infer them or track them.
    // For this Phase 7, let's assume we just visualize the nodes. 
    // OR, if we want to show 'active' binds, we might need to add a 'bound_to' field to TetryonNode later.
    // For now, let's leave links empty or mock a connection from N0 to others if they are non-zero.
    
    // Mock: Connect Origin (N0) to all active registers
    int first_link = 1;
    for (int i = 1; i < reg_count; i++) {
        // If node has significant energy
        if (registers[i].r > 0.001 || registers[i].real_coords[0] > 0.001) {
            if (!first_link) fprintf(f, ",\n");
            fprintf(f, "    {\"source\": 0, \"target\": %d}", i);
            first_link = 0;
        }
    }

    fprintf(f, "\n  ]\n");
    fprintf(f, "}\n");

    fclose(f);
    printf("[GeoGPU] Snapshot rendered to %s\n", filename);
}
