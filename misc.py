# functions for reuse, for image processing, etc.
import os
import sys
import copy
import math
import struct
import numpy as np
import os.path as path
import matplotlib.pyplot as plt

args = sys.argv

# print message and exit
def err(c):
    print('Error: ' + c)
    sys.exit(1)

def run(c):
    print 'run("' + str(c) + '")'
    a = os.system(c)
    if a != 0:
        err("command failed to run:\n\t" + c)
    return a

def exists(f):
    return os.path.exists(f)

def hdr_fn(bin_fn):
    # return filename for header file, given name for bin file
    hfn = bin_fn[:-4] + '.hdr'
    if not exists(hfn):
        hfn2 = bin_fn + '.hdr'
        if not exists(hfn2):
            err("didn't find header file at: " + hfn + " or: " + hfn2)
        return hfn2
    return hfn

def read_hdr(hdr):
    samples, lines, bands = 0, 0, 0
    for line in open(hdr).readlines():
        words = line.split('=')
        if len(words) == 2:
            for f in ['samples', 'lines', 'bands']:
                if words[0].strip() == f:
                    exec(f + ' = int(' + words[1].strip() + ')')
    return samples, lines, bands

# require a filename, or list of filenames, to exist
def assert_exists(fn):
    try:
        if type(fn) != str:
            iterator = iter(fn)
            for f in fn:
                assert_exists(f)
            return
    except:
        # not iterable
        pass

    if not exists(fn):
        err("couldn't find required file: " + fn)

# use numpy to read a floating-point data file (4 bytes per float, byte order 0)
def read_float(fn):
    print "+r", fn
    return np.fromfile(fn, '<f4')

def wopen(fn):
    f = open(fn, "wb")
    if not f:
        err("failed to open file for writing: " + fn)
    print "+w", fn
    return f

def read_binary(fn):
    hdr = hdr_fn(fn)

    # read header and print parameters
    samples, lines, bands = read_hdr(hdr)
    print "header," + hdr
    for f in ['samples', 'lines', 'bands']:
        exec('print("\t' + f + '," + str(' +  f + '));')

    data = read_float(fn)
    return samples, lines, bands, data

def write_binary(np_ndarray, fn):
    of = wopen(fn)
    np_ndarray.tofile(of, '', '<f4')
    of.close()
