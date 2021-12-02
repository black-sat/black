import os, os.path, sys, argparse

DIRNAME="scalable_2"

def main(argv):
    parser = argparse.ArgumentParser(description='Parser for generate.py')
    parser.add_argument('-s', '--start', dest='start', type=int,
                        default=0,
                        help='Starting index')
    parser.add_argument('-e', '--end', dest='end', type=int,
                        default=0,
                        help='Ending index')
    args = parser.parse_args()

    if int(args.start) > int(args.end):
        sys.exit("ERROR: -s must be <= than -e")

    for index in range(int(args.start),int(args.end)+1):
        filename = DIRNAME+"_"+str(index)+'.ltlfmt'
        with open(filename, mode='w') as data_handle: 
            data_handle.write('x=0 & ')
            acc = "next(x)"
            for x in range(0,index):
                acc = "next("+acc+")"
            data_handle.write(acc+'='+str(args.end)+' & ')
            data_handle.write('G(x<'+str(args.end)+')')
        with open(DIRNAME+'.index', mode='w') as data_handle:
            data_handle.write(filename+':UNSAT\n')



if __name__ == "__main__":
   main(sys.argv[1:])
