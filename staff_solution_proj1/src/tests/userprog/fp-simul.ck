# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(fp-simul) begin
(fp-simul) Computing e...
(compute-e) Success!
compute-e: exit(162)
(fp-simul) Success!
fp-simul: exit(162)
EOF
pass;
