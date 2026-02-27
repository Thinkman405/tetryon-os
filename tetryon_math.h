#ifndef TETRYON_MATH_H
#define TETRYON_MATH_H

#include <math.h>
#include <complex.h>
#include <stdint.h>

// Define Howard Comma Constant
#define HOWARD_COMMA_CONSTANT 6.626e-34
#define M_PI 3.14159265358979323846

// TetryonNode Structure
typedef struct {
    double r;
    double theta;
    double real_coords[3];
    double complex imag_bifurcation;
    double negative_bifurcation[2];
    uint64_t harmonic_sig;
} TetryonNode;

// Function Declarations
void init_node(TetryonNode* node, double value);
double apply_howard_comma(double value);
double geometric_distance(TetryonNode* node1, TetryonNode* node2);
double tetryon_force(TetryonNode* node1, TetryonNode* node2, double k);
void geometric_tensor_product(TetryonNode* nodes, int count, double* result_tensor); // Optional for Phase 1 but good to have signature

#endif // TETRYON_MATH_H
