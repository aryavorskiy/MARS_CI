#!/usr/bin/python3
import datetime as dt
import shutil
import subprocess
import sys
import time

#############################################################################
# Universal Python frontend by aryavorskiy                                  #
# INFO:                                                                     #
# A parameter is a single value of many that the program needs for its work #
# A config is a set of parameters enough for the program to work            #
# A session is a single run of the program with a specific config           #
#############################################################################

# These lines can be edited
REQUIRED_PARAMS = ['start', 'end', 'step', 'lat_type', 'lat_arg', 'threads', 'block_data', 'block_qty',
                   'results']  # Aliases of the program's run parameters
PROGRAM_FILENAME = './cmake-build-release/MARS_CI'  # Path to launch the program
QUESTION_SUFFIX = '?'  # If a line of the program's stdout ends with this, a new parameter is written to its stdin


class ConsoleGoodies:
    """These constants make the output beautiful"""
    TERM_WIDTH = 80 if not shutil.get_terminal_size((80, 25)).columns else shutil.get_terminal_size((80, 25)).columns
    PROGRAM_OUTPUT = '\033[1m\033[95m>>\033[0m'
    PROGRAM_INPUT = '\033[1m\033[96m<<\033[0m'
    MESSAGE = '\033[94mMessage:\033[0m'
    LINE = '\033[93m' + '-' * TERM_WIDTH + '\033[0m'
    ERROR = '\033[91mError:\033[0m'
    FAIL = '\033[91mFailed\033[0m'


def extract_args_from_line(text: str, params: dict) -> None:
    """Parses needed parameters from given text and stores it in a given dictionary"""
    for word in text.split():
        split_text = word.split(sep="=", maxsplit=2)
        if len(split_text) == 2:
            params.update({split_text[0].replace("-", ""): split_text[1]})


def load_config(filename: str, override_args: dict) -> list:
    """Loads session configs from given file to a list which is returned. Parameters are represented as a dictionary"""
    sessions_params = []
    session_parameters = {}
    try:
        config_reader = open(filename)
        for line in config_reader:
            line = line.replace('\n', '')
            if line.startswith('config_end') or line.startswith('session_end'):
                session_repeat = 1
                try:
                    session_repeat = int(line.split()[2])
                except (ValueError, IndexError):
                    pass  # Repeat number not specified
                session_parameters.update(override_args)
                sessions_params.extend([session_parameters] * session_repeat)
                session_parameters = {}
            else:
                extract_args_from_line(line, session_parameters)
        session_parameters.update(override_args)
        sessions_params.extend([session_parameters])
    except FileNotFoundError:
        pass  # No session config file at given location
    return sessions_params


def check_args(args: dict, verbose: bool = True) -> bool:
    """Checks if all required arguments are present in config"""
    ret = True
    for arg in REQUIRED_PARAMS:
        if list(args.keys()).count(arg) == 0:
            if verbose:
                print(ConsoleGoodies.ERROR + 'Missing parameter definition for \'{}\''.format(arg))
            ret = False
    return ret


def process_load_args(process: subprocess.Popen, params_to_load: dict) -> None:
    """Writes the config's parameters to the process' stdin. Returns when all arguments are loaded"""
    print('Started loading parameters to program. Awaiting line ending with suffix \'{}\'...'.format(QUESTION_SUFFIX))
    print()
    print(ConsoleGoodies.LINE)
    for req_arg in REQUIRED_PARAMS:
        new_line = process.stdout.readline()
        print(ConsoleGoodies.PROGRAM_OUTPUT, new_line, end='')
        while not new_line[:-1].endswith(QUESTION_SUFFIX):
            new_line = process.stdout.readline()
            print(ConsoleGoodies.PROGRAM_OUTPUT, new_line, end='')
        print(ConsoleGoodies.PROGRAM_INPUT, params_to_load[req_arg], '[{}]'.format(req_arg))
        process.stdin.write(params_to_load[req_arg] + '\n')
        process.stdin.flush()
    print(ConsoleGoodies.LINE)


def process_listen(process: subprocess.Popen, file_writer=None) -> None:
    """Reads the process' stdout and writes it into a file. Returns when the process terminates"""
    start_time = time.time()
    start_msg = '[Program started at {}]'.format(time.ctime())
    if file_writer is not None:
        file_writer.write('{:-^90}\n\n'.format(start_msg))
    print(start_msg[1:-1])
    print()
    print(ConsoleGoodies.LINE)
    while process.poll() is None:  # Repeat while process is alive
        new_line = process.stdout.readline()
        if new_line != '':
            if file_writer is not None:
                file_writer.write(new_line)
            print(ConsoleGoodies.PROGRAM_OUTPUT, new_line, end='')
        if file_writer is not None:
            file_writer.flush()
    finish_msg = '[Program finished working at {}; {} elapsed]'.format(
        time.ctime(), dt.timedelta(seconds=int(time.time() - start_time)))
    if file_writer is not None:
        file_writer.write('\n{:-^90}\n\n'.format(finish_msg))
        file_writer.flush()
    print(ConsoleGoodies.LINE)
    print()
    print(finish_msg[1:-1])


def session(session_params: dict) -> None:
    """Launches the program, loads given parameters, captures its output and returns"""
    print('Ensuring that all parameters are defined... ', end='')
    if not check_args(session_params, verbose=False):  # Inconsistent config
        print(ConsoleGoodies.FAIL)
        check_args(session_params)
        print('Session aborted\n')
        return
    print('Check')
    filename = session_params.get('file', 'data')
    file_writer = open(filename, session_params.get('write_mode', 'a+'))
    proc = subprocess.Popen([PROGRAM_FILENAME], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                            universal_newlines=True)
    process_load_args(proc, session_params)
    print('\nAll parameters loaded, writing program\'s stdout to file \'{}\', duplicating here:'.format(filename))
    process_listen(proc, file_writer)
    print()


cli_args = {}
sessions = []
for cli_arg in sys.argv:
    extract_args_from_line(cli_arg, cli_args)

if 'config' in cli_args.keys():
    print(ConsoleGoodies.MESSAGE, '\'config\' argument detected, trying to read file \'{}\'...'.format(
        cli_args['config']))
    sessions = load_config(cli_args['config'], cli_args)
    if not sessions:
        print(ConsoleGoodies.ERROR, 'No config file found where specified, fallback')

if not sessions:
    print(ConsoleGoodies.MESSAGE, 'only CLI arguments will be loaded as parameters')
    session(cli_args)
else:
    for session_config in sessions:
        print('\nLaunched session #{}.'.format(1 + sessions.index(session_config)))
        session(session_config)
print(ConsoleGoodies.MESSAGE, 'All sessions complete, exiting')
