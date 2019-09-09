import sys
import subprocess

req_args = ['lambda', 'start', 'end', 'step', 'threads', 'points']
buff = ""
args = {}
for arg in sys.argv:
    argsp = arg.split(sep = "=", maxsplit = 2)
    if len(argsp) == 2:
        args.update({arg.split(sep = "=", maxsplit = 2)[0].replace("-", "") : arg.split(sep = "=",maxsplit = 2)[1]})
        
if len(args) != len(req_args):
    print("ERROR: Too {} arguments.\nRequired arguments:".format({True: "many", False: "few"}[len(args)>6]))
    for a in req_args:
        print(a)
elif not set(args.keys()).issubset(set(req_args)):
    for ab in req_args:
        if list(args.keys()).count(ab)==0:
            print("Error: Missing argument definition for {}".format(ab))
else:
    proc = subprocess.Popen(['./MARS_2'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)
    for req_arg in req_args:
        print(proc.stdout.readline(), end="")
        proc.stdin.write(args[req_arg]+'\n')
        proc.stdin.flush()
    s = proc.stdout.readline()
    while s != '':
        print(s, end="")
        s = proc.stdout.readline()
