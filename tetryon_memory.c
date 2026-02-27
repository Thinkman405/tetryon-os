#include "tetryon_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

SpiralSector* SpiralMemory = NULL;

void init_spiral_memory() {
    // Free existing memory to prevent leaks
    SpiralSector* current = SpiralMemory;
    while (current != NULL) {
        SpiralSector* next = current->next;
        free(current);
        current = next;
    }
    SpiralMemory = NULL;
}

// Calculate Energy: E = C_H * nu
// We use the node's radius (log(value)) or theta as a proxy for frequency/energy in this simulation.
// Higher value -> Higher Energy -> Closer to center (Head).
double calculate_energy(TetryonNode* n) {
    // Using r (log(value)) as the primary energy metric for sorting.
    // Larger value = Larger r.
    // Let's say High Energy = Large Value.
    return n->r; 
}

void spiral_alloc(TetryonNode n) {
    SpiralSector* new_sector = (SpiralSector*)malloc(sizeof(SpiralSector));
    new_sector->data = n;
    new_sector->next = NULL;
    
    double energy = calculate_energy(&n);
    
    // Insert sorted by Energy (Descending) -> Highest energy at head (Center)
    if (SpiralMemory == NULL || calculate_energy(&SpiralMemory->data) < energy) {
        new_sector->next = SpiralMemory;
        SpiralMemory = new_sector;
    } else {
        SpiralSector* current = SpiralMemory;
        while (current->next != NULL && calculate_energy(&current->next->data) >= energy) {
            current = current->next;
        }
        new_sector->next = current->next;
        current->next = new_sector;
    }

    // Update Geometry (Radius/Theta) based on position in list
    // r = a * e^(b*theta)
    // We will just assign increasing radius as we go down the list to simulate the spiral arm.
    SpiralSector* current = SpiralMemory;
    int index = 0;
    while (current != NULL) {
        current->radius = 1.0 + (index * 0.5); // Simple linear growth for visualization
        current->theta = index * (M_PI / 4.0); // 45 degrees per node
        current = current->next;
        index++;
    }
    
    printf("Node [%llu] suspended at Radius [%.2f]\n", n.harmonic_sig, new_sector->radius);
}

int spiral_retrieve(uint64_t harmonic_sig, TetryonNode* out_node) {
    SpiralSector* current = SpiralMemory;
    SpiralSector* prev = NULL;

    while (current != NULL) {
        if (current->data.harmonic_sig == harmonic_sig) {
            *out_node = current->data;
            
            // Remove from list
            if (prev == NULL) {
                SpiralMemory = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            return 1; // Found
        }
        prev = current;
        current = current->next;
    }
    return 0; // Not found
}

void visualize_spiral() {
    printf("\n--- Spiral Memory State ---\n");
    SpiralSector* current = SpiralMemory;
    if (current == NULL) {
        printf("(Empty)\n");
        return;
    }
    while (current != NULL) {
        printf("  R:%.2f Theta:%.2f | Sig: %llu | Val: %.2f\n", 
               current->radius, current->theta, current->data.harmonic_sig, exp(current->data.r));
        current = current->next;
    }
    printf("---------------------------\n");
}
