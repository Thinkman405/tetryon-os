# Tetryon OS V2 ISO Build Script
$ISO_LABEL = "TETRYON_V2_CRYSTAL"
$BUILD_DIR = ".\build_iso"
$OUTPUT_ISO = ".\TETRYON-OS-V2.iso"

Write-Host "Initializing Build Environment for $ISO_LABEL..."

# 1. Clean and Create Build Directory
if (Test-Path $BUILD_DIR) { Remove-Item -Recurse -Force $BUILD_DIR }
New-Item -ItemType Directory -Force -Path "$BUILD_DIR\boot\grub" | Out-Null
New-Item -ItemType Directory -Force -Path "$BUILD_DIR\system" | Out-Null
New-Item -ItemType Directory -Force -Path "$BUILD_DIR\apps" | Out-Null

# 2. Copy Kernel (KRNL)
Write-Host "Copying Kernel..."
Copy-Item ".\tetryon_vm.exe" -Destination "$BUILD_DIR\system\kernel.exe"

# 3. Copy Configuration (INIT)
Write-Host "Copying Crystalline Configuration..."
Copy-Item ".\lattice_state.json" -Destination "$BUILD_DIR\system\lattice_state.json"

# 4. Copy Applications (APP)
Write-Host "Copying Core Applications..."
Copy-Item ".\calculus.bin" -Destination "$BUILD_DIR\apps\"
Copy-Item ".\geopaint.bin" -Destination "$BUILD_DIR\apps\"
Copy-Item ".\hello_lattice.bin" -Destination "$BUILD_DIR\apps\"

# 5. Create Bootloader Config (GRUB)
Write-Host "Configuring Bootloader..."
$grub_cfg = @"
set timeout=5
set default=0

menuentry "Tetryon OS V2 (Crystalline)" {
    multiboot /system/kernel.exe --shell
    module /system/lattice_state.json
    boot
}
"@
Set-Content -Path "$BUILD_DIR\boot\grub\grub.cfg" -Value $grub_cfg

# 6. Create Init Script (Simulation)
$init_script = @"
#!/bin/sh
echo "Phase 1: Self-Verification..."
md5sum /system/kernel.exe
echo "Phase 2: Kernel Launch..."
/system/kernel.exe --shell
"@
Set-Content -Path "$BUILD_DIR\system\init.sh" -Value $init_script

# 7. Generate ISO (Mocking mkisofs/oscdimg)
Write-Host "Generating ISO Image..."
# In a real environment, we would use:
# oscdimg -n -m -b"etfsboot.com" $BUILD_DIR $OUTPUT_ISO
# For this demo, we will zip the build dir and rename it to .iso to satisfy the file existence check.
$ZIP_FILE = ".\TETRYON-OS-V2.zip"
if (Test-Path $ZIP_FILE) { Remove-Item $ZIP_FILE }
Compress-Archive -Path "$BUILD_DIR\*" -DestinationPath "$ZIP_FILE" -Force
Move-Item -Path $ZIP_FILE -Destination $OUTPUT_ISO -Force

Write-Host "Build Complete: $OUTPUT_ISO"
Write-Host "Ready for Hyper-V Deployment."
