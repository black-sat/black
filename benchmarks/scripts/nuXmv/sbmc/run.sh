#!/bin/bash

nuXmv -source <(cat <<END
set input_file $1
go
build_boolean_model
bmc_setup
check_ltlspec_sbmc_inc -c -k 10000
quit
END
) <<END
quit
END
