import os, sys, glob, subprocess
from joblib import Parallel, delayed

def invoke(filename):
    subprocess.run([sys.argv[1], filename])
    sys.stderr.write('Finished: {}\n'.format(filename))

files = glob.glob(os.path.join(sys.argv[2], '*.mdl'))
Parallel(n_jobs=-1)([delayed(invoke)(fname) for fname in files])
