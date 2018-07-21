#!/usr/bin/env python

import sys
import os
import re
import numpy as np

def optimize(asm):
    cmds = []
    cnt = 0
    for i in range(len(asm) - 1):
        if asm[i] == asm[i+1]:
            a = list(map(int, asm[i].split()[1:]))
            for j in range(3):
                if a[j] != 0:
                    sign = a[j]
                    coord = j
                    break
            cnt += 1 + (1 if cnt == 0 else 0)
        elif cnt > 0:
            data = [0, 0, 0]
            while cnt > 0:
                if cnt >= 15:
                    data[coord] = sign * 15
                    cnt -= 15
                else:
                    data[coord] = sign * cnt
                    cnt = 0
                cmds.append("SMove %d %d %d" % tuple(data))
            """
            data[coord] = sign * cnt
            cmds.append("smove %d %d %d" % tuple(data))
            """
            cnt = 0
        else:
            cmds.append(asm[i].strip())
    if cnt > 0:
        cmds.append("cnt %d" % cnt)
    else:
        cmds.append(asm[-1].strip())
    return cmds

def main(args):
    asm = open(args[1]).readlines()
    opt_asm = optimize(asm)
    #print(opt_asm)
    for line in opt_asm:
        print(line)

if __name__ == "__main__":
    main(sys.argv)
