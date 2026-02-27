# Tetryon OS v1.0-Alpha

> "The Universe is not linear. Your OS shouldn't be either."

## Overview
Tetryon OS is a non-linear, hyperbolic operating system based on the principles of Tetryon Geometry. Unlike traditional systems that use linear memory and hierarchical files, Tetryon uses **Spiral Memory**, **Crystalline Storage**, and **Hyperbolic Mesh Networking**.

## Core Concepts

### 1. Spiral Memory
Data is not stored in linear addresses (0x00...0xFF). It is mapped to nodes in a 3D curvilinear lattice ($r, \theta, \phi$). Memory allocation follows a spiral pattern, ensuring optimal geometric distribution.

### 2. Crystalline Storage (GeoFS)
Files are not streams of bytes. They are **Crystals**—rigid geometric structures persisted to disk. Reading a file ("Sublimation") reconstructs the geometry in memory. Writing a file ("Crystallization") collapses the wave function into a static shape.

### 3. Hyperbolic Mesh (GeoNet)
Networking does not use TCP/IP. Data propagates as **Wavefronts** along geodesics in a hyperbolic mesh. Nodes are addressed by their spatial coordinates, and signal strength decays with distance ($1/r^2$).

### 4. Resonance (Time)
There is no system clock. Time is measured in **Resonance Cycles** of Pulsars. Processes are scheduled based on their geometric priority and harmonic alignment.

## Build Instructions

### Prerequisites
- GCC Compiler
- Python 3 (for Assembler)

### Building
Run the build script:
```bash
./build_distro.sh
```
This will compile the kernel (`tetryon_vm.exe`), assemble the demos, and create a `Tetryon_v1.0` distribution folder.

## Usage

### Running a Program
```bash
./tetryon_vm.exe <program.bin>
```

### Interactive Shell
```bash
./tetryon_vm.exe --shell
```

## Configuration
Edit `lattice.conf` to define remote nodes for the mesh network:
```
# ID, R, Theta, Hostname
1,1.01,0.5,Node-Alpha
```

## License
Proprietary - Tetryon Research Group.
