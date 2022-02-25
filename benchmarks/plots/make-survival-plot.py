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
    parser.add_argument('-p', '--pdf', dest='pdfopt', 
                        action='store_true', default=0,
                        help='Dumps the pdf file with the plot.')
    parser.add_argument('-m', '--html', dest='htmlopt',
                        action='store_true', default=0,
                        help='Opens the browser with the interactive plot.')
    parser.add_argument('-t', '--threshold', dest='threshold',
                        default=360,  # corresponds to the timeout imposed by the benchmark suite (5 mins) + 60s
                        help='Threshold value (= maximum value for the x-axis).')
    parser.add_argument('-i', '--tiks', dest='tiks',
                        default=100,
                        help='Number of tiks of the x-axis.')
    parser.add_argument('-d', '--no-legend', dest='nolegendopt',
                        action='store_true', default=0,
                        help='If set, suppresses the generation of the legend')
    parser.add_argument('--percentage', dest='usepercentage', action='store_true', default=0,
                        help='If set, y axis will have percentage instead of the number of instances solved.')
    parser.add_argument('--bothsat', dest='both', action='store_true', default=0,
                        help='if set, create also a plot with both sat and unsat instances together.')
    parser.add_argument('--family', dest='family', nargs='?',
                        help='Restrict the plot to a single family of benchmarks.')
    args = parser.parse_args()


    
    # check on the options
    if not os.path.exists(args.datafile) or args.datafile=='_error_':
        sys.exit('Please specify the datafile')
    if args.tools=="_error_":
        sys.exit('Please specify the tools')

    # create list with tool's name
    toolsnames=[]
    tool_list = args.tools.strip('\"')
    for toolname in tool_list.split(','):
        toolsnames += [toolname.strip()]
    #datafile's name without path
    datafile_name = args.datafile.split('/')[-1]
    #path for image
    strfamily = ''
    if args.family:
        strfamily = args.family + "."
    img_path_name = "survivalplot." + strfamily + datafile_name.replace(".csv",".pdf").replace(".dat",".pdf")
    # number of benchmarks
    sat_total_bench_counter = 0
    unsat_total_bench_counter = 0
    both_total_bench_counter = 0
    # tools'times : it is a dictionary:  name => list of times
    # one for SAT one for UNSAT
    sat_toolstimes={}
    unsat_toolstimes={}
    both_toolstimes={}
    for toolname in toolsnames:
        sat_toolstimes[toolname] = []
        unsat_toolstimes[toolname] = []
        both_toolstimes[toolname] = []


    # opening the datafile 
    with open(args.datafile, mode ='r') as data_handle: 
        # read the datafile with the empty space as delimiter
        data_file = csv.DictReader(data_handle, delimiter=" ")
        # read the content of the datafile
        for line in data_file:
            # retrieve the result of this benchmarks
            if not args.family or line['family'] == args.family:

                # check consistency of results
                # note: we remove inconsistent results, they are due to aalta/v2 bugs and they are not that many
                is_sat = False
                is_unsat = False

                for toolname in toolsnames:
                    if line[toolname + ":result"].strip() == 'SAT':
                        is_sat = True
                    elif line[toolname + ":result"].strip() == 'UNSAT':
                        is_unsat = True

                # retrieve times
                for toolname in toolsnames:
                    if is_sat and not is_unsat:
                        is_sat = True
                        time = line[toolname]
                        if is_number(time):
                            sat_toolstimes[toolname].append(float(time))
                    if is_unsat and not is_sat:
                        is_unsat = True
                        time = line[toolname]
                        if is_number(time):
                            unsat_toolstimes[toolname].append(float(time))
                    if is_sat or is_unsat:
                        if is_number(time):
                            both_toolstimes[toolname].append(float(time))

                # count instances
                # note: note every instance is either sat or unsat, could be that any solver solved it
                if is_sat and not is_unsat:
                    sat_total_bench_counter += 1
                if is_unsat and not is_sat:
                    unsat_total_bench_counter += 1
                both_total_bench_counter += 1
                


    # if we don't want the percentage
    shared_y_axis = True
    y_axis_label = "Percentages of Completions (%)"
    sat_total = sat_total_bench_counter
    unsat_total = unsat_total_bench_counter
    both_total = both_total_bench_counter
    if not args.usepercentage:
        sat_total_bench_counter = 1
        unsat_total_bench_counter = 1
        both_total_bench_counter = 1
        shared_y_axis = False
        y_axis_label = "Number of instances solved"

    # create the instants
    instants = []
    inst_num = 0
    factor = float(args.threshold)/float(args.tiks)
    while ( inst_num < float(args.threshold) + factor ):
        instants.append( inst_num )
        inst_num += factor


    ### Count the # of benchmarks solved in less than "instant" seconds, for each tool

    # dictionary:  name  =>  list of % of solved benchs in less than "instant" seconds
    sat_toolspercent = {}
    unsat_toolspercent = {}
    both_toolspercent = {}
    for toolname in toolsnames:
        sat_toolspercent[toolname] = []
        unsat_toolspercent[toolname] = []
        both_toolspercent[toolname] = []

    for toolname in toolsnames:
        for instant in instants:
            # sat
            counter = 0
            for tooltime in sat_toolstimes[toolname]:
                if tooltime <= instant:
                    counter += 1
            percentage = counter/sat_total_bench_counter if sat_total_bench_counter != 0 else 0
            sat_toolspercent[toolname].append(percentage)
            # unsat
            counter = 0
            for tooltime in unsat_toolstimes[toolname]:
                if tooltime <= instant:
                    counter += 1
            percentage = counter/unsat_total_bench_counter if unsat_total_bench_counter != 0 else 0
            unsat_toolspercent[toolname].append(percentage)
            # both
            counter = 0
            for tooltime in both_toolstimes[toolname]:
                if tooltime <= instant:
                    counter += 1
            percentage = counter/both_total_bench_counter if both_total_bench_counter != 0 else 0
            both_toolspercent[toolname].append(percentage)


    ### Create the Survival Plot
    fig = make_subplots(
        rows=1, 
        cols=3 if args.both else 2,
        shared_yaxes=shared_y_axis, # sharing the y-axis
        horizontal_spacing = 0.01 if args.usepercentage else 0.04  # spacing between the two subplots
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
                      showlegend=False if args.nolegendopt else True,
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
        if args.both:
            fig.add_trace(go.Scatter(
                      x=instants[1:], 
                      y=both_toolspercent[toolname][1:],
                      mode='lines',
                      line=dict(
                          color=PLOTLY_COLORS[counter]
                      ),
                      name=toolname,
                      showlegend=False
                      ),
            row=1,col=3)
        counter += 1
    
    if not args.usepercentage:
        fig.add_hline(y=sat_total, row=1, col=1,
            annotation_text=str(sat_total),
            annotation_position="top left",
            #annotation_font_size=20,
            annotation_font_color="grey",
            line_color="grey",
            line_dash="dot",
            )
        fig.add_hline(y=unsat_total, row=1, col=2,
            annotation_text=str(unsat_total),
            annotation_position="top left",
            annotation_font_color="grey",
            line_color="grey",
            line_dash="dot",
            )
        if args.both:
            fig.add_hline(y=both_total, row=1, col=3,
                annotation_text=str(both_total),
                annotation_position="top left",
                annotation_font_color="grey",
                line_color="grey",
                line_dash="dot",
                )

    # labels
    fig.update_xaxes(
        title_text="Time for SAT (sec.)",
        row=1,
        col=1,
        type="log",
        dtick=1,
        rangemode="tozero",
        )
    fig.update_yaxes(
        title_text=y_axis_label,
        row=1,
        col=1,
        scaleanchor = "x", 
        scaleratio = 1, # same scale of x-axis
        rangemode="tozero",
        )
    fig.update_xaxes(
        title_text="Time for UNSAT (sec.)",
        row=1,
        col=2,
        type="log",
        dtick=1,
        rangemode="tozero",
        )
    fig.update_yaxes(
        title_text="", #share label of y-axis
        row=1,
        col=2,
        scaleanchor = "x", 
        scaleratio = 1, # same scale of x-axis
        rangemode="tozero",
        )
    if args.both:
        fig.update_xaxes(
            title_text="Time for both (sec.)",
            row=1,
            col=3,
            type="log",
            dtick=1,
            rangemode="tozero",
            )
        fig.update_yaxes(
            title_text="", #share label of y-axis
            row=1,
            col=3,
            scaleanchor = "x", 
            scaleratio = 1, # same scale of x-axis
            rangemode="tozero",
            )
    
    # Legend
    fig.update_layout(
        legend=dict(
            orientation="h",
            y=-0.15,
            )
        )

    if args.nolegendopt:
        fig.update_layout(height=405, width=1280)
    else:
        fig.update_layout(height=450, width=1280)  # add space for the legend
    
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
