#!/bin/bash
# build_bootable.sh - Tetryon OS ISO Builder (WSL Version)
# Usage: ./build_bootable.sh

set -e
# Production fixes
sed -i 's/\r$//' *.sh 2>/dev/null || true
rm -f TETRYON-OS-V2.iso

WORK_DIR="iso_build"
ISO_DIR="$WORK_DIR/iso"
INITRAMFS_DIR="$WORK_DIR/initramfs"

echo "[Builder] Initializing Tetryon ISO Build Environment..."

# 0. Fix Line Endings (Self-Healing)
# This handles if files were edited in Windows
sed -i 's/\r$//' shim_init.sh 2>/dev/null || true

# 1. Check Dependencies
MISSING_DEPS=""
for cmd in xorriso grub-mkrescue gcc wget; do
    if ! command -v $cmd &> /dev/null; then
        MISSING_DEPS="$MISSING_DEPS $cmd"
    fi
done

if [ ! -z "$MISSING_DEPS" ]; then
    echo "[Error] Missing dependencies:$MISSING_DEPS"
    echo "Please install them in WSL:"
    echo "  sudo apt-get update && sudo apt-get install -y build-essential xorriso grub-common grub-pc-bin wget"
    exit 1
fi

# 2. Prepare Directories
echo "[Builder] Cleaning workspace..."
rm -rf $WORK_DIR
mkdir -p $ISO_DIR/boot/grub
mkdir -p $INITRAMFS_DIR/bin
mkdir -p $INITRAMFS_DIR/proc
mkdir -p $INITRAMFS_DIR/sys
mkdir -p $INITRAMFS_DIR/dev

# 3. Compile Tetryon VM (Static)
echo "[Builder] Compiling Tetryon Kernel (Static)..."
# Note: linking static to avoid libc dependency issues in initramfs
gcc -static tetryon_vm.c tetryon_gpu.c tetryon_memory.c tetryon_math.c tetryon_interrupt.c georun.c tetryon_fs.c tetryon_net.c tetryon_dev.c tetryon_fb.c -o $INITRAMFS_DIR/bin/tetryon_vm -lm
if [ $? -ne 0 ]; then
    echo "[Error] Compilation failed!"
    exit 1
fi

# 4. Download Busybox (Static)
if [ ! -f "busybox" ]; then
    echo "[Builder] Fetching Busybox..."
    wget -q -O busybox https://busybox.net/downloads/binaries/1.35.0-x86_64-linux-musl/busybox
    chmod +x busybox
else
    echo "[Builder] Using local busybox..."
fi
cp busybox $INITRAMFS_DIR/bin/busybox
chmod +x $INITRAMFS_DIR/bin/busybox

# Create symlinks for busybox
$INITRAMFS_DIR/bin/busybox --install -s $INITRAMFS_DIR/bin

# 5. Install Init Script
echo "[Builder] Installing Shim Init..."
cp shim_init.sh $INITRAMFS_DIR/init
chmod +x $INITRAMFS_DIR/init

# 6. Pack Initramfs
echo "[Builder] Packing Initramfs..."
cd $INITRAMFS_DIR
find . | cpio -o -H newc | gzip > ../../$ISO_DIR/boot/initramfs.gz
cd ../..

# 7. Check for Kernel Seed
if [ ! -f "vmlinuz" ]; then
    echo "[WARNING] 'vmlinuz' (Linux Kernel) not found!"
    echo "Attempting to download Alpine Linux kernel..."
    wget -O vmlinuz https://dl-cdn.alpinelinux.org/alpine/v3.18/releases/x86_64/netboot/vmlinuz-virt
fi
cp vmlinuz $ISO_DIR/boot/vmlinuz

# 8. Generate GRUB Config
echo "[Builder] Generating GRUB Config..."
cat > $ISO_DIR/boot/grub/grub.cfg <<EOF
set timeout=5
set default=0

menuentry "Tetryon OS v2.0 (Hyperbolic)" {
    linux /boot/vmlinuz quiet console=tty0
    initrd /boot/initramfs.gz
}
EOF

# 9. Build ISO
echo "[Builder] Forging ISO Image..."
grub-mkrescue -o TETRYON-OS-V2.iso $ISO_DIR

echo ""
echo "[Success] TETRYON-OS-V2.iso created!"
echo "Size: $(du -h TETRYON-OS-V2.iso | cut -f1)"
