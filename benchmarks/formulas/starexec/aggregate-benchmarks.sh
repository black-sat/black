#!/bin/bash

case $1 in
  --field=time)
   field=5
  ;;
  --field=memory)
   field=7
  ;;
  --field=[0-9])
    field=$(echo $1 | cut -d= -f2)
  ;;
  *)
    cat <<MSG 1>&2
Please specify 'time' or 'memory' or a column index as first argument
MSG
    exit 1
  ;;
esac
shift

files=$@

solvers="aalta trp++ ls4 Leviathan-06d4951 NuSMV-2.6.0,BDD pltl,graph pltl,tree"

formulae() {
  find acacia alaska/lift schuppan trp -depth 1
  find rozier anzu -depth 2
  echo forobots
}

data() {
  cat $files | grep $1 | grep $2 | cut -d',' -f $field | st --avg --fmt=%f
}

solver_names() {
  sed 's/Leviathan-06d4951/Leviathan/g ; s/NuSMV-2.6.0,BDD/NuSMV/g'
}

sanitize() {
  sed 's#/.*/#/#g; s#/#-#g; s/formula//g; s/arry//g; s/inear//g; s/[,_]/-/g'
}

(
  echo "group $solvers" # Header
  
  for formula in $(formulae); do
    echo -n "$formula "
    for solver in $solvers; do
      echo -n "$(data $formula $solver) "
    done
    echo
  done
) | solver_names | sanitize | column -t


# # 10.43