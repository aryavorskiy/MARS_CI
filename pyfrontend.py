import sys
import subprocess

buff = ""
args = {}
for arg in sys.argv:
    argsp = arg.split(sep = "=", maxsplit = 2)
    if len(argsp) == 2:
        args.update({arg.split(sep = "=", maxsplit = 2)[0].replace("-", "") : arg.split(sep = "=",maxsplit = 2)[1]})
if len(args) != 6:
    print("ERROR: Too {} arguments. Terminated.".format({True: "many", False: "few"}[len(args)>6]))
else:
    a = PIPE;
    a.write(args['start'])
    subprocess.run("./MARS_2", stdin=a)
