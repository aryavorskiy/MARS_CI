#!/usr/bin/python3
import subprocess
import sys

req_args = ['start', 'end', 'step', 'lat_type', 'lat_arg', 'threads', 'block_data', 'block_qty', 'links', 'int_q',
            'results']
buff = ""
args = {}
for arg in sys.argv:
    argsp = arg.split(sep="=", maxsplit=2)
    if len(argsp) == 2:
        args.update({arg.split(sep="=", maxsplit=2)[0].replace("-", ""): arg.split(sep="=", maxsplit=2)[1]})

if len(args) < len(req_args):
    print("ERROR: Too few arguments.\nMissing arguments:")
    for arg in req_args:
        if list(args.keys()).count(arg) == 0:
            print(arg)
elif not set(req_args).issubset(args.keys()):
    for arg in req_args:
        if list(args.keys()).count(arg) == 0:
            print("Error: Missing argument definition for {}".format(arg))
else:
    fout = open(args.get('file', 'data'), 'w')
    proc = subprocess.Popen(['./MARS_CI'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                            universal_newlines=True)
    proc.stdout.readline()  # Skip description line
    for req_arg in req_args:
        print(proc.stdout.readline(), end="")
        proc.stdin.write(args[req_arg] + '\n')
        proc.stdin.flush()
    s = proc.stdout.readline()
    while s != '':
        fout.write(s)
        fout.flush()
        s = proc.stdout.readline()
