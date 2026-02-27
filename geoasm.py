import sys
import struct
import re

# Opcode Definitions (16-bit)
OPCODES = {
    'NOP':   0x0000,
    'NODE':  0x0001,
    'TUNE':  0x000A,
    'LINK':  0x000B,
    'RET':   0x000C,
    'GATE':  0x000D,
    'EXIT':  0x000E,
    'DEVICE_IO': 0x0005,
    'LOAD':  0x0010,
    'MOVH':  0x0011,
    'SAVE':  0x0012,
    'READ':  0x0013,
    'WAVE':  0x0014,
    'RENDER': 0x0015,
    'ROT':   0x0100,
    'REFL':  0x0101,
    'BND':   0x0110,
    'UNBND': 0x0111,
    'ADDH':  0x1000,
    'SUBH':  0x1001,
    'SUB':   0x1001, # Alias
    'JUMP':  0x1010,
    'CJMP':  0x1011,
    'JNZ':   0x1011, # Alias
    'IN':    0x1100,
    'OUT':   0x1101,
    'HALT':  0x1111,
    'FORCE': 0x2000,
    'PUSH':  0x2001,
    'PULL':  0x2002
}

# Mode Definitions (16-bit)
MODES = {
    'DIRECT':    0x0000,
    'IMMEDIATE': 0x0001,
    'HYPERBOLIC': 0x0010,
    'ANGLE':     0x0011,
    'AXIS_X':    0x0100,
    'AXIS_Y':    0x0101,
    'COMPOSITE': 0x0110,
    'SYSTEM':    0x1111
}

def parse_line(line):
    # Remove comments and whitespace
    line = line.split(';')[0].strip()
    if not line:
        return None
    
    # Check for Label
    label = None
    if ':' in line:
        parts = line.split(':')
        label = parts[0].strip()
        line = parts[1].strip()
        if not line:
            return {'label': label, 'type': 'label'}
    
    # Parse instruction
    parts = re.split(r'[,\s]+', line)
    mnemonic = parts[0].upper()
    
    if mnemonic not in OPCODES:
        raise ValueError(f"Unknown mnemonic: {mnemonic}")
    
    opcode = OPCODES[mnemonic]
    args = [p for p in parts[1:] if p]
    
    return {
        'type': 'instruction',
        'label': label,
        'mnemonic': mnemonic,
        'opcode': opcode,
        'args': args
    }

def get_instruction_size(instr):
    # Base size is 1 word (64-bit)
    # If immediate, add 1 word
    # We need to determine mode to know size
    # This is a bit circular, but we can infer mode from args
    
    mnemonic = instr['mnemonic']
    args = instr['args']
    
    # Simple heuristic for immediate mode
    is_immediate = False
    
    # Check for explicit immediate syntax
    for arg in args:
        if arg.startswith('#'):
            is_immediate = True
            break
            
    # Check for implicit immediate (Labels or Numbers without #)
    if not is_immediate:
        if mnemonic in ['JUMP', 'JNZ', 'CJMP', 'LINK']:
            # Second arg for JNZ/LINK, First for JUMP
            target = args[-1] 
            if not target.startswith('N'):
                is_immediate = True
        elif mnemonic in ['LOAD', 'TUNE']:
            # Second arg is value
            if len(args) >= 2:
                val = args[1]
                # If it looks like a number, it's immediate
                try:
                    float(val)
                    is_immediate = True
                except ValueError:
                    pass
        elif mnemonic in ['SUB', 'SUBH']:
             if len(args) >= 2 and not args[1].startswith('N'):
                 is_immediate = True
        elif mnemonic == 'GATE':
             # GATE <ID> is always immediate
             is_immediate = True

    return 2 if is_immediate else 1

def encode_instruction(instr, labels):
    mnemonic = instr['mnemonic']
    opcode = instr['opcode']
    args = instr['args']
    
    dst = 0
    src = 0
    mode = MODES['DIRECT']
    immediate_val = None
    immediate_type = None # 'float' or 'uint64'

    # Argument Parsing Logic
    if mnemonic in ['LOAD', 'TUNE']:
        if len(args) < 2: raise ValueError(f"Invalid args for {mnemonic}")
        dst = int(args[0][1:])
        val_str = args[1]
        
        # Try to parse as immediate float
        try:
            # Remove # if present
            clean_val = val_str.lstrip('#')
            immediate_val = float(clean_val)
            mode = MODES['IMMEDIATE']
            immediate_type = 'float'
        except ValueError:
            # Check if it is a label
            clean_val = val_str.lstrip('#')
            if clean_val in labels:
                 immediate_val = float(labels[clean_val])
                 mode = MODES['IMMEDIATE']
                 immediate_type = 'float'
            else:
                # Not a float, maybe a register?
                pass

    elif mnemonic in ['ROT']:
        # ROT N1, 0.5
        dst = int(args[0][1:])
        val_str = args[1]
        try:
            immediate_val = float(val_str)
            mode = MODES['IMMEDIATE']
            immediate_type = 'float'
        except ValueError:
            if val_str.startswith('N'):
                src = int(val_str[1:])
                mode = MODES['DIRECT']
            else:
                raise ValueError(f"Invalid arg for ROT: {val_str}")

    elif mnemonic in ['DEVICE_IO']:
        # DEVICE_IO N1, 1.0
        dst = int(args[0][1:])
        val_str = args[1]
        try:
            immediate_val = float(val_str)
            mode = MODES['IMMEDIATE']
            immediate_type = 'float'
        except ValueError:
             raise ValueError(f"Invalid arg for DEVICE_IO: {val_str}")

    elif mnemonic in ['GATE']:
        if len(args) < 1: raise ValueError(f"Invalid args for {mnemonic}")
        val_str = args[0]
        if val_str.startswith('N'):
             dst = int(val_str[1:])
             mode = MODES['DIRECT']
        else:
             mode = MODES['IMMEDIATE']
             immediate_val = int(val_str)
             immediate_type = 'uint64'

    elif mnemonic in ['EXIT']:
        mode = MODES['DIRECT']

    elif mnemonic in ['LINK']:

        if len(args) < 2: raise ValueError(f"Invalid args for {mnemonic}")
        dst = int(args[0][1:])
        target = args[1]
        mode = MODES['IMMEDIATE']
        immediate_type = 'uint64'
        if target in labels:
            immediate_val = labels[target]
        else:
            try:
                immediate_val = int(target)
            except:
                 raise ValueError(f"Unknown label: {target}")

    elif mnemonic in ['JUMP', 'JNZ', 'CJMP']:
        # JNZ N1, LABEL
        # JUMP LABEL
        if mnemonic == 'JUMP':
            target = args[0]
            mode = MODES['IMMEDIATE']
            immediate_type = 'uint64'
            if target in labels:
                immediate_val = labels[target]
            else:
                immediate_val = int(target)
        else: # JNZ/CJMP
            dst = int(args[0][1:])
            target = args[1]
            mode = MODES['IMMEDIATE']
            immediate_type = 'uint64'
            if target in labels:
                immediate_val = labels[target]
            else:
                immediate_val = int(target)

    elif mnemonic in ['SUB', 'SUBH']:
        # SUB N1, 1
        dst = int(args[0][1:])
        val_str = args[1]
        if val_str.startswith('N'):
            src = int(val_str[1:])
            mode = MODES['DIRECT']
        else:
            mode = MODES['IMMEDIATE']
            immediate_val = float(val_str)
            immediate_type = 'float'

    elif mnemonic in ['BND', 'FORCE']:
        dst = int(args[0][1:])
        src = int(args[1][1:])
        mode = MODES['COMPOSITE'] if mnemonic == 'BND' else MODES['DIRECT']

    elif mnemonic == 'OUT':
        dst = int(args[0][1:])
        mode = MODES['DIRECT']
        
    elif mnemonic == 'RET':
        mode = MODES['DIRECT']

    elif mnemonic == 'HALT':
        mode = MODES['DIRECT']

    elif mnemonic == 'RENDER':
        mode = MODES['DIRECT']

    elif mnemonic in ['SAVE', 'READ']:
        # SAVE 1, N1  -> Dst=1, Src=1
        # READ 1, N1  -> Dst=1, Src=1
        # Arg0 is FileID (Immediate usually), Arg1 is Register
        
        if len(args) < 2: raise ValueError(f"Invalid args for {mnemonic}")
        
        # Parse FileID (Arg0)
        file_id_str = args[0]
        try:
            dst = int(file_id_str) # Put FileID in Dst
        except ValueError:
            raise ValueError("File ID must be an integer constant for now")

        # Parse Register (Arg1)
        reg_str = args[1]
        if not reg_str.startswith('N'):
             raise ValueError("Second argument must be a register (e.g., N1)")
        src = int(reg_str[1:])
        
        mode = MODES['DIRECT'] # We use DIRECT as we packed FileID into Dst directly

    elif mnemonic == 'WAVE':
        # WAVE 1, N1  -> Dst=0, Src=1, Imm=1
        # WAVE N2, N1 -> Dst=2, Src=1, Mode=DIRECT
        
        if len(args) < 2: raise ValueError(f"Invalid args for {mnemonic}")
        
        target_str = args[0]
        if target_str.startswith('N'):
             dst = int(target_str[1:])
             mode = MODES['DIRECT']
        else:
             dst = 0 # Not used in Immediate
             mode = MODES['IMMEDIATE']
             immediate_val = int(target_str)
             immediate_type = 'uint64'
        
        src = int(args[1][1:])



    # Encoding
    base_word = struct.pack('>HHHH', opcode, dst, src, mode)
    extended_word = b''
    
    if mode == MODES['IMMEDIATE'] and immediate_val is not None:
        if immediate_type == 'float':
            extended_word = struct.pack('>d', float(immediate_val))
        elif immediate_type == 'uint64':
            extended_word = struct.pack('>Q', int(immediate_val))
            
    return base_word + extended_word

def banner():
    print(r"""
  _______   _                      
 |__   __| | |                     
    | | ___| |_ _ __ _   _  ___  _ __ 
    | |/ _ \ __| '__| | | |/ _ \| '_ \ 
    | |  __/ |_| |  | |_| | (_) | | | |
    |_|\___|\__|_|   \__, |\___/|_| |_|
                      __/ |            
                     |___/             
    """)
    print("GeoASM - Tetryon Geometric Assembler v1.0")
    print("=========================================")

def main():
    banner()
    if len(sys.argv) != 3:
        print("Usage: python geoasm.py <input.lat> <output.bin>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    try:
        with open(input_file, 'r') as f:
            lines = f.readlines()

        # Pass 1: Symbol Table
        labels = {}
        current_address = 0 # In Words (64-bit)
        parsed_lines = []

        for line_num, line in enumerate(lines):
            try:
                parsed = parse_line(line)
                if parsed:
                    if parsed['label']:
                        labels[parsed['label']] = current_address
                        # print(f"DEBUG: Label {parsed['label']} at {current_address}")
                    
                    if parsed['type'] == 'instruction':
                        size = get_instruction_size(parsed)
                        # print(f"DEBUG: Line {line_num+1} {parsed['mnemonic']} Size: {size}")
                        current_address += size
                        parsed_lines.append(parsed)
            except Exception as e:
                print(f"Error on line {line_num + 1}: {e}")
                sys.exit(1)

        # Pass 2: Code Generation
        with open(output_file, 'wb') as out:
            for instr in parsed_lines:
                try:
                    instruction_bytes = encode_instruction(instr, labels)
                    out.write(instruction_bytes)
                except Exception as e:
                    print(f"Error encoding {instr['mnemonic']}: {e}")
                    sys.exit(1)
                    
        print(f"Successfully assembled {input_file} to {output_file}")
        print(f"Program Size: {current_address} words")

    except FileNotFoundError:
        print(f"Error: File {input_file} not found.")
        sys.exit(1)

if __name__ == "__main__":
    main()

if __name__ == "__main__":
    main()
