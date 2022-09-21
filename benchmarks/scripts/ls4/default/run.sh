#!/bin/bash

set -e

lsproblem=$(mktemp)
translate=$(which TST-translate.pl)

swipl --stack_limit=4G -s $translate -g go -t "halt(1)" -- $1 $lsproblem 

ls4 < $lsproblem
