#ifndef TETRYON_NET_H
#define TETRYON_NET_H

#include <stdint.h>
#include "tetryon_math.h"

// Remote Node: A point in the hyperbolic mesh
typedef struct {
    double r;
    double theta;
    uint64_t node_id;
    char* hostname;
} RemoteNode;

// Wave Packet: Data propagating through the lattice
typedef struct {
    TetryonNode payload;
    double signal_strength; // 0.0 to 1.0
} WavePacket;

// Function Prototypes
void net_init();
double calculate_geodesic(TetryonNode* src, RemoteNode* dst);
int transmit_wave(TetryonNode data, uint64_t target_id);

#endif // TETRYON_NET_H
