#!/usr/bin/python3
import datetime as dt
import subprocess
import sys
import time

# These lines can be edited
required_args = ['start', 'end', 'step', 'lat_type', 'lat_arg', 'threads', 'block_data', 'block_qty', 'links', 'int_q',
                 'results']
program_filename = './cmake-build-release/MARS_CI'
question_suffix = '?'


def extract_args(text, eval_args):
    split_text = text.split(sep="=", maxsplit=2)
    if len(split_text) == 2:
        eval_args.update({split_text[0].replace("-", ""): split_text[1]})


def load_config(filename):
    conf_args = {}
    try:
        conf_reader = open(filename)
        for line in conf_reader:
            for word in line.split():
                extract_args(word, conf_args)
        return conf_args
    except FileNotFoundError:
        return {}


def evaluate_config(eval_args):
    file_writer = open(eval_args.get('file', 'data'), 'w')
    proc = subprocess.Popen([program_filename], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                            universal_newlines=True)
    print('Started writing parameters to program. Awaiting line ending with suffix \'{}\'...'.format(question_suffix))
    for req_arg in required_args:
        new_line = proc.stdout.readline()
        print('>> ' + new_line, end='')
        while not new_line[:-1].endswith(question_suffix):
            new_line = proc.stdout.readline()
            print('>> ' + new_line, end='')
        print('Line ends with suffix \'{}\', appending parameter {}={}\n'.format(question_suffix, req_arg,
                                                                                 eval_args[req_arg]))
        proc.stdin.write(eval_args[req_arg] + '\n')
        proc.stdin.flush()
    print('All parameters written, writing program\'s stdout to file \'{}\', duplicating here:'.format(
        eval_args.get('file', 'data')))
    start_time = time.time()
    start_msg = '[Program started at {}]'.format(time.ctime())
    file_writer.write('{:-^90}\n\n'.format(start_msg))
    print(start_msg[1:-1])
    while proc.poll() is None:
        new_line = proc.stdout.readline()
        if new_line != '':
            file_writer.write(new_line)
            print('>> ' + new_line, end='')
        file_writer.flush()
        s = proc.stdout.readline()
    finish_msg = '[Program finished working at {}; {} elapsed]'.format(
        time.ctime(), dt.timedelta(seconds=int(time.time() - start_time)))
    file_writer.write('\n{:-^90}'.format(finish_msg))
    print(finish_msg[1:-1] + '\n')


def check_args(args, verbose=True):
    ret = True
    for arg in required_args:
        if list(args.keys()).count(arg) == 0:
            if verbose:
                print('Error: Missing argument definition for {}'.format(arg))
            ret = False
    return ret


buff = ""
cli_args = {}
final_args = {}
for cli_arg in sys.argv:
    extract_args(cli_arg, cli_args)

if 'session' in cli_args.keys():
    print('Message: \'session\' argument detected, trying to read file \'{}\'...'.format(cli_args['session']))
    try:
        open(cli_args['session'])
        print('Message: Session config loaded, starting...')
        for conf_file in open(cli_args['session']):
            print('Message[Session]: Loading config \'{}\'... '.format(conf_file[:-1]), end='')
            conf_args = load_config(conf_file[:-1])
            conf_args.update(cli_args)
            if not conf_args:
                print('Failed\nError[Session]: Config missing or without parameters, skipping')
            elif check_args(conf_args, verbose=False):
                print('Success')
                evaluate_config(conf_args)
            else:
                print('Failed\nError: One or more parameter definitions missing:')
                check_args(conf_args)
    except FileNotFoundError:
        print('Error: No session file found where specified, fallback')
    print('Message: Session complete, exiting')
    exit()

if 'config' in cli_args.keys():
    print('Message: \'config\' argument detected, trying to read file \'{}\'...'.format(cli_args['config']))
    final_args = load_config(cli_args['config'])
    if not final_args:
        print('Error: No config file found where specified, fallback')

final_args.update(cli_args)
if check_args(final_args):
    evaluate_config(final_args)
else:
    print('Error: Argument definitions missing, exiting')
