import os, sys, glob, subprocess
from joblib import Parallel, delayed

def invoke(filename):
    basename = os.path.basename(filename)
    subprocess.run([sys.argv[1], filename, basename.split('_')[0] + '.nbt'])
    sys.stderr.write('Finished: {}\n'.format(filename))
    sys.stderr.flush()

files = glob.glob(os.path.join(sys.argv[2], '*.mdl'))
Parallel(n_jobs=-1)([delayed(invoke)(fname) for fname in files])
