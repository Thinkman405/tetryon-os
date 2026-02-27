#!/bin/bash

echo "[Build] Compiling Tetryon Kernel..."
gcc tetryon_vm.c tetryon_fs.c tetryon_dev.c tetryon_net.c georun.c tetryon_memory.c tetryon_math.c tetryon_interrupt.c -o tetryon_vm.exe

echo "[Build] Assembling Demos..."
python geoasm.py genesis.lat genesis.bin
python geoasm.py pulsar.lat pulsar.bin
python geoasm.py storage.lat storage.bin
python geoasm.py internet.lat internet.bin

echo "[Build] Creating Distribution..."
mkdir -p Tetryon_v1.0
cp tetryon_vm.exe Tetryon_v1.0/
cp *.bin Tetryon_v1.0/

echo "[Build] Bundling..."
tar -czf tetryon_os_alpha.tar.gz Tetryon_v1.0

echo "[Build] Success! Tetryon v1.0-Alpha is ready."
