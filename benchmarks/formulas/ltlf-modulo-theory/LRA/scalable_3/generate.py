import os, os.path, sys, argparse

DIRNAME="scalable_3"

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
            data_handle.write('x1 > 1 & ')
            for i in range(2,index+1):
                for j in range(1,i-1):
                    data_handle.write('X ')
                data_handle.write('(next(x'+str(i)+')=x'+str(i-1)+'+1) & ')
            data_handle.write('G(');
            for i in range(1,index+1):
                data_handle.write('wnext(x'+str(i)+')=x'+str(i))
                data_handle.write(' & ' if i < index else '')
            data_handle.write(') & F(')
            for i in range(1,index+1):
                data_handle.write('x'+str(i))
                data_handle.write('+' if i < index else '')
            data_handle.write(' = ('+str(index)+'*'+str(index)+'/2)')
            data_handle.write(')')



if __name__ == "__main__":
   main(sys.argv[1:])
