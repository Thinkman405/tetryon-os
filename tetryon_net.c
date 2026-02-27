#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tetryon_net.h"

#define MAX_REMOTE_NODES 64
#define SIGNAL_THRESHOLD 0.1
#define CONFIG_FILE "lattice.conf"

RemoteNode mesh_nodes[MAX_REMOTE_NODES];
int remote_node_count = 0;

void load_lattice_config() {
    FILE* f = fopen(CONFIG_FILE, "r");
    if (!f) {
        printf("[GeoNet] Warning: %s not found. Mesh is empty.\n", CONFIG_FILE);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || strlen(line) < 5) continue; // Skip comments/empty

        // Format: ID, R, Theta, Hostname
        uint64_t id;
        double r, theta;
        char hostname[64];

        if (sscanf(line, "%llu,%lf,%lf,%s", &id, &r, &theta, hostname) == 4) {
            if (remote_node_count < MAX_REMOTE_NODES) {
                mesh_nodes[remote_node_count].node_id = id;
                mesh_nodes[remote_node_count].r = r;
                mesh_nodes[remote_node_count].theta = theta;
                mesh_nodes[remote_node_count].hostname = strdup(hostname);
                remote_node_count++;
            }
        }
    }
    fclose(f);
}

void net_init() {
    printf("[GeoNet] Initializing Hyperbolic Mesh (Production Mode)...\n");
    remote_node_count = 0;
    
    // Hotfix 1: Robust Deterministic Initialization
    // Node 1: Alpha
    mesh_nodes[0].node_id = 1;
    mesh_nodes[0].r = 5.0;
    mesh_nodes[0].theta = 0.5;
    mesh_nodes[0].hostname = strdup("Alpha");
    
    // Node 2: Beta
    mesh_nodes[1].node_id = 2;
    mesh_nodes[1].r = 10.0;
    mesh_nodes[1].theta = 1.5;
    mesh_nodes[1].hostname = strdup("Beta");

    // Node 3: Gamma
    mesh_nodes[2].node_id = 3;
    mesh_nodes[2].r = 15.0;
    mesh_nodes[2].theta = 2.5;
    mesh_nodes[2].hostname = strdup("Gamma");
    
    remote_node_count = 3;
    
    // Load additional from config if needed, but ensure base mesh exists
    load_lattice_config();
    
    printf("[GeoNet] Mesh Established. %d Remote Singularities detected.\n", remote_node_count);
}

// Calculate Geodesic: Distance in hyperbolic space
double calculate_geodesic(TetryonNode* src, RemoteNode* dst) {
    // Simplified polar distance for now
    double r1 = src->r; // Assuming src has valid r/theta
    double t1 = src->theta;
    double r2 = dst->r;
    double t2 = dst->theta;
    
    double dist_sq = r1*r1 + r2*r2 - 2*r1*r2*cos(t1 - t2);
    return sqrt(dist_sq);
}

int transmit_wave(TetryonNode data, uint64_t target_id) {
    // Hotfix 2: Search by ID (Corrected Logic)
    RemoteNode* target = NULL;
    for (int i = 0; i < remote_node_count; i++) {
        if (mesh_nodes[i].node_id == target_id) {
            target = &mesh_nodes[i];
            break;
        }
    }
    
    if (!target) {
        printf("[GeoNet] Error: Target Node %llu not found in Lattice.\n", target_id);
        return 0;
    }
    
    printf("[GeoNet] Wavefront propagating to %s (ID: %llu)...\n", target->hostname, target_id);
    
    // Simulate distance from "Origin" (where the VM is, r=0)
    // Distance is just target->r
    double distance = target->r;
    if (distance < 1.0) distance = 1.0; 
    
    double initial_strength = 100.0;
    double signal_strength = initial_strength / (distance * distance);
    
    printf("[GeoNet] Signal Strength: %.2f%%...\n", signal_strength);
    
    if (signal_strength > SIGNAL_THRESHOLD) {
        printf("[GeoNet] Delivery Confirmed.\n");
        return 1;
    } else {
        printf("[GeoNet] Signal lost in the noise floor.\n");
        return 0;
    }
}
