#ifndef TETRYON_INTERRUPT_H
#define TETRYON_INTERRUPT_H

#include <stdint.h>

// Maximum number of pulsars allowed in the system
#define MAX_PULSARS 16

// The Pulsar Node Structure
// Time is Frequency. A Pulsar oscillates (rotates theta) automatically.
typedef struct {
    int active;                     // 1 if oscillating, 0 if static
    double frequency;               // Increment per cycle (d_theta)
    double current_theta;           // Current phase angle (0 to 2PI)
    uint64_t handler_instruction_ptr; // Address of the Resonance Handler
} Pulsar;

// The Interrupt Vector (The Rhythm Section)
extern Pulsar interrupt_vector[MAX_PULSARS];

// Function Prototypes
void init_interrupts();
void update_pulsars();
int check_resonance(); // Returns handler IP or -1
void tune_node(int index, double freq);
void link_handler(int index, uint64_t handler_ip);

#endif // TETRYON_INTERRUPT_H
