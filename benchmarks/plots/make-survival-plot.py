import os, os.path, sys, argparse
import csv 
from plotly.subplots import make_subplots
import plotly.graph_objects as go
import numpy as np



# Function for checking if a string "s" is a number.
# taken from https://stackoverflow.com/questions/354038/how-do-i-check-if-a-string-is-a-number-float#354073
def is_number(s):
    try:
        float(s)
        return True
    except ValueError:
        return False



def main(argv):
    parser = argparse.ArgumentParser(description='Parser for make-survival-plot.py')
    parser.add_argument('datafile', metavar='datafile', 
                        nargs='?', default='_error_',
                        help='Name of the data file')
    parser.add_argument('tools', metavar='tools', 
                        nargs='?', default='_error_',
                        help='List of comma-separated names of the tools.')
    parser.add_argument('timeout', metavar='timeout', type=int,
                        default=0,
                        help='Value for the timeout.')
    parser.add_argument('-p', '--png', dest='pngopt', 
                        action='store_true', default=0,
                        help='Dumps the png file with the plot.')
    parser.add_argument('-m', '--html', dest='htmlopt',
                        action='store_true', default=0,
                        help='Opens the browser with the interactive plot.')
    parser.add_argument('-t', '--threshold', dest='threshold',
                        default=0,
                        help='Threshold value (= maximum value for the x-axis).')
    parser.add_argument('-i', '--tiks', dest='tiks',
                        default=0,
                        help='Number of tiks of the x-axis.')
    args = parser.parse_args()


    
    # check on the options
    if not os.path.exists(args.datafile) or args.datafile=='_error_':
        sys.exit('Please specify the datafile')
    if args.tools=="_error_":
        sys.exit('Please specify the tools')
    if args.timeout==0:
        sys.exit('Please specify the timeout value')

    # create list with tool's name
    toolsnames=[]
    for toolname in args.tools.split(','):
        toolsnames += [toolname.strip()]
    #datafile's name without path
    datafile_name = args.datafile.split('/')[-1]
    #path for image
    img_path_name = "survivalplot"+datafile_name.replace(".csv",".png").replace(".dat",".png")
    # number of column for tools: it's a dictionary:   name => numcol
    toolsnumcol={}
    # number of benchmarks
    total_bench_counter = 0
    # tools'times : it is a dictionary:  name => list of times
    toolstimes={}
    # tools'max times
    toolsmaxtimes={}


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
                for toolname in toolsnames:
                    # initializing toolsmaxtimes
                    toolsmaxtimes[toolname]=0
                    # initializing toolsmaxtimes
                    toolstimes[toolname]=[]
                    colindex = 0
                    # computing toolsnumcol
                    for col in line:
                        if toolname == col:
                            toolsnumcol[toolname] = colindex
                        colindex += 1
                header = False
            else:
                # increment 'total_bench_counter'
                total_bench_counter += 1
                # take times
                for toolname in toolsnames:
                    time = line[toolsnumcol[toolname]]
                    # check is the benchmark terminated or is an error
                    if is_number(time):
                        toolstimes[toolname] += [float(time)]
                        # compute the max
                        toolsmaxtimes[toolname] = float(time) if float(time) > toolsmaxtimes[toolname] else toolsmaxtimes[toolname]


    # create the instants
    instants = []
    inst_num = 0
    factor = float(args.threshold)/float(args.tiks)
    while ( inst_num < float(args.threshold) + factor ):
        instants.append( inst_num )
        inst_num += factor

    ## TODO: remove it!
    #aalta_counter = 0
    #black_counter = 0
    #for toolname in toolsnames:
    #    for time in toolstimes[toolname]:
    #        if time < 0.7:
    #            if toolname=='black/finite':
    #                aalta_counter += 1
    #            if toolname=='aalta/finite':
    #                black_counter+=1
    #print("aalta_counter = "+str(aalta_counter))
    #print("black_counter = "+str(black_counter))



    ### count the # of benchmarks solved in less than "instant" seconds, for
    ### each tool

    # dictionary:  name  =>  list of % of solved benchs in less than
    # "instant" seconds
    toolspercent = {}
    for toolname in toolsnames:
        toolspercent[toolname] = []

    for toolname in toolsnames:
        for instant in instants:
            counter = 0
            for tooltime in toolstimes[toolname]:
                if tooltime <= instant:
                    counter += 1
            toolspercent[toolname] += [counter/total_bench_counter]

    ##TODO: remove it!
    #for toolname in toolsnames:
    #    print(toolname)
    #    counter = 0
    #    for instant in instants:
    #        if 0.3 <= instant and instant <= 0.5:
    #            print(toolspercent[toolname][counter])
    #        counter += 1
    #    print('\n\n')
     

    ### Create the Survival Plot
    fig = go.Figure()
    for toolname in toolsnames:
        fig.add_trace(go.Scatter(
                      x=instants[1:], 
                      y=toolspercent[toolname][1:],
                      mode='lines+markers',
                      name=toolname))
    
    # labels
    fig.update_xaxes(title_text="Time (sec.)")
    fig.update_yaxes(title_text="Percentage of Completion (%)")

    if args.pngopt:
        # save file into 'img' folder
        fig.write_image(img_path_name)
    if args.htmlopt:
        # show
        fig.show()



if __name__ == "__main__":
   main(sys.argv[1:])
