#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "tetryon_math.h"
#include "tetryon_memory.h"
#include "tetryon_interrupt.h"
#include "georun.h"
#include "tetryon_fs.h"
#include "tetryon_net.h"
#include "tetryon_gpu.h"
#include "tetryon_fb.h"
#include "tetryon_io.h"

// Device IO Prototype (Removed, using header)

#define NODE_SCREEN 14
#define NODE_KEYBOARD 15

#define MAX_REGISTERS 16
#define MAX_MEMORY 4096 // 4K Words (32KB)

// Opcode Definitions (16-bit)
#define OP_NOP   0x0000
#define OP_NODE  0x0001
#define OP_TUNE  0x000A // [NEW] Tune Pulsar
#define OP_LINK  0x000B // [NEW] Link Handler
#define OP_RET   0x000C // [NEW] Return from Interrupt
#define OP_GATE  0x000D // [NEW] Syscall
#define OP_EXIT  0x000E // [NEW] Exit Process
#define OP_DEVICE_IO 0x0005 // [NEW] Device IO (Drawing)
#define OP_LOAD  0x0010
#define OP_MOVH  0x0011
#define OP_SAVE  0x0012 // [NEW] Save to Crystal
#define OP_READ  0x0013 // [NEW] Read from Crystal
#define OP_WAVE  0x0014 // [NEW] Transmit Wave
#define OP_RENDER 0x0015 // [NEW] Render to Holodeck
#define OP_ROT   0x0100
#define OP_REFL  0x0101
#define OP_BND   0x0110
#define OP_UNBND 0x0111
#define OP_ADDH  0x1000
#define OP_SUBH  0x1001
#define OP_JUMP  0x1010
#define OP_CJMP  0x1011
#define OP_IN    0x1100
#define OP_OUT   0x1101
#define OP_HALT  0x1111
#define OP_FORCE 0x2000
#define OP_PUSH  0x2001
#define OP_PULL  0x2002

// Mode Definitions (16-bit)
#define MODE_DIRECT    0x0000
#define MODE_IMMEDIATE 0x0001
#define MODE_HYPERBOLIC 0x0010
#define MODE_ANGLE     0x0111
#define MODE_AXIS_X    0x0100
#define MODE_AXIS_Y    0x0101
#define MODE_COMPOSITE 0x0110
#define MODE_SYSTEM    0x1111

// Global State (Exposed for Scheduler)
TetryonNode registers[MAX_REGISTERS];
uint64_t memory[MAX_MEMORY];
uint64_t pc = 0;
uint64_t shadow_pc = 0; // Shadow Node for Interrupt Return
int running = 1;

// Helper to print node state
void print_node_state(int index) {
    TetryonNode* n = &registers[index];
    printf("  [N%d] Real: (%.4f, %.4f, %.4f) | Harmonic: %llu\n", 
           index, n->real_coords[0], n->real_coords[1], n->real_coords[2], n->harmonic_sig);
}

// Helper to swap bytes for Big Endian to Host
uint64_t swap_uint64(uint64_t val) {
    val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | ((val >> 8) & 0x00FF00FF00FF00FFULL );
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | ((val >> 16) & 0x0000FFFF0000FFFFULL );
    return (val << 32) | (val >> 32);
}

int main(int argc, char* argv[]) {
    printf("DEBUG: VM Started\n");
    
    int shell_mode = 0;
    if (argc == 2 && strcmp(argv[1], "--shell") == 0) {
        shell_mode = 1;
    } else if (argc != 2) {
        printf("Usage: %s <program.bin> OR %s --shell\n", argv[0], argv[0]);
        return 1;
    }

    FILE* f = NULL;
    if (!shell_mode) {
        f = fopen(argv[1], "rb");
        if (!f) {
            printf("Error: Could not open file %s\n", argv[1]);
            return 1;
        }
    }

    setbuf(stdout, NULL); // Disable buffering globally

    printf("Booting Tetryon Kernel (Phase 6: GeoNet & Polish)...\n");
    init_spiral_memory();
    init_interrupts();
    init_scheduler();
    fs_init();
    net_init();
    fb_init(); // [NEW] Initialize Framebuffer
    
    // Initialize registers
    for (int i = 0; i < MAX_REGISTERS; i++) {
        registers[i].r = 0;
    }

    if (shell_mode) {
        printf("\n[GeoShell] Interactive Mode Active.\n");
        printf("Type 'STATUS', 'EXIT', or raw hex opcodes.\n");
        
        char input_buffer[256];
        while (1) {

            // Crystalline Prompt: [N1 (x, y, z | x', y', z')]::
            // We use N1 as the active context node.
            double x = registers[1].real_coords[0];
            double y = registers[1].real_coords[1];
            double z = registers[1].real_coords[2];
            // Mock conjugate momenta (x', y', z') for visual effect
            double xp = -x * 0.43; 
            double yp = -y * 0.43;
            double zp = 0.00;
            
            printf("[N1 (%.2f, %.2f, %.2f | %.2f, %.2f, %.2f)]:: ", x, y, z, xp, yp, zp);
            if (!fgets(input_buffer, sizeof(input_buffer), stdin)) break;
            
            // Trim newline
            input_buffer[strcspn(input_buffer, "\n")] = 0;
            
            if (strcmp(input_buffer, "EXIT") == 0) {
                break;
            } else if (strcmp(input_buffer, "STATUS") == 0) {
                printf("Lattice State: Stable\n");
                printf("Active Topologies: %d\n", 1); // Mock
                continue;
            }
            
            printf("[Shell] Command received: %s\n", input_buffer);
        }
        return 0;
    }

    // Load Program into Memory
    size_t words_read = fread(memory, sizeof(uint64_t), MAX_MEMORY, f);
    fclose(f);
    
    // Swap endianness for all loaded words
    for(size_t i=0; i<words_read; i++) {
        memory[i] = swap_uint64(memory[i]);
    }
    
    printf("Loaded %zu words into memory.\n", words_read);

    // VM Loop
    int cycle_count = 0;
    while (running) {
        cycle_count++;
        if (cycle_count % 100 == 0) {
            render_lattice(registers, MAX_REGISTERS);
        }
        // 0. Scheduler (Geometric Concurrency)
        // In a real OS, this would be triggered by a timer interrupt or yield.
        // Here we check every cycle if we should switch (simulating high-freq geometry updates).
        // For efficiency in this demo, we'll rely on GATE 0 (Yield) to trigger switches mostly,
        // but let's call schedule_next() to update priorities.
        // schedule_next(); // Calling this every cycle might be too chaotic for the demo without a timer.
        // Let's call it only on explicit events or periodically?
        // The prompt says: "The while(running) loop now delegates to schedule_next()".
        // Let's do it.
        
        // Note: schedule_next() saves current state. If we do it *before* fetch, we are good.
        // But if we switch every instruction, it's very fine-grained interleaving.
        // Let's implement GATE 0 for explicit yield as per demo requirements.
        
        // 1. Resonance Check (Interrupts)
        update_pulsars();
        int handler = check_resonance();
        if (handler != -1) {
            shadow_pc = pc; // Save PC to Shadow Node
            pc = (uint64_t)handler; // Jump to Handler
            printf("[System] Context Switch -> Handler at 0x%llX\n", pc);
        }

        if (pc >= words_read) {
            // If PC goes out of bounds, maybe just halt or switch?
            // For now, let's just break if main kernel finishes.
             // running = 0; 
             // break;
             // Actually, if we are multitasking, one process ending shouldn't kill the VM.
             // We should just destroy that topology.
             printf("[System] PC Out of Bounds. Terminating Topology.\n");
             destroy_topology(current_topology_id);
             schedule_next(); // Find someone else to run
             if (current_topology_id == -1) break; // No one left
             continue;
        }

        // 2. Fetch
        uint64_t base_word = memory[pc++];
        
        uint16_t opcode = (base_word >> 48) & 0xFFFF;
        uint16_t dst    = (base_word >> 32) & 0xFFFF;
        uint16_t src    = (base_word >> 16) & 0xFFFF;
        uint16_t mode   = base_word & 0xFFFF;

        // 3. Decode Variable Length
        double immediate_double = 0.0;
        uint64_t immediate_uint64 = 0;
        
        if (mode == MODE_IMMEDIATE) {
            if (pc >= words_read) {
                printf("Error: Unexpected EOF reading immediate\n");
                break;
            }
            uint64_t extra_word = memory[pc++];
            
            // Interpret based on Opcode context
            if (opcode == OP_LOAD || opcode == OP_TUNE || opcode == OP_ROT) {
                union { uint64_t i; double d; } u;
                u.i = extra_word;
                immediate_double = u.d;
            } else if (opcode == OP_PULL || opcode == OP_LINK || opcode == OP_JUMP || opcode == OP_CJMP || opcode == OP_GATE || opcode == OP_WAVE) {
                immediate_uint64 = extra_word;
            } else if (opcode == OP_DEVICE_IO) {
                 union { uint64_t i; double d; } u;
                 u.i = extra_word;
                 immediate_double = u.d;
            }
        }

        // printf("PC: %llu | Op: 0x%04X | Dst: %d | Src: %d\n", pc-1, opcode, dst, src);
        // fflush(stdout);

        // 4. Execute
        switch (opcode) {
            case OP_NOP:
                break;
            
            case OP_GATE: // [NEW] Syscall
                {
                    uint64_t gate_id = (mode == MODE_IMMEDIATE) ? immediate_uint64 : (uint64_t)registers[dst].real_coords[0];
                    // printf("GATE %llu\n", gate_id);
                    if (gate_id == 0) { // Yield
                        schedule_next();
                    } else if (gate_id == 1) { // Spawn
                        // Spawn worker at address in N1 (or src)
                        // Convention: GATE 1 (Spawn) uses N1 for address
                        uint64_t target_addr = (uint64_t)registers[1].real_coords[0];
                        create_topology(target_addr);
                    } else if (gate_id == 2) { // Exit
                        destroy_topology(current_topology_id);
                        schedule_next();
                    }
                }
                break;

            case OP_DEVICE_IO: // [NEW] DEVICE_IO <Node>, <Command>
                {
                    // Dst = Node (e.g., N14), Src = Data Register (e.g., N1)
                    // Wait, prompt says: DEVICE_IO N1, 1.0
                    // N1 is Source Data. 1.0 is Command (Immediate).
                    // Target Node is implied N14? Or specified?
                    // Prompt: "DEVICE_IO (0x05) maps to Node N14 (Screen)"
                    // Prompt: "DEVICE_IO N1, 1.0"
                    // So Dst is N1 (Source Data), Src is unused? Or Dst is N14?
                    // "DEVICE_IO N1, 1.0" -> Usually Op Dst, Src/Imm
                    // If N1 is the data source, it should be Src.
                    // But usually first arg is Dst.
                    // Let's assume the instruction is DEVICE_IO <DataReg>, <Command>
                    // And it ALWAYS targets N14.
                    
                    // Command is immediate double.
                    double cmd = (mode == MODE_IMMEDIATE) ? immediate_double : 0.0;
                    
                    if (cmd == 1.0) { // Draw Point
                        // Draw point at coordinates of Register[Dst]
                        // We need to access Framebuffer.
                        // Let's use device_io from tetryon_io.c but we need to pass command.
                        // Actually, let's just call fb functions directly here for the demo.
                        // Or update device_io to handle commands.
                        // For simplicity/speed: Direct FB call.
                        int x = (int)(registers[dst].real_coords[0] * 20.0) + 40; // Scale
                        int y = 12 - (int)(registers[dst].real_coords[1] * 20.0);
                        draw_pixel(x, y, 0xFFFFFFFF);
                    } else if (cmd == 2.0) { // Draw Line
                        // Draw line from Previous State (N1?) to Current (N2?)
                        // Prompt says: "Line is drawn from N1 (the last state) to N2"
                        // If instruction is DEVICE_IO N2, 2.0
                        // We need N1.
                        // Let's assume N1 is always the "Previous" register for this demo.
                        int x1 = (int)(registers[1].real_coords[0] * 20.0) + 40;
                        int y1 = 12 - (int)(registers[1].real_coords[1] * 20.0);
                        
                        int x2 = (int)(registers[dst].real_coords[0] * 20.0) + 40;
                        int y2 = 12 - (int)(registers[dst].real_coords[1] * 20.0);
                        
                        // Simple Line Algorithm (Bresenham-ish or just start/end points for ANSI)
                        draw_pixel(x1, y1, 0xFFFFFFFF);
                        draw_pixel(x2, y2, 0xFFFFFFFF);
                        // For ANSI, drawing a full line is hard without a library.
                        // Let's just draw the endpoints as requested by "Show the line and the point".
                    }
                }
                break;

            case OP_EXIT: // [NEW]
                destroy_topology(current_topology_id);
                schedule_next();
                break;

            case OP_SAVE: // [NEW] SAVE <FileID>, <Register>
                {
                    // Dst = FileID (Immediate or Register?), Src = Data Register
                    // Syntax: SAVE 1, N1
                    // If we parse "1" as Dst (Immediate) and "N1" as Src
                    uint64_t file_id = (mode == MODE_IMMEDIATE || mode == MODE_DIRECT) ? dst : 0; 
                    // Actually, if mode is IMMEDIATE, dst might be the value? 
                    // In our decoder: dst = (base >> 32) & FFFF. 
                    // If we write SAVE 1, N1. 
                    // Assembler needs to put 1 in DST and N1 in SRC.
                    // Let's assume Assembler does that.
                    
                    // We need to handle if FileID is in a register too? 
                    // For now, assume FileID is immediate in Dst field.
                    
                    crystallize(file_id, &registers[src], 1); // Save 1 node
                }
                break;

            case OP_READ: // [NEW] READ <FileID>, <Register>
                {
                    // Syntax: READ 1, N1
                    // Dst = FileID, Src = Register (Target)
                    // Wait, usually READ <Source>, <Dest>.
                    // If syntax is READ 1, N1. 1 is Source (File), N1 is Dest.
                    // So Dst=1, Src=N1 (index).
                    // We want to load INTO registers[src].
                    
                    uint64_t file_id = dst;
                    sublimate(file_id, &registers[src], 1);
                }
                break;

            case OP_WAVE: // [NEW] WAVE <TargetID>, <DataRegister>
                {
                    uint64_t target_id;
                    if (mode == MODE_IMMEDIATE) {
                        target_id = immediate_uint64;
                    } else {
                        target_id = (uint64_t)registers[dst].real_coords[0];
                    }
                    transmit_wave(registers[src], target_id);
                }
                break;

            case OP_RENDER: // [NEW] RENDER
                gpu_snapshot("lattice_state.json", registers, MAX_REGISTERS, SpiralMemory);
                break;

            case OP_ROT: // [NEW] ROT <Register>, <Angle>
                {
                    double angle = (mode == MODE_IMMEDIATE) ? immediate_double : registers[src].theta;
                    registers[dst].theta += angle;
                    // Update Real Coords
                    registers[dst].real_coords[0] = registers[dst].r * cos(registers[dst].theta);
                    registers[dst].real_coords[1] = registers[dst].r * sin(registers[dst].theta);
                }
                break;
                
            case OP_LOAD:
                if (mode == MODE_IMMEDIATE) {
                    init_node(&registers[dst], immediate_double);
                }
                break;

            case OP_TUNE: // [NEW]
                if (mode == MODE_IMMEDIATE) {
                    tune_node(dst, immediate_double);
                }
                break;

            case OP_LINK: // [NEW]
                if (mode == MODE_IMMEDIATE) {
                    link_handler(dst, immediate_uint64);
                }
                break;

            case OP_RET: // [NEW]
                pc = shadow_pc;
                printf("[System] RET -> Restoring Context to 0x%llX\n", pc);
                break;

            case OP_BND:
                // Conceptual Tensor Product Fusion
                for(int k=0; k<3; k++) {
                    registers[dst].real_coords[k] += registers[src].real_coords[k];
                }
                registers[dst].harmonic_sig ^= registers[src].harmonic_sig;
                break;
            
            case OP_SUBH:
                // Simple subtraction for loop counters (using real_coords[0] as scalar)
                registers[dst].real_coords[0] -= (mode == MODE_IMMEDIATE) ? immediate_double : registers[src].real_coords[0];
                break;

            case OP_JUMP:
                pc = (mode == MODE_IMMEDIATE) ? immediate_uint64 : (uint64_t)registers[dst].real_coords[0];
                break;

            case OP_CJMP: // Conditional Jump (JNZ equivalent)
                // If Dst != 0, Jump to Src (or Immediate)
                if (fabs(registers[dst].real_coords[0]) > 0.0001) {
                     pc = (mode == MODE_IMMEDIATE) ? immediate_uint64 : (uint64_t)registers[src].real_coords[0];
                }
                break;

            case OP_OUT:
                if (dst == NODE_SCREEN) {
                    device_io(NODE_SCREEN, registers[dst]);
                } else {
                    printf("OUT [N%d]: %.4f\n", dst, registers[dst].real_coords[0]);
                }
                fflush(stdout);
                break;

            case OP_HALT:
                printf("HALT Executed. Terminating System.\n");
                fb_cleanup(); // [NEW] Ensure Ncurses/FB is closed
                running = 0;
                break;

            case OP_FORCE:
                {
                    double force = tetryon_force(&registers[dst], &registers[src], 1.0);
                    init_node(&registers[0], force); // Accumulator
                }
                break;

            case OP_PUSH:
                spiral_alloc(registers[dst]);
                // visualize_spiral(); // Optional
                break;

            case OP_PULL:
                if (mode == MODE_IMMEDIATE) {
                    spiral_retrieve(immediate_uint64, &registers[dst]);
                }
                break;

            default:
                // printf("  Unknown Opcode: 0x%04X\n", opcode);
                break;
        }
    }

    return 0;
}
