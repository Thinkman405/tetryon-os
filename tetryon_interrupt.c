#include <stdio.h>
#include <math.h>
#include "tetryon_interrupt.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Pulsar interrupt_vector[MAX_PULSARS];

void init_interrupts() {
    for (int i = 0; i < MAX_PULSARS; i++) {
        interrupt_vector[i].active = 0;
        interrupt_vector[i].frequency = 0.0;
        interrupt_vector[i].current_theta = 0.0;
        interrupt_vector[i].handler_instruction_ptr = 0;
    }
    printf("[System] Resonance Engine Initialized.\n");
}

// Called every VM cycle. Increments theta for all active pulsars.
void update_pulsars() {
    for (int i = 0; i < MAX_PULSARS; i++) {
        if (interrupt_vector[i].active) {
            interrupt_vector[i].current_theta += interrupt_vector[i].frequency;
        }
    }
}

// Checks if any Pulsar has completed a cycle (2PI).
// Returns the handler instruction pointer if resonance occurs, otherwise -1.
// Prioritizes lower index pulsars if multiple fire (simple priority).
int check_resonance() {
    for (int i = 0; i < MAX_PULSARS; i++) {
        if (interrupt_vector[i].active) {
            if (interrupt_vector[i].current_theta >= 2 * M_PI) {
                // Resonance Spike!
                printf("[~] Resonance Spike detected on Node %d!\n", i);
                
                // Reset phase (wrap around)
                interrupt_vector[i].current_theta -= 2 * M_PI;
                
                // Return the handler to jump to
                if (interrupt_vector[i].handler_instruction_ptr != 0) {
                    return (int)interrupt_vector[i].handler_instruction_ptr;
                }
            }
        }
    }
    return -1;
}

// Converts a standard Register into a Pulsar (TUNE Opcode)
void tune_node(int index, double freq) {
    if (index >= 0 && index < MAX_PULSARS) {
        interrupt_vector[index].active = 1;
        interrupt_vector[index].frequency = freq;
        interrupt_vector[index].current_theta = 0.0; // Reset phase on retune
        printf("[System] Node %d Tuned to Frequency %.4f\n", index, freq);
    }
}

// Links a Pulsar to a Code Label/Address (LINK Opcode)
void link_handler(int index, uint64_t handler_ip) {
    if (index >= 0 && index < MAX_PULSARS) {
        interrupt_vector[index].handler_instruction_ptr = handler_ip;
        printf("[System] Node %d Linked to Handler at 0x%llX\n", index, handler_ip);
    }
}
