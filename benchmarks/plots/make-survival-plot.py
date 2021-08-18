import os, os.path, sys, argparse
import csv 
import plotly
from plotly.subplots import make_subplots
import plotly.graph_objects as go
import numpy as np

PLOTLY_COLORS = plotly.colors.DEFAULT_PLOTLY_COLORS


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
    parser.add_argument('-p', '--pdf', dest='pdfopt', 
                        action='store_true', default=0,
                        help='Dumps the pdf file with the plot.')
    parser.add_argument('-m', '--html', dest='htmlopt',
                        action='store_true', default=0,
                        help='Opens the browser with the interactive plot.')
    parser.add_argument('-t', '--threshold', dest='threshold',
                        default=0,
                        help='Threshold value (= maximum value for the x-axis).')
    parser.add_argument('-i', '--tiks', dest='tiks',
                        default=0,
                        help='Number of tiks of the x-axis.')
    parser.add_argument('-d', '--no-legend', dest='nolegendopt',
                        action='store_true', default=0,
                        help='If set, suppresses the generation of the legend')
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
    img_path_name = "survivalplot."+datafile_name.replace(".csv",".pdf").replace(".dat",".pdf")
    # number of column for tools: it's a dictionary:   name => numcol
    toolsnumcol={}
    # number of benchmarks
    sat_total_bench_counter = 0
    unsat_total_bench_counter = 0
    # tools'times : it is a dictionary:  name => list of times
    # one for SAT one for UNSAT
    sat_toolstimes={}
    unsat_toolstimes={}


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
                    # initializing
                    sat_toolstimes[toolname]=[]
                    unsat_toolstimes[toolname]=[]
                    colindex = 0
                    # computing toolsnumcol
                    for col in line:
                        if toolname == col:
                            toolsnumcol[toolname] = colindex
                        colindex += 1
                header = False
            else:
                # retrieve the result of this benchmarks
                is_sat = 0
                for toolname in toolsnames:
                    if line[toolsnumcol[toolname]+1].strip() == 'SAT':
                        is_sat = 1
                # increment 'total_bench_counter'
                if is_sat:
                    sat_total_bench_counter += 1
                else:
                    unsat_total_bench_counter += 1
                # take times
                for toolname in toolsnames:
                    time = line[toolsnumcol[toolname]]
                    # check is the benchmark terminated or is an error
                    if is_number(time):
                        if is_sat:
                            sat_toolstimes[toolname] += [float(time)]
                        else:
                            unsat_toolstimes[toolname] += [float(time)]


    # create the instants
    instants = []
    inst_num = 0
    factor = float(args.threshold)/float(args.tiks)
    while ( inst_num < float(args.threshold) + factor ):
        instants.append( inst_num )
        inst_num += factor


    ### count the # of benchmarks solved in less than "instant" seconds, for
    ### each tool

    # dictionary:  name  =>  list of % of solved benchs in less than
    # "instant" seconds
    sat_toolspercent = {}
    unsat_toolspercent = {}
    for toolname in toolsnames:
        sat_toolspercent[toolname] = []
        unsat_toolspercent[toolname] = []

    for toolname in toolsnames:
        for instant in instants:
            counter = 0
            for tooltime in sat_toolstimes[toolname]:
                if tooltime <= instant:
                    counter += 1
            sat_toolspercent[toolname] += [counter/sat_total_bench_counter]
            counter = 0
            for tooltime in unsat_toolstimes[toolname]:
                if tooltime <= instant:
                    counter += 1
            unsat_toolspercent[toolname] += [counter/unsat_total_bench_counter]


    ### Create the Survival Plot
    fig = make_subplots(
        rows=1, 
        cols=2,
        shared_yaxes=True, # sharing the y-axis
        horizontal_spacing = 0.02 # spacing between the two subplots
    )

    # plot
    counter = 0
    for toolname in toolsnames:
        fig.add_trace(go.Scatter(
                      x=instants[1:], 
                      y=sat_toolspercent[toolname][1:],
                      mode='lines',
                      line=dict(
                          color=PLOTLY_COLORS[counter]
                      ),
                      name=toolname,
                      showlegend=False if args.nolegendopt else True
                      ),
            row=1,col=1)
        fig.add_trace(go.Scatter(
                      x=instants[1:], 
                      y=unsat_toolspercent[toolname][1:],
                      mode='lines',
                      line=dict(
                          color=PLOTLY_COLORS[counter]
                      ),
                      name=toolname,
                      showlegend=False
                      ),
            row=1,col=2)
        counter += 1
    
    # labels
    fig.update_xaxes(
        title_text="Time (sec.)",
        row=1,
        col=1,
        type="log"
        )
    fig.update_yaxes(title_text="Percentage of Completion (%)",row=1,col=1)
    fig.update_xaxes(
        title_text="Time (sec.)",
        row=1,
        col=2,
        type="log")
    fig.update_yaxes(title_text="",row=1,col=2) #share label of y-axis
    
    # Legend
    fig.update_layout(
        #height=600, width=1100,
        legend=dict(
            orientation="h",
            y=-0.15,
        ))
    
    # remove margins
    fig['layout'].update(margin=dict(l=0,r=0,b=0,t=0))

    if args.pdfopt:
        # save file into 'img' folder
        fig.write_image(img_path_name)
    if args.htmlopt:
        # show
        fig.show()



if __name__ == "__main__":
   main(sys.argv[1:])
