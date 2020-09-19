#!/usr/bin/env python3
import sys
import os 

key = 0xff

# utils

def pack(elf, size, key):
    packed_content = bytearray(size)
    for i in range(size):
        packed_content[i] = elf[i] ^ key
    return packed_content
 

# main

if len(sys.argv) != 2:
    print("Usage: {} <path to elf>".format(sys.argv[0]))
    exit(0)

try:
    path = os.path.abspath(sys.argv[1])
    elf_buf = bytearray(open(path, 'rb').read())
    elf_len = len(elf_buf)
    packed = pack(elf_buf, elf_len, key)

    print("Packing {}".format(path))

    open(path + '.packed', 'wb').write(packed)

except FileNotFoundError:
    print("Failed. Cannot pack")


