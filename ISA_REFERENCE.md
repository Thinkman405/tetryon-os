# Tetryon GeoASM ISA Reference

## Instruction Format
All instructions are 64-bit Base Words, optionally followed by a 64-bit Extended Word for immediate values.

`[Opcode: 16] [Dst: 16] [Src: 16] [Mode: 16]`

## Addressing Modes
- **DIRECT (0x0000)**: Register to Register.
- **IMMEDIATE (0x0001)**: Immediate value to Register.
- **HYPERBOLIC (0x0010)**: Geometric operation.

## Opcodes

### System
- `NOP` (0x0000): No Operation.
- `HALT` (0x1111): Stop the VM.
- `GATE` (0x000D): Syscall (0=Yield, 1=Spawn, 2=Exit).
- `EXIT` (0x000E): Terminate current topology.

### Data Movement
- `LOAD` (0x0010): Load value into register.
- `MOVH` (0x0011): Move register to register.
- `PUSH` (0x2001): Allocate new node in Spiral Memory.
- `PULL` (0x2002): Retrieve node from Spiral Memory.

### Arithmetic & Logic
- `ADDH` (0x1000): Hyperbolic Addition.
- `SUBH` (0x1001): Hyperbolic Subtraction.
- `BND` (0x0110): Bind/Tensor Product.

### Control Flow
- `JUMP` (0x1010): Unconditional Jump.
- `CJMP` (0x1011): Conditional Jump (if Dst != 0).
- `RET` (0x000C): Return from Interrupt.

### Storage & IO
- `SAVE` (0x0012): Crystallize register to file. `SAVE <FileID>, <Reg>`
- `READ` (0x0013): Sublimate file to register. `READ <FileID>, <Reg>`
- `WAVE` (0x0014): Transmit wave to network. `WAVE <TargetID>, <Reg>`
- `OUT` (0x1101): Output to device (Screen).

### Physics
- `TUNE` (0x000A): Tune a Pulsar.
- `LINK` (0x000B): Link a Resonance Handler.
- `FORCE` (0x2000): Calculate Tetryon Force.

## Registers
- `N0` - `N15`: General Purpose Tetryon Nodes.
- `N14`: Screen Buffer (Convention).
- `N15`: Keyboard Buffer (Convention).
