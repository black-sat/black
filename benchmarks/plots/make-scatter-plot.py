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
    parser = argparse.ArgumentParser(description='Parser for make-scatter-plot.py')
    parser.add_argument('datafile', metavar='datafile', 
                        nargs='?', default='_error_',
                        help='Name of the data file')
    parser.add_argument('xtool', metavar='xtool', 
                        nargs='?', default='_error_',
                        help='Name of the tool for the x-axis')
    parser.add_argument('ytool', metavar='ytool', 
                        nargs='?', default='_error_',
                        help='Name of the tool for the y-axis')
    parser.add_argument('errortime', metavar='errortime', type=int,
                        default=0,
                        help='In order to distinguish the benchmarks that have an error from those that'
                             'reach the timeout, we define the ERRORTIME as the timeout time plus 180'
                             'seconds')
    parser.add_argument('-p', '--pdf', dest='pdfopt', 
                        action='store_true', default=0,
                        help='Dumps the pdf file with the plot.')
    parser.add_argument('-m', '--html', dest='htmlopt',
                        action='store_true', default=0,
                        help='Opens the browser with the interactive plot.')
    parser.add_argument('-l', '--log', dest='logopt',
                        action='store_true', default=0,
                        help='Uses the logarithmic scale for the x- and y-axis.')
    parser.add_argument('-d', '--no-legend', dest='nolegendopt',
                        action='store_true', default=0,
                        help='If set, suppresses the generation of the legend')
    args = parser.parse_args()
    
    # check on the options
    if not os.path.exists(args.datafile) or args.datafile=='_error_':
        sys.exit('Please specify the datafile')
    if args.xtool=="_error_" or args.ytool=="_error_":
        sys.exit('Please specify both the tools for the x- and y-axis')
    if args.errortime==0:
        sys.exit('Please specify the errortime value')

    #datafile's name without path
    datafile_name = args.datafile.split('/')[-1]
    #path for image
    img_path_name = "scatterplot."+args.xtool.replace("/","")+"."+args.ytool.replace("/","")+"."+datafile_name.replace(".csv",".pdf").replace(".dat",".pdf")


    # categories dictionaries (one for SAT and one for UNSAT):
    # 
    #   {category_1} -> ([category_1_time_x], [category_1_time_y]),
    #   {category_2} -> ([category_2_time_x], [category_2_time_y]),
    #       ...
    #   {category_3} -> ([category_n_time_x], [category_n_time_y])
    # 
    sat_categories          = {}
    unsat_categories        = {}
    # number of column for xtool
    numcol_xtool            = -1
    # number of column for ytool
    numcol_ytool            = -1
    # number of column for family
    numcol_family           = -1

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
                # computing numcol_xtool , numcol_ytool , numcol_family
                colindex = 0
                for col in line:
                    if col == args.xtool and numcol_xtool == -1:
                        numcol_xtool = colindex
                    if col == args.ytool and numcol_ytool == -1:
                        numcol_ytool = colindex
                    if col == 'family' and numcol_family == -1:
                        numcol_family = colindex
                    colindex += 1
                header = False
            else:
                # take category name
                category=line[numcol_family].strip()
                
                # take result of benchmark
                # 0 = UNSAT,  1 = SAT,  2 = err
                bench_is_sat = 2
                for col in line[4::2]: # the results start at index 4 of the datafile. We take only the even positions.
                    if col.strip() == 'SAT':
                        bench_is_sat = 1
                        break
                    if col.strip() == 'UNSAT':
                        bench_is_sat = 0
                        break
                    if not 'err' in col.strip():
                        print(line)
                        sys.exit('Error in the datafile')

                # check the current times for xtool and ytool
                time_xtool = 0 
                x_is_sat = 2 # 0 = UNSAT,  1 = SAT,  2 = err
                if 'SAT' in line[numcol_xtool+1] or 'UNSAT' in line[numcol_xtool+1]:
                    # result
                    x_is_sat = 1 if line[numcol_xtool+1].strip() == 'SAT' else 0
                    #time
                    if is_number(line[numcol_xtool]):
                        time_xtool = float(line[numcol_xtool])
                    else:
                        time_xtool = float(args.errortime)
                        print(line)
                        print('Error in the datafile for tool '+
                                 args.xtool+' : the result is '+
                                 line[numcol_xtool+1].strip()+
                                 ' but the associated time is '+
                                 str(line[numcol_xtool].strip()))
                elif 'err' in line[numcol_xtool+1]:
                    x_is_sat = 2 
                    time_xtool = float(args.errortime)
                else:
                    sys.exit('Error: unreachable code')
                
                time_ytool = 0 
                y_is_sat = 2
                if 'SAT' in line[numcol_ytool+1] or 'UNSAT' in line[numcol_ytool+1]:
                    y_is_sat = 1 if line[numcol_ytool+1].strip() == 'SAT' else 0
                    if is_number(line[numcol_ytool]):
                        time_ytool = float(line[numcol_ytool])
                    else:
                        time_xtool = float(args.errortime)
                        print(line)
                        print('Error in the datafile for tool '+
                                 args.ytool+' : the result is '+
                                 line[numcol_ytool+1].strip()+
                                 ' but the associated time is '+
                                 str(line[numcol_ytool].strip()))
                elif 'err' in line[numcol_ytool+1]:
                    y_is_sat = 2
                    time_ytool = float(args.errortime)
                else:
                    sys.exit('Error: unreachable code')

                # check the results 
                if bench_is_sat == 0 and (x_is_sat == 1 or y_is_sat == 1):
                    print(line)
                    print('Error: found different results.')
                if bench_is_sat == 1 and (x_is_sat == 0 or y_is_sat == 0):
                    print(line)
                    print('Error: found different results.')
                if (x_is_sat==1 and y_is_sat==0) or (x_is_sat==0 and y_is_sat==1):
                    print('Error: found different results.')

                # insert times in the right category
                if bench_is_sat == 1:
                    sat_categories[category] = (
                        sat_categories[category][0]+[time_xtool] if category in sat_categories else [time_xtool],
                        sat_categories[category][1]+[time_ytool] if category in sat_categories else [time_ytool]
                    )
                elif bench_is_sat == 0:
                    unsat_categories[category] = (
                        unsat_categories[category][0]+[time_xtool] if category in unsat_categories else [time_xtool],
                        unsat_categories[category][1]+[time_ytool] if category in unsat_categories else [time_ytool]
                    )
                else: # bench_is_sat == 2
                    continue




    ### scatter plot for time (fig = go.Figure())
    fig = make_subplots(
        rows=1, 
        cols=2,
        shared_yaxes=True, # sharing the y-axis
        horizontal_spacing = 0.02 # spacing between the two subplots
    )


    ### Add a TRACE for each category in SAT and UNSAT

    # color_categories
    color_categories = {}
    color_counter = 3
    for category in list(sat_categories.keys())+list(unsat_categories.keys()):
        if not category in color_categories:
            color_categories[category] = PLOTLY_COLORS[color_counter]
            color_counter = (color_counter + 1) % len(PLOTLY_COLORS)

    # SAT
    for category in sat_categories:
        fig.add_trace(go.Scatter(
            x=sat_categories[category][0],
            y=sat_categories[category][1],
            mode='markers',
            marker=dict(
                symbol='triangle-up',
                size=5,
                color=color_categories[category]
            ),
            name=category,
            legendgroup="group"+category,
            showlegend=False if args.nolegendopt else True),
            row=1,col=1)

    # UNSAT
    for category in unsat_categories:
        fig.add_trace(go.Scatter(
            x=unsat_categories[category][0],
            y=unsat_categories[category][1],
            mode='markers',
            marker=dict(
                symbol='triangle-up',
                size=5,
                color=color_categories[category]
            ),
            name=category,
            legendgroup="group"+category,
            showlegend=False if category in sat_categories or args.nolegendopt else True),
            row=1,col=2)

    # name the two axis
    fig.update_xaxes(
        title_text=args.xtool,
        type="log" if args.logopt else "",
        dtick=1
    )
    fig.update_yaxes(
        title_text=args.ytool,
        type="log" if args.logopt else "",
        dtick=1
    )

    # name the two axis
    fig.update_xaxes(
        title_text=args.xtool,
        type="log" if args.logopt else "",
        row=1,col=1)
    fig.update_yaxes(
        title_text=args.ytool,
        type="log" if args.logopt else "",
        scaleanchor = "x", scaleratio = 1, # same scale of x-axis
        row=1,col=1)

    # name the two axis
    fig.update_xaxes(
        title_text=args.xtool,
        type="log" if args.logopt else "",
        row=1,col=2)
    fig.update_yaxes(
        title_text='', # share the label on the y-axis
        type="log" if args.logopt else "",
        scaleanchor = "x", scaleratio = 1, # same scale of x-axis
        row=1, col=2)


    fig.update_layout(
        height=600, width=1100,
        legend=dict(
            orientation="h",
            #yanchor="bottom",
            y=-0.15,
            #xanchor="right",
            #x=1
        ),
        #font=dict(
        #    #family="Courier New, monospace",
        #    size=20,
        #    #color="RebeccaPurple"
        #)
    )

    # remove margins
    fig['layout'].update(margin=dict(l=0,r=0,b=0,t=0))

    # save img
    if args.pdfopt:
        fig.write_image(img_path_name)
    if args.htmlopt:
        fig.show()




if __name__ == "__main__":
   main(sys.argv[1:])
