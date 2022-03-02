import os, os.path, sys, argparse
import csv 
import plotly
from plotly.subplots import make_subplots
import plotly.graph_objects as go
import numpy as np

# disable MathJax because of https://github.com/plotly/plotly.py/issues/3469
import plotly.io as pio   
pio.kaleido.scope.mathjax = None

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
    parser.add_argument('--bothsat', dest='both', action='store_true', default=0,
                        help='if set, create also a plot with both sat and unsat instances together.')
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


    # families dictionaries (one for SAT, one for UNSAT, and one for both):
    # 
    #   {   family_1: ([family_1_time_x], [family_1_time_y]),
    #       family_2: ([family_2_time_x], [family_2_time_y]),
    #       ...
    #       family_n: ([family_n_time_x], [family_n_time_y])
    #   }
    # 
    sat_families    = {}
    unsat_families  = {}
    both_families   = {}

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
            xtool_res = line[args.xtool+":result"].strip()
            ytool_res = line[args.ytool+":result"].strip()
            if xtool_res == 'SAT' or ytool_res == 'SAT':
                is_sat = True
            if xtool_res == 'UNSAT' or ytool_res == 'UNSAT':
                is_unsat = True

            if is_sat and is_unsat:
                print("Warning: found inconsistent results in line:\n" + " ".join(line.values()))

            # save times for xtool and ytool
            if is_sat or is_unsat:
                is_xtime_err = is_number(line[args.xtool]) and xtool_res != 'err'
                is_ytime_err = is_number(line[args.ytool]) and ytool_res != 'err'
                time_xtool = line[args.xtool] if is_xtime_err else float(args.errortime)
                time_ytool = line[args.ytool] if is_ytime_err else float(args.errortime)

                # insert times in the right family
                if is_sat and not is_unsat:
                    if family not in sat_families:
                        sat_families[family] = ([], [])
                    
                    sat_families[family][0].append(time_xtool)
                    sat_families[family][1].append(time_ytool)

                if is_unsat and not is_sat:
                    if family not in unsat_families:
                        unsat_families[family] = ([], [])
                    
                    unsat_families[family][0].append(time_xtool)
                    unsat_families[family][1].append(time_ytool)

                if family not in both_families:
                    both_families[family] = ([], [])
                    
                both_families[family][0].append(time_xtool)
                both_families[family][1].append(time_ytool)



    ### scatter plot for time (fig = go.Figure())
    fig = make_subplots(
        rows=1, 
        cols=3 if args.both else 2,
        shared_yaxes=True, # sharing the y-axis
        horizontal_spacing = 0.02 # spacing between the two subplots
    )


    ### Add a TRACE for each family in SAT, UNSAT, and eventually both

    # color_categories
    family_colors = {}
    color_counter = 3
    for family in list(both_families.keys()):
        if not family in family_colors:
            family_colors[family] = PLOTLY_COLORS[color_counter]
            color_counter = (color_counter + 1) % len(PLOTLY_COLORS)

    # SAT
    for family in sat_families:
        fig.add_trace(go.Scatter(
            x=sat_families[family][0],
            y=sat_families[family][1],
            mode='markers',
            marker=dict(
                symbol='triangle-up',
                size=5,
                color=family_colors[family]
            ),
            name=family,
            legendgroup="group"+family,
            showlegend=False if args.nolegendopt else True),
            row=1,col=1)

    # UNSAT
    for family in unsat_families:
        fig.add_trace(go.Scatter(
            x=unsat_families[family][0],
            y=unsat_families[family][1],
            mode='markers',
            marker=dict(
                symbol='triangle-up',
                size=5,
                color=family_colors[family]
            ),
            name=family,
            legendgroup="group"+family,
            showlegend=False if family in sat_families or args.nolegendopt else True),
            row=1,col=2)

    # BOTH
    if args.both:
        for family in both_families:
            fig.add_trace(go.Scatter(
                x=both_families[family][0],
                y=both_families[family][1],
                mode='markers',
                marker=dict(
                    symbol='triangle-up',
                    size=5,
                    color=family_colors[family]
                ),
                name=family,
                legendgroup="group"+family,
                showlegend=False if family in sat_families or family in unsat_families or args.nolegendopt else True),
                row=1,col=3)

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
        title_text=args.xtool + " (SAT)",
        type="log" if args.logopt else "",
        row=1,col=1)
    fig.update_yaxes(
        title_text=args.ytool,
        type="log" if args.logopt else "",
        scaleanchor = "x", scaleratio = 1, # same scale of x-axis
        row=1,col=1)

    fig.update_xaxes(
        title_text=args.xtool + " (UNSAT)",
        type="log" if args.logopt else "",
        row=1,col=2)
    fig.update_yaxes(
        title_text='', # share the label on the y-axis
        type="log" if args.logopt else "",
        scaleanchor = "x", scaleratio = 1, # same scale of x-axis
        row=1, col=2)

    if args.both:
        fig.update_xaxes(
            title_text=args.xtool + " (both)",
            type="log" if args.logopt else "",
            row=1,col=3)
        fig.update_yaxes(
            title_text='', # share the label on the y-axis
            type="log" if args.logopt else "",
            scaleanchor = "x", scaleratio = 1, # same scale of x-axis
            row=1, col=3)


    fig.update_layout(
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
        #),
        # shapes = [
        #     dict(type="line", xref="x", yref="y",
        #         x0=10**-2, y0=10**-2, x1=600, y1=600, line_width=1),
        # ],
        margin=dict(l=0,r=0,b=0,t=0)
    )

    if args.nolegendopt:
        fig.update_layout(height=405, width=1280)
    else:
        fig.update_layout(height=450, width=1280)  # add space for the legend

    # save img
    if args.pdfopt:
        fig.write_image(img_path_name)
    if args.htmlopt:
        fig.show()


if __name__ == "__main__":
   main(sys.argv[1:])
