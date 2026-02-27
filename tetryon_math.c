#include "tetryon_math.h"
#include <stdio.h>
#include <stdlib.h>

// Helper to hash a double for harmonic signature
uint64_t hash_value(double value) {
    union {
        double d;
        uint64_t l;
    } u;
    u.d = value;
    return u.l;
}

void init_node(TetryonNode* node, double value) {
    if (value <= 0) {
        // Handle invalid input gracefully, though in a real kernel this might panic
        value = 1.0; 
    }

    // --- Foundational Principle: The 1 x 1 = 2 Axiom ---
    double processed_value = value;
    if (value == 1.0) {
        processed_value = 2.0;
    }

    // Map to 3D curvilinear lattice
    node->r = log(processed_value);
    node->theta = sqrt(processed_value) * (M_PI / 3.0);

    node->real_coords[0] = node->r * cos(node->theta);
    node->real_coords[1] = node->r * sin(node->theta);
    node->real_coords[2] = log(processed_value);

    // Bifurcations
    node->imag_bifurcation = sqrt(processed_value) * I;
    node->negative_bifurcation[0] = -node->r * cos(node->theta);
    node->negative_bifurcation[1] = -node->r * sin(node->theta);

    node->harmonic_sig = hash_value(processed_value);
}

double apply_howard_comma(double value) {
    // Perturbative harmonic correction
    // Using a small constant for the correction factor as per Python reference logic
    // Python ref: return self.injected_value * (1 + howard_comma_constant * np.sin(self.injected_value))
    // Note: Python ref used 1e-24 locally in the method, but we have global HOWARD_COMMA_CONSTANT.
    // The prompt asks to define HOWARD_COMMA_CONSTANT as 6.626e-34.
    // We will use the global constant.
    
    return value * (1.0 + HOWARD_COMMA_CONSTANT * sin(value));
}

double geometric_distance(TetryonNode* node1, TetryonNode* node2) {
    double dx = node1->real_coords[0] - node2->real_coords[0];
    double dy = node1->real_coords[1] - node2->real_coords[1];
    double dz = node1->real_coords[2] - node2->real_coords[2];
    
    double euclidean_dist = sqrt(dx*dx + dy*dy + dz*dz);
    double imag_dist = cabs(node1->imag_bifurcation - node2->imag_bifurcation);

    // Fractal correction: D(r) = sqrt(dist^2 + imag^2 + sin^2(pi * r / C_H))
    // The prompt specifies: sin^2(pi * r / C_H) where r is euclidean_dist
    double fractal_term = sin(M_PI * euclidean_dist / HOWARD_COMMA_CONSTANT);
    double fractal_correction = fractal_term * fractal_term;

    return sqrt(euclidean_dist*euclidean_dist + imag_dist*imag_dist + fractal_correction);
}

double tetryon_force(TetryonNode* node1, TetryonNode* node2, double k) {
    double distance = geometric_distance(node1, node2);
    if (distance == 0) {
        return INFINITY;
    }
    return k / (distance * distance);
}
