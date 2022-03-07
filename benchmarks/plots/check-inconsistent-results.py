import os, os.path, sys, argparse
import csv


def main(argv):
    parser = argparse.ArgumentParser(description='Check if there are inconsistent results')
    parser.add_argument('datafile', metavar='datafile', 
                        nargs='?', default='_error_',
                        help='Name of the data file')
    args = parser.parse_args()
    
    # check on the options
    if not os.path.exists(args.datafile) or args.datafile=='_error_':
        sys.exit('Please specify the datafile')

    #datafile's name without path
    datafile_name = args.datafile.split('/')[-1]

    exists_inconsistent = False

    # opening the datafile 
    with open(args.datafile, mode ='r') as data_handle: 
        # read the datafile with the empty space as delimiter
        data_file = csv.DictReader(data_handle, delimiter=" ")
        for line in data_file: 
            # take family name
            family = line['family'].strip()
            
            # check results consistency
            is_sat = False
            is_unsat = False
            for col in line:
                if ":result" in col:
                    if line[col].strip() == 'SAT':
                        is_sat = True
                    if line[col].strip() == 'UNSAT':
                        is_unsat = True

            if is_sat and is_unsat:
                if not exists_inconsistent:
                    print("The following inconsistencies have been found:")
                    exists_inconsistent = True
                print(" ".join(line.values()))

    if not exists_inconsistent:
        print("No inconsitencies found.")


if __name__ == "__main__":
   main(sys.argv[1:])
