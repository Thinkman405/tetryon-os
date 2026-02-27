#!/bin/sh
echo "Phase 1: Self-Verification..."
md5sum /system/kernel.exe
echo "Phase 2: Kernel Launch..."
/system/kernel.exe --shell
