#!/usr/bin/env python

import sys
import os
import re
import struct
import numpy as np

def assemble(asm, outfile):
    outf = open(outfile, "wb")
    for line in asm:
        data = line.strip().split()
        cmd = data[0]
        if cmd == "SMove":
            xs = tuple(map(int, data[1:]))
            hi = 0
            lo = 0
            idx = 0
            for i in range(3):
                if xs[i] != 0:
                    idx = i
                    break
            hi |= ((idx+1) << 4)
            hi |= 0x04
            lo |= ((xs[idx] + 15) & 0x1f)
            outf.write(struct.pack("BB", hi, lo))
        elif cmd == "Fill":
            xs = tuple(map(int, data[1:]))
            nd = (xs[0] + 1) * 9 + (xs[1] + 1) * 3 + (xs[2] + 1)
            outf.write(struct.pack("B", (((nd << 3) & 0xf4) | 0x03)))
        elif cmd == "Flip":
            outf.write(struct.pack("B", 0xfd))
        elif cmd == "Wait":
            outf.write(struct.pack("B", 0xfe))
        elif cmd == "Halt":
            outf.write(struct.pack("B", 0xff))
        else:
            print("*** unknown")
            outf.write(struct.pack(">B", 0x00))
    outf.close()

def main(args):
    asm = open(args[1]).readlines()
    #R = struct.pack("B", data)
    #generate_code(asm, R, outfile)
    assemble(asm, args[2])

if __name__ == "__main__":
    main(sys.argv)
