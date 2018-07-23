#!/usr/bin/env python

import sys
import os
import subprocess
import shlex
import glob
import datetime

#COMMAND = "/home/futatsugi/develop/contests/icfpc/2018/icfpc2018/futatsugi/nmms"
COMMAND = "./nmms_parallel"

def perform(command, logf):
    print(command) #####
    p = subprocess.Popen(shlex.split(command), shell=False, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    for line in p.stdout:
        logf.write(line)
        logf.flush()

def main(args):
    if len(args) < 2:
        print("%s problem_id" % args[0])
        sys.exit(1)

    problem_id = int(args[1])

    problem_file = "./work/problemsF/FA%03d_tgt.mdl" % problem_id
    basename, ext = os.path.splitext(os.path.basename(problem_file))
    command = "%s %s %s" % (COMMAND, problem_file, "%s_nf.nbt" % basename)
    perform(command, sys.stdout)

if __name__ == "__main__":
    main(sys.argv)
