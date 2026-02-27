#ifndef GEORUN_H
#define GEORUN_H

#include <stdint.h>
#include "tetryon_memory.h"

#define MAX_TOPOLOGIES 4
#define MAX_REGISTERS 16

// Process Topology (The "Shape" of a running process)
typedef struct {
    int id;
    int active;
    uint64_t pc;
    TetryonNode registers[MAX_REGISTERS];
    double geometric_priority; // Distance from Origin (0,0,0)
} ProcessTopology;

// Global Scheduler State
extern ProcessTopology topologies[MAX_TOPOLOGIES];
extern int current_topology_id;

// Function Prototypes
void init_scheduler();
int create_topology(uint64_t entry_point);
void destroy_topology(int id);
void schedule_next(); // The Core Geometric Logic
void context_switch(int next_id);

#endif // GEORUN_H
