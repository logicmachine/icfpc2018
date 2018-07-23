#!/usr/bin/env python

import sys
import os
import subprocess
import shlex
import glob
import datetime

#SUBMIT_DIR = "/home/futatsugi/develop/contests/icfpc/2018/icfpc2018/futatsugi/work/submission01"
SUBMIT_DIR = "./submission01"
#PROBLEM_DIR = "/home/futatsugi/develop/contests/icfpc/2018/icfpc2018/futatsugi/work/problemsF"
PROBLEM_DIR = "./work/problemsF"
#COMMAND = "/home/futatsugi/develop/contests/icfpc/2018/icfpc2018/futatsugi/nmms"
COMMAND = "./nmms"

def perform(command, logf):
    print(command) #####
    p = subprocess.Popen(shlex.split(command), shell=False, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    for line in p.stdout:
        logf.write(line)
        logf.flush()

def main(args):
    FA_files = glob.glob(os.path.join(PROBLEM_DIR, "FA*_tgt.mdl"))

    logf = open("run_nmms_%s.log" % datetime.datetime.now().strftime("%Y%m%d%H%M"), "wb")
    for fname in FA_files:
        basename, ext = os.path.splitext(os.path.basename(fname))
        command = "%s %s %s" % (COMMAND, fname, os.path.join(SUBMIT_DIR, "%s_nf.nbt" % basename))
        perform(command, logf)
    logf.close()

if __name__ == "__main__":
    main(sys.argv)
