#include <stdio.h>
#include <math.h>
#include "georun.h"
#include "tetryon_math.h"

ProcessTopology topologies[MAX_TOPOLOGIES];
int current_topology_id = -1;

// External references to VM state (defined in tetryon_vm.c)
extern TetryonNode registers[MAX_REGISTERS];
extern uint64_t pc;

void init_scheduler() {
    for (int i = 0; i < MAX_TOPOLOGIES; i++) {
        topologies[i].id = i;
        topologies[i].active = 0;
        topologies[i].geometric_priority = 1000.0; // Far away (low priority)
    }
    
    // Create the initial Kernel Topology (ID 0)
    topologies[0].active = 1;
    topologies[0].geometric_priority = 0.0; // At the center
    current_topology_id = 0;
    
    printf("[GeoRun] Scheduler Initialized. Kernel Topology Active.\n");
}

int create_topology(uint64_t entry_point) {
    for (int i = 0; i < MAX_TOPOLOGIES; i++) {
        if (!topologies[i].active) {
            topologies[i].active = 1;
            topologies[i].pc = entry_point;
            topologies[i].geometric_priority = 0.0; // Start at center (high priority)
            
            // Initialize registers to zero
            for(int r=0; r<MAX_REGISTERS; r++) {
                topologies[i].registers[r].r = 0;
                topologies[i].registers[r].theta = 0;
                topologies[i].registers[r].harmonic_sig = 0;
                for(int k=0; k<3; k++) topologies[i].registers[r].real_coords[k] = 0.0;
            }
            
            printf("[GeoRun] Spawning Topology ID: %d at PC: 0x%llX\n", i, entry_point);
            fflush(stdout);
            return i;
        }
    }
    printf("[GeoRun] Error: Max Topologies Reached.\n");
    return -1;
}

void destroy_topology(int id) {
    if (id >= 0 && id < MAX_TOPOLOGIES) {
        topologies[id].active = 0;
        topologies[id].geometric_priority = 1000.0;
        printf("[GeoRun] Destroyed Topology ID: %d\n", id);
    }
}

// Calculates distance from Origin for a Topology
// We use the "Mass" of the registers to determine priority.
// For this prototype, we'll use a simple accumulation of activity.
// Every time a process runs, we push it away (decay).
void schedule_next() {
    // 1. Decay the current process (Push it away from center)
    if (current_topology_id != -1 && current_topology_id < MAX_TOPOLOGIES) {
        topologies[current_topology_id].geometric_priority += 0.1; 
        
        // Save current state back to topology
        topologies[current_topology_id].pc = pc;
        for(int i=0; i<MAX_REGISTERS; i++) {
            topologies[current_topology_id].registers[i] = registers[i];
        }
    }

    // 2. Find the closest active topology to the Origin (Lowest Priority Value)
    int best_id = -1;
    double min_dist = 999999.0;

    for (int i = 0; i < MAX_TOPOLOGIES; i++) {
        if (topologies[i].active) {
            // Natural Decay Recovery (Gravity pulls them back in)
            if (topologies[i].geometric_priority > 0.0) {
                topologies[i].geometric_priority -= 0.05; 
                if (topologies[i].geometric_priority < 0) topologies[i].geometric_priority = 0;
            }

            if (topologies[i].geometric_priority < min_dist) {
                min_dist = topologies[i].geometric_priority;
                best_id = i;
            }
        }
    }

    // 3. Context Switch if needed
    if (best_id != -1 && best_id != current_topology_id) {
        context_switch(best_id);
    }
}

void context_switch(int next_id) {
    printf("[GeoRun] Switched to Topology ID: %d (Dist: %.4f)\n", next_id, topologies[next_id].geometric_priority);
    fflush(stdout);
    
    // Restore state from next topology
    pc = topologies[next_id].pc;
    for(int i=0; i<MAX_REGISTERS; i++) {
        registers[i] = topologies[next_id].registers[i];
    }
    
    current_topology_id = next_id;
}
