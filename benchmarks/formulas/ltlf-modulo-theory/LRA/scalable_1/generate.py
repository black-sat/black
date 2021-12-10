import os, os.path, sys, argparse

DIRNAME="scalable_1"

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
        sys.exit("-s must be <= than -e")

    for index in range(int(args.start),int(args.end)+1):
        filename = DIRNAME+"_"+str(index)+'.ltlfmt'
        with open(filename, mode='w') as data_handle: 
            data_handle.write('x=0 & ')
            data_handle.write('G(wnext(x)=x+1) & ')
            data_handle.write('F(x='+str(index)+')')
        with open(DIRNAME+'.index', mode='w') as data_handle:
            data_handle.write(filename+':SAT\n')



if __name__ == "__main__":
   main(sys.argv[1:])
