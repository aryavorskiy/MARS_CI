#!/usr/bin/python3
import subprocess
import sys

# These lines can be edited
req_args = ['start', 'end', 'step', 'lat_type', 'lat_arg', 'threads', 'block_data', 'block_qty', 'links', 'int_q',
            'results']
program_filename = "./cmake-build-debug/MARS_CI"


def parse_text_for_args(text, args):
    splitted = text.split(sep="=", maxsplit=2)
    if len(splitted) == 2:
        args.update({splitted[0].replace("-", ""): splitted[1]})


buff = ""
args = {}
for cli_arg in sys.argv:
    parse_text_for_args(cli_arg, args)

try:
    fname = args['config']
    print("Message: 'config' argument detected, reading file '{}'...".format(fname))
    conf_reader = open(fname)
    for line in conf_reader:
        for word in line.split():
            parse_text_for_args(word, args)
    print("Done reading, new config values will overwrite old ones if any")
except FileNotFoundError:
    print("Error: No config file found where specified, fallback")
except KeyError:
    pass

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
    proc = subprocess.Popen([program_filename], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
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
