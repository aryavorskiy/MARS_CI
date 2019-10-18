#!/usr/bin/python3
import subprocess
import sys

# These lines can be edited
required_args = ['start', 'end', 'step', 'lat_type', 'lat_arg', 'threads', 'block_data', 'block_qty', 'links', 'int_q',
                 'results']
program_filename = "./cmake-build-release/MARS_CI"
question_suffix = '?'


def parse_text_for_args(text, args_dict):
    split_text = text.split(sep="=", maxsplit=2)
    if len(split_text) == 2:
        args_dict.update({split_text[0].replace("-", ""): split_text[1]})


buff = ""
acquired_args = {}
for cli_arg in sys.argv:
    parse_text_for_args(cli_arg, acquired_args)

try:
    filename = acquired_args['config']
    print("Message: 'config' argument detected, reading file '{}'...".format(filename))
    conf_reader = open(filename)
    for line in conf_reader:
        for word in line.split():
            parse_text_for_args(word, acquired_args)
    print("Done reading, new config values will overwrite old ones if any")
except FileNotFoundError:
    print("Error: No config file found where specified, fallback")
except KeyError:
    pass

if not set(required_args).issubset(acquired_args.keys()):
    for arg in required_args:
        if list(acquired_args.keys()).count(arg) == 0:
            print("Error: Missing argument definition for {}".format(arg))
else:
    file_writer = open(acquired_args.get('file', 'data'), 'w')
    proc = subprocess.Popen([program_filename], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                            universal_newlines=True)
    print("Started writing parameters to program. Awaiting line ending with suffix '{}'...".format(question_suffix))
    for req_arg in required_args:
        new_line = proc.stdout.readline()
        print('>> ' + new_line, end='')
        while not new_line[:-1].endswith(question_suffix):
            new_line = proc.stdout.readline()
            print('>> ' + new_line, end='')
        print("Line ends with suffix '{}', appending parameter {}={}\n".format(question_suffix, req_arg,
                                                                               acquired_args[req_arg]))
        proc.stdin.write(acquired_args[req_arg] + '\n')
        proc.stdin.flush()
    print("All parameters written, writing program's stdout to file '{}'".format(
        acquired_args.get('file', 'data')))
    s = proc.stdout.readline()
    while s != '':
        file_writer.write(s)
        file_writer.flush()
        s = proc.stdout.readline()