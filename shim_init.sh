#!/bin/sh
# Tetryon OS Shim Init - PID 1
# "The Hermit Crab"

# 1. Mount Essential Filesystems
/bin/busybox mount -t proc proc /proc
/bin/busybox mount -t sysfs sysfs /sys
/bin/busybox mount -t devtmpfs devtmpfs /dev
/bin/busybox mkdir -p /dev/pts
/bin/busybox mount -t devpts devpts /dev/pts

# 2. Clear Screen and Print Banner
/bin/busybox clear
echo "  _______   _                      "
echo " |__   __| | |                     "
echo "    | | ___| |_ _ __ _   _  ___  _ "
echo "    | |/ _ \ __| '__| | | |/ _ \| |"
echo "    | |  __/ |_| |  | |_| | (_) | |"
echo "    |_|\___|\__|_|   \__, |\___/|_|"
echo "                      __/ |        "
echo "                     |___/         "
echo "Tetryon Bootloader v1.0"
echo "======================="

# 3. The Bridge Message
echo ""
echo "[ BOOT ] Bridging Linear Hardware to Hyperbolic Space..."
echo ""
/bin/busybox sleep 2

# 4. Launch Tetryon VM (Taking over the system)
# We use exec to replace this shell with the VM process
exec /bin/tetryon_vm --shell

# 5. Rescue Shell (If VM crashes)
echo ""
echo "[CRITICAL] Tetryon Kernel Panic! Dropping to rescue shell..."
exec /bin/sh
