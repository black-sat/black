#!/bin/bash

nuXmv -source <(cat <<END
set input_file $1
go
build_boolean_model
check_ltlspec_klive
quit
END
) <<END
quit
END
