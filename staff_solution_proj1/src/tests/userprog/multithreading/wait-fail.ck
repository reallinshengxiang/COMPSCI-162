# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(wait-fail) begin
(child-simple) run
child-simple: exit(81)
(wait-fail) Thread successfully finished waiting
(wait-fail) Thread successfully finished waiting
(wait-fail) end
wait-fail: exit(0)
EOF
pass;
