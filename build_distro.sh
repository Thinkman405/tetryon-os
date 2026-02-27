#!/bin/bash
set -e
echo "[Tetryon Build] Compiling kernel..."
gcc -O2 -Wall \
    tetryon_vm.c tetryon_fs.c tetryon_dev.c tetryon_net.c georun.c \
    tetryon_memory.c tetryon_math.c tetryon_interrupt.c tetryon_gpu.c \
    tetryon_fb.c tetryon_io.c -o tetryon_vm.exe -lm

echo "[Tetryon Build] Assembling geometric modules..."
python geoasm.py genesis.lat genesis.bin
python geoasm.py pulsar.lat pulsar.bin
python geoasm.py storage.lat storage.bin
python geoasm.py internet.lat internet.bin

echo "[Tetryon Build] Creating distribution..."
rm -rf Tetryon_v1.0
mkdir -p Tetryon_v1.0/modules
cp tetryon_vm.exe Tetryon_v1.0/
cp *.bin Tetryon_v1.0/modules/
cp lattice.conf lattice_state.json README.md Tetryon_v1.0/

echo "[Tetryon Build] Packaging..."
tar -czf tetryon_os_alpha.tar.gz Tetryon_v1.0
echo "[SUCCESS] Tetryon v1.0-Alpha ready!"