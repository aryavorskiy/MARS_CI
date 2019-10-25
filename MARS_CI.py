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


def extract_args(text: str, eval_args: dict):
    for word in text.split():
        split_text = word.split(sep="=", maxsplit=2)
        if len(split_text) == 2:
            eval_args.update({split_text[0].replace("-", ""): split_text[1]})


def load_config(filename: str, cli_args: dict):
    args_list = []
    conf_args = {}
    try:
        conf_reader = open(filename)
        for line in conf_reader:
            if line.startswith('config_end'):
                conf_repeat = 1
                if len(line) > 10:
                    try:
                        conf_repeat = int(line.replace(' ', '')[11:-1])
                    except ValueError:
                        pass  # Repeat number not specified
                conf_args.update(cli_args)
                args_list.extend([conf_args] * conf_repeat)
                conf_args = {}
            else:
                extract_args(line, conf_args)
        conf_args.update(cli_args)
        args_list.extend([conf_args])
    except FileNotFoundError:
        pass
    return args_list


def evaluate_config(eval_args):
    file_writer = open(eval_args.get('file', 'data'), eval_args['write_mode'])  # Write mode can be passed as argument
    proc = subprocess.Popen([program_filename], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                            universal_newlines=True)
    print('Started writing parameters to program. Awaiting line ending with suffix \'{}\'...'.format(question_suffix))
    for req_arg in required_args:
        new_line = proc.stdout.readline()
        print('>> ' + new_line, end='')
        while not new_line[:-1].endswith(question_suffix):
            new_line = proc.stdout.readline()
            print('>> ' + new_line, end='')
        print('Line ends with suffix \'{}\', appending parameter {}={}'.format(question_suffix, req_arg,
                                                                               eval_args[req_arg]))
        proc.stdin.write(eval_args[req_arg] + '\n')
        proc.stdin.flush()
    print('\nAll parameters written, writing program\'s stdout to file \'{}\', duplicating here:'.format(
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
    finish_msg = '[Program finished working at {}; {} elapsed]'.format(
        time.ctime(), dt.timedelta(seconds=int(time.time() - start_time)))
    file_writer.write('\n{:-^90}'.format(finish_msg))
    print(finish_msg[1:-1] + '\n')


def check_args(args, verbose=True):
    ret = True
    for arg in required_args:
        if list(args.keys()).count(arg) == 0:
            if verbose:
                print('Error: Missing argument definition for \'{}\''.format(arg))
            ret = False
    return ret


cli_args = {'write_mode': 'a+'}
final_args = []
for cli_arg in sys.argv:
    extract_args(cli_arg, cli_args)

if 'session' in cli_args.keys():
    print('Message: \'session\' argument detected, trying to read file \'{}\'...'.format(cli_args['session']))
    try:
        open(cli_args['session'])
        print('Message: Session config loaded, starting...')
        for conf_file in open(cli_args['session']):
            print('Message[Session]: Loading config \'{}\'... '.format(conf_file[:-1]), end='')
            conf_args = load_config(conf_file[:-1], cli_args)
            if not conf_args:
                print('Failed\nError[Session]: Config missing or without parameters, skipping')
            else:
                print('Success')
                for subconf_args in conf_args:
                    print('\nLoading subconf #{}... '.format(1 + conf_args.index(subconf_args)), end='')
                    if check_args(subconf_args, verbose=False):
                        print('Success')
                        evaluate_config(subconf_args)
                    else:
                        print('Failed\nError: One or more parameter definitions missing:')
                        check_args(subconf_args)
    except FileNotFoundError:
        print('Error: No session file found where specified, fallback')
    print('Message: Session complete, exiting')
    exit()

if 'config' in cli_args.keys():
    print('Message: \'config\' argument detected, trying to read file \'{}\'...'.format(cli_args['config']))
    final_args = load_config(cli_args['config'], cli_args)
    if not final_args:
        print('Error: No config file found where specified, fallback')

for subconf_args in final_args:
    print('\nLoading subconf #{}... '.format(1 + final_args.index(subconf_args)), end='')
    if check_args(subconf_args, verbose=False):
        print('Success')
        evaluate_config(subconf_args)
    else:
        print('Failed\nError: One or more parameter definitions missing:')
        check_args(subconf_args)
print('Message: Session complete, exiting')
