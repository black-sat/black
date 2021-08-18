import os, os.path, sys, argparse
import csv 


# Function for checking if a string "s" is a number.
# taken from https://stackoverflow.com/questions/354038/how-do-i-check-if-a-string-is-a-number-float#354073
def is_number(s):
    try:
        float(s)
        return True
    except ValueError:
        return False



def main(argv):
    parser = argparse.ArgumentParser(description='Parser for check-results.py')
    parser.add_argument('datafile', metavar='datafile', 
                        nargs='?', default='_error_',
                        help='name of the data file')
    parser.add_argument('xtool', metavar='xtool', 
                        nargs='?', default='_error_',
                        help='name of the tool for the x-axis')
    parser.add_argument('ytool', metavar='ytool', 
                        nargs='?', default='_error_',
                        help='name of the tool for the y-axis')
    args = parser.parse_args()

    # number of column for xtool
    numcol_xtool    = -1
    # number of column for ytool
    numcol_ytool    = -1
    # index of the line in the benchmark file
    index_line      = 1
    # flag: 1 <=> there's at least one error
    at_least_one_bad = 0

    # opening the datafile 
    with open(args.datafile, mode ='r') as data_handle: 
        # read the datafile with the empty space as delimiter
        data_file = csv.reader(data_handle, delimiter=" ")
        # read the content of the datafile 
        header = True
        for line in data_file: 
            # remove empty elements
            line = [el for el in line if el!='']
            if header:
                # computing numcol_xtool and numcol_ytool
                colindex = 0
                for col in line:
                    if col == args.xtool and numcol_xtool == -1:
                        numcol_xtool = colindex
                    if col == args.ytool and numcol_ytool == -1:
                        numcol_ytool = colindex
                    colindex += 1
                header = False
            else:
                # take the two times
                time_xtool = line[numcol_xtool]
                time_ytool = line[numcol_ytool]
                # take the two results
                result_xtool = line[numcol_xtool+1]
                result_ytool = line[numcol_ytool+1]
                # if at least one between the two times is a timeout or an
                # error, than skip this benchmark. Otherwise, check if the
                # results coincide.
                if is_number(time_xtool) and is_number(time_ytool):
                    if result_xtool.strip() != result_ytool.strip():
                        at_least_one_bad = 1
                        print('The results of '+args.xtool+' and '+
                              args.ytool+' do not coincide on line '+
                              str(index_line)+'.')
            index_line+=1

        # the results are ok
        if not at_least_one_bad:
            print('All results of '+args.xtool+' and '+args.ytool+' coincide.')
    




if __name__ == "__main__":
   main(sys.argv[1:])
