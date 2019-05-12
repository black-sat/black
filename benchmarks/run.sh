#!/bin/bash

# timeout time in seconds
TIMEOUT=300 # five minutes

# utilities
report() {
  echo $@ >&2
}

output() {
  echo $@
}

die() {
  report $1
  exit 1
}

# Wrapper over time and timeout
measure() {
  # Temporary file to store the time measurement
  timefile=$(mktemp)
  # Temporary file to store the command error stream output
  outputfile=$(mktemp)

  # actually runs the command
  n=0 # number of runs
  elapsed=0.0 # total elapsed time

  export TIMEFORMAT=%R
  while [ $n -le 10 -a $(bc -l <<< "$elapsed>1") -ne 1 ]; do
    # The following line is the core of everything:
    # 1. Lunch the given command ($@)
    # 2. with the given timeout (\timeout $TIMEOUT)
    # 3. and measure its execution time (time)
    time=$(time (\timeout $TIMEOUT $@ > /dev/null 2> $outputfile) 2>&1)

    # accumulate the time and increment the counter for the average
    elapsed=$(bc -l <<< $time+$elapsed)
    n=$((n+1))
  done

  # Compute the average time
  avg=$(bc -l <<< $elapsed/$n)

  # Output it to the temporary file
  \printf "%.3f" $avg > $timefile

  # re-output the command's standard error
  cat $outputfile >&2

  # define the call-back for the caller
  print_time() {
    cat $timefile
  }
}

# setups the benchmark
setup() {
  # Reset locales
  export LC_ALL=C

  # Reset working directory
  cd "$(git rev-parse --show-toplevel)/benchmarks"

  # find solvers directory
  solvers_dir=$(pwd)/solvers
  solvers_arch_dir=$(pwd)/solvers-$(uname -s)-$(uname -m)

  if [ -d $solvers_arch_dir ]; then
    solvers_dir=$solvers_arch_dir
  fi

  # get list of solvers
  solvers=$(ls $solvers_dir)

  # setup PATH for solver scripts
  for solver in $solvers; do
    export PATH="$solvers_dir/$solver/bin:$PATH"
  done

  # ensure to use GNU utilities on macOS
  # (they have to be installed with Homebrew for this to work)
  # export PATH="/usr/local/opt/gnu-time/libexec/gnubin:$PATH"
  export PATH="/usr/local/opt/coreutils/libexec/gnubin:$PATH"
  export PATH="/usr/local/opt/findutils/libexec/gnubin:$PATH"

  # get list of solvers run modes
  modes=$(for s in $solvers; do
    if [ -d scripts/$s ]; then
      for m in $(find "scripts/$s" -mindepth 1 -type d -exec basename {} ';'); do
        echo $s/$m
      done
    fi
  done)

  report "Detected solvers: "
  for mode in $modes; do
    report "- $mode"
  done

  report "Timeout: $TIMEOUT seconds"
}

# setups the run of a specific solver mode on one formula
setup_run() {
  mode=$1
  formula=$2

  [ -f "./formulas/$formula" ] || die "Unable to open file: $2"

  prescript="scripts/$mode/../pre.sh"
  runscript="scripts/$mode/run.sh"
  postscript="scripts/$mode/../post.sh"

  if [ ! -x "$prescript" ]; then
    prescript=cat
  fi

  if [ ! -x "$postscript" ]; then
    postscript=cat
  fi

  do_run() {
    wrapper=$@

    # Pre-process input
    input=$(mktemp)
    $prescript "./formulas/$formula" > $input

    # Run and Post-process
    $wrapper $runscript $input
  }

  postprocess() {
    $postscript
  }
}

#
# User-invokable entry points start here
#

# runs one specific solver mode on one formula
run() {
  setup &> /dev/null
  setup_run $@
  do_run | postprocess
}

# benchmarks one specific solver mode on one formula
bench()
{
  setup &> /dev/null
  setup_run $@

  do_run measure > /dev/null
  print_time
  output
}

bench_formula()
{
  setup &> /dev/null

  formula=$1

  if [ ! "$indexfile" ]; then
    indexfile=formulas/index
  fi

  statusmsg="Benchmarking formula: $formula"
  if [ "$PARALLEL_SEQ" ]; then
    total=$(cat $indexfile | wc -l | sed 's/ //g')
    statusmsg="Benchmarking formula $PARALLEL_SEQ/$total: $formula"
  fi

  report $statusmsg

  output -n $formula
  for mode in $modes; do
    report -n "- $mode..."
    setup_run $mode $formula
    do_run measure 1>&2
    \printf " %.3f" $(print_time)
    report " done"
  done
  output
}

bench_all()
{
  [ "$1" ] || die "Please specify an output file for data"
  output=$1

  setup

  indexfile=formulas/index

  # Get total number of formulas
  total=$(cat $indexfile | wc -l | sed 's/ //g')

  report -e "Found $total formulas"

  if [ -f "$output" ]; then
    tempindex=$(mktemp)
    grep -F -v -f \
      <(cat $output | tail -n +2 | awk '{print $1}') $indexfile \
      > $tempindex
    indexfile=$tempindex

    total=$(cat $indexfile | wc -l | sed 's/ //g')
    report -e "Running on $total missing formulas\\n"
  fi

  report -e "Starting...\\n"

  # print header of the output table
  if [ ! -f "$output" ]; then
    {
      output -n "formula"
      for mode in $modes; do
        \printf " $mode"
      done
      output
    } > $output
  fi

  export indexfile

  # heavy lifting
  cat $indexfile | \
    parallel ./run.sh bench_formula {} '{#}' $total >> $output
}

# Actual script starts here. Just executes whatever is found on the command line
$@
