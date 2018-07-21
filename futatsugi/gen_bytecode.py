#!/usr/bin/env python

import sys
import os
import re
import struct
import numpy as np

def generate_code(asm, R, outfile):
    print("""#include "../nbtasm/nbtasm.hpp"
    int main(){""")

    print("State s(%d, 20);" % R)
    for line in asm:
        data = line.strip().split()
        cmd = data[0]
        if cmd == "SMove":
            xs = tuple(map(int, data[1:]))
            print("s.bots(0).smove(Vec3{ %d, %d, %d });" % xs)
            print("s.commit();")
        elif cmd == "Fill":
            xs = tuple(map(int, data[1:]))
            print("s.bots(0).fill(Vec3{ %d, %d, %d });" % xs)
            print("s.commit();")
        elif cmd == "Flip":
            print("s.bots(0).flip();")
            print("s.commit();")
        elif cmd == "Halt":
            print("s.bots(0).halt();")
            print("s.commit();")
        else:
            print("*** unknown")
    print("""s.export_trace("%s.nbt");
	return 0;
    }""" % outfile)

def main(args):
    asm = open(args[1]).readlines()
    fmodel = open(args[2], "rb")
    outfile = args[3]
    data = fmodel.read(1)
    R = struct.unpack("B", data)
    generate_code(asm, R, outfile)

if __name__ == "__main__":
    main(sys.argv)
