import os, os.path, sys, argparse
import csv 
import plotly
from plotly.subplots import make_subplots
import plotly.graph_objects as go
import numpy as np
import time

# disable MathJax because of https://github.com/plotly/plotly.py/issues/3469
import plotly.io as pio   
pio.kaleido.scope.mathjax = None

PLOTLY_COLORS = plotly.colors.DEFAULT_PLOTLY_COLORS

def list_div(num, den):
    assert len(num) == len(den)
    div=[]

    for i in range(0,len(num)):
        div.append(num[i]/den[i])

    return div

def main(argv):
    parser = argparse.ArgumentParser(
        description='Create bar charts from raw data.'
        )
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
    parser.add_argument('-w', '--width', dest='width',
                        default=1280,
                        help='Width of the plot in pixels. The height is 500px.')
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
    img_path_name = "barchart." + datafile_name.replace(".csv",".pdf").replace(".dat",".pdf")

    # count instances (dictionary): {tool's name: {family: number of solved instances}, ...}
    solved_instances={}
    total_instances={}
    for toolname in toolsnames:
        solved_instances[toolname]={}
        total_instances[toolname]={}

    # list of families
    families=[]

    # opening the datafile 
    with open(args.datafile, mode ='r') as data_handle:
        # read the datafile with the empty space as delimiter
        data_file = csv.DictReader(data_handle, delimiter=" ")
        # read the content of the datafile
        for line in data_file:
            # retrieve the result of this benchmarks
            if line['family'] not in families:
                families.append(line['family'])
                for toolname in toolsnames:
                    solved_instances[toolname][line['family']] = 0
                    total_instances[toolname][line['family']] = 0

            for toolname in toolsnames:
                is_sat = line[toolname + ":result"].strip() == 'SAT'
                is_unsat = line[toolname + ":result"].strip() == 'UNSAT'
                if is_sat or is_unsat:
                    solved_instances[toolname][line['family']] += 1

                total_instances[toolname][line['family']] += 1

    # prepare data for charts
    data = []
    for toolname in toolsnames:
        solved = list(solved_instances[toolname].values())
        total = list(total_instances[toolname].values())
        data.append(go.Bar(
            name=toolname,
            x=families,
            y=list_div(solved,total),
            text=solved,
            )
        )

    fig = go.Figure(data)

    fig.update_xaxes(
        title_text="Family of formulas",
    )
    fig.update_yaxes(
        title_text="Percentage of solved instances (%)",
    )

    fig.update_layout(
        barmode='group',
        legend=dict(
            orientation="h",
            y=-0.15,
        ),
        width=int(args.width),
        height=500,
        margin=dict(l=0,r=0,b=0,t=0),  # remove margins
    )

    if args.pdfopt:
        # save file into 'img' folder
        fig.write_image(img_path_name)
    if args.htmlopt:
        # show
        fig.show()



if __name__ == "__main__":
   main(sys.argv[1:])