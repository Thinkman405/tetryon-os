# Tetryon OS Deployment Guide (Phase 8)

## Overview
This guide explains how to build the bootable `tetryon_live.iso` and deploy it to a Hyper-V or VirtualBox virtual machine. We use a "Shim" approach, wrapping the Tetryon VM in a minimal Linux environment to interface with bare metal hardware.

## Prerequisites
To build the ISO, you need a **Linux environment** (WSL2, Ubuntu, Debian, etc.) with the following tools installed:
- `gcc` (for compiling the kernel)
- `xorriso` (for ISO creation)
- `grub-pc-bin` or `grub-common` (for bootloader)
- `wget` (for downloading dependencies)

**Install Command (Ubuntu/Debian):**
```bash
sudo apt-get update
sudo apt-get install build-essential xorriso grub-pc-bin grub-common wget
```

## Build Instructions

1.  **Transfer Files**: Ensure all Tetryon source files (`.c`, `.h`, `.py`), `shim_init.sh`, and `build_bootable.sh` are in the same directory on your Linux machine.
2.  **Make Executable**:
    ```bash
    chmod +x build_bootable.sh shim_init.sh
    ```
3.  **Run Builder**:
    ```bash
    ./build_bootable.sh
    ```
    *The script will automatically download a minimal Linux kernel (Alpine vmlinuz) if one is not present.*

4.  **Output**:
    - Upon success, you will see `[Success] tetryon_live.iso created!`.
    - The file `tetryon_live.iso` will be in your current directory.

## Deployment (Hyper-V)

1.  **Create VM**:
    - Open Hyper-V Manager.
    - Create a **New Virtual Machine**.
    - **Generation**: Choose **Generation 1** (BIOS) for maximum compatibility, or **Generation 2** (UEFI) if you disable Secure Boot.
    - **Memory**: 512MB is sufficient.
    - **Network**: Default Switch (optional).

2.  **Attach ISO**:
    - Go to VM **Settings**.
    - Select **DVD Drive** (IDE Controller 1).
    - Select **Image file** and browse to `tetryon_live.iso`.

3.  **Boot**:
    - Start the VM.
    - You should see the GRUB menu: `Tetryon OS v1.0 (Hyperbolic)`.
    - Press Enter.
    - Watch for the banner: `[ BOOT ] Bridging Linear Hardware to Hyperbolic Space...`.
    - The Tetryon Shell `[∞]::> ` will appear.

## Troubleshooting

-   **"Kernel Panic - not syncing: No working init found"**:
    - Ensure `shim_init.sh` was compiled/copied correctly as `/init` in the initramfs.
    - Ensure `shim_init.sh` has executable permissions (`chmod +x`).
-   **"Exec format error"**:
    - Ensure `tetryon_vm` was compiled with `-static`. The build script does this automatically.
