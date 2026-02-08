#!/usr/bin/env python3
"""
Compile TI-BASIC launcher to C header file for ESP32
"""

import re

def parse_tibasic(source_path):
    """Parse TI-BASIC source file and return the code"""
    with open(source_path, 'r') as f:
        return f.read()

def tibasic_to_tokens(lines):
    """Convert TI-BASIC code to token bytes"""
    # TI-84 token table (subset for this program)
    tokens = {
        'ClrHome': b'\xE1',
        'ClrDraw': b'\x85',
        'Lbl ': b'\xD6',
        'Goto ': b'\xD7',
        'Menu(': b'\xE6',
        'If ': b'\xCE',
        'Then': b'\xCF',
        'End': b'\xD4',
        'Repeat ': b'\xD3',
        'Stop': b'\xD9',
        'Pause': b'\xD8',
        'Disp ': b'\xDE',
        'Output(': b'\xE7',
        'Input ': b'\xDC',
        'Prompt ': b'\xDD',
        'Send(': b'\xE5',
        'Get(': b'\xE2',
        'getKey': b'\xAD',
        'GridOff': b'\x74',
        'AxesOff': b'\x63',
        'RecallPic ': b'\x62',
        'Str0': b'\xAA\x00',
        'Str1': b'\xAA\x01',
        'Str2': b'\xAA\x02',
        'Pic1': b'\x60\x00',
        'max(': b'\xB9',
        '->': b'\x04',
        ':': b'\x3F',
        '\n': b'\x3F',
        '~': b'\xB0',  # negative sign
        '"': b'\x2A',
        ',': b'\x2B',
        '(': b'\x10',
        ')': b'\x11',
        '+': b'\x70',
        '-': b'\x71',
        '*': b'\x82',
        '/': b'\x83',
        '=': b'\x6A',
        '>': b'\x6C',
        '<': b'\x6B',
        '{': b'\x08',
        '}': b'\x09',
        ' ': b'\x29',
    }

    # Single letter variables
    for i, c in enumerate('ABCDEFGHIJKLMNOPQRSTUVWXYZ'):
        tokens[c + '->'] = bytes([0x41 + i]) + b'\x04'

    # Numbers
    for i in range(10):
        tokens[str(i)] = bytes([0x30 + i])

    result = bytearray()

    for line in lines:
        if not line.strip():
            continue

        i = 0
        while i < len(line):
            matched = False
            # Try longest tokens first
            for token, byte_val in sorted(tokens.items(), key=lambda x: -len(x[0])):
                if line[i:].startswith(token):
                    result.extend(byte_val)
                    i += len(token)
                    matched = True
                    break

            if not matched:
                # Single character
                c = line[i]
                if c.isupper():
                    result.append(0x41 + ord(c) - ord('A'))
                elif c.islower():
                    result.append(0x41 + ord(c) - ord('a'))
                elif c.isdigit():
                    result.append(0x30 + int(c))
                elif c == '\n' or c == ':':
                    result.append(0x3F)
                else:
                    # Unknown, skip
                    pass
                i += 1

        # End of line
        result.append(0x3F)

    return bytes(result)

def create_8xp(name, token_data):
    """Create a complete .8xp file structure"""
    # This is a simplified version - using the existing .8xp file structure
    # Header (55 bytes)
    header = bytearray(55)
    header[0:8] = b'**TI83F*'
    header[8:11] = b'\x1A\x0A\x00'
    # Comment (42 bytes, padded with zeros)
    comment = b'Created by TI-84 GPT Hack'
    header[11:11+len(comment)] = comment

    # Variable entry
    var_entry = bytearray()
    data_len = len(token_data) + 2  # +2 for the size word inside data
    var_entry.extend((data_len + 17).to_bytes(2, 'little'))  # data section length

    # Variable header (13 bytes)
    var_entry.extend(data_len.to_bytes(2, 'little'))
    var_entry.append(0x05)  # Program type
    var_entry.extend(name.upper().ljust(8, '\x00').encode('ascii')[:8])
    var_entry.append(0x00)  # Version
    var_entry.append(0x00)  # Archived flag
    var_entry.extend(data_len.to_bytes(2, 'little'))

    # Data
    var_entry.extend(len(token_data).to_bytes(2, 'little'))
    var_entry.extend(token_data)

    # Checksum
    checksum = sum(var_entry) & 0xFFFF
    var_entry.extend(checksum.to_bytes(2, 'little'))

    return bytes(header) + bytes(var_entry)

def main():
    import os

    source_file = '/Users/master/Downloads/TI-32-main 5/programs/LAUNCHER.8xp.txt'
    output_header = '/Users/master/Downloads/TI-32-main 5/esp32/launcher.h'

    # Read the existing launcher.h to get the actual compiled bytes
    # Since proper tokenization is complex, we'll use tivars if available
    try:
        from tivars import TIProgram
        from tivars.models import TI_84P

        # Read source
        code = parse_tibasic(source_file)

        # Create program
        prog = TIProgram(name="ANDYGPT")
        prog.load_string(code, model=TI_84P)

        # Get calc_data which is what the calculator expects (size + tokens)
        data = prog.calc_data

    except ImportError:
        print("tivars not installed, using manual tokenization")
        lines = parse_tibasic(source_file)
        token_data = tibasic_to_tokens(lines)
        data = create_8xp("ANDYGPT", token_data)

    # Create C header
    with open(output_header, 'w') as f:
        f.write('// Auto-generated launcher program\n')
        f.write('// Program name: ANDYGPT\n\n')
        f.write(f'unsigned int __launcher_var_len = {len(data)};\n')
        f.write('unsigned char __launcher_var[] = {\n')

        for i in range(0, len(data), 16):
            chunk = data[i:i+16]
            hex_vals = ', '.join(f'0x{b:02X}' for b in chunk)
            f.write(f'    {hex_vals},\n')

        f.write('};\n')

    print(f"Generated {output_header} ({len(data)} bytes)")

if __name__ == '__main__':
    main()
