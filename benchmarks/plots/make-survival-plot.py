import os, os.path, sys, argparse
import csv 
import matplotlib.pyplot as plt



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
    parser.add_argument('-t', '--html', dest='htmlopt',
                        action='store_true', default=0,
                        help='Opens the browser with the interactive plot.')
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
                    if is_number(time):
                        toolstimes[toolname] += [float(time)]
                        # compute the max
                        toolsmaxtimes[toolname] = float(time) if float(time) > toolsmaxtimes[toolname] else toolsmaxtimes[toolname]



        # create the instants
        instants = []
        inst_num = 0
        factor = args.timeout/20
        while ( inst_num < args.timeout + factor ):
            instants.append( inst_num )
            inst_num += factor


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
                    if tooltime < instant:
                        counter += 1
                toolspercent[toolname] += [counter/total_bench_counter]
               


        ### Create plot
        fig = plt.figure()
        ax = fig.add_subplot(1, 1, 1)#, axisbg="1.0")

        for toolname in toolsnames:
            plt.plot(instants, toolspercent[toolname], label=toolname)

        
        # grid
        ax.grid(color='b', ls = '-.', lw = 0.25)
        
        # ticks
        #ticks_x = [ round((max_x / 10)*i,3) for i in range(0,10) ]
        #ticks_y = [ round((max_y / 10)*i,3) for i in range(0,10) ]
        #plt.xticks(ticks_x, ticks_x, rotation='vertical')
        #plt.yticks(ticks_y, ticks_y)
        # limits
        #ax.set_ylim(0,180)
        #ax.set_xlim(0,180)
        # pad between axis values
        #ax.xaxis.labelpad = 10
        # labels
        ax.set_xlabel("Time (sec.)")
        ax.set_ylabel("Percentage of Completion (%)")
        # title
        plt.title('Survival plot')
        plt.legend()
        

        if args.pngopt:
            # save file into 'img' folder
            fig.savefig(img_path_name)
        if args.htmlopt:
            # show
            plt.show()



if __name__ == "__main__":
   main(sys.argv[1:])
