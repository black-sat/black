#!/bin/bash

nuXmv -source <(cat <<END
set on_failure_script_quits 1
set input_file $1
go
build_boolean_model
bmc_setup
check_ltlspec_sbmc_inc -c -k 100000
quit
END
)
