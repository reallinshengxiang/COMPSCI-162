# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(join-exit-1) begin
(join-exit-1) Main starting
(join-exit-1) Thread starting
(join-exit-1) Thread finished
join-exit-1: exit(0)
EOF
pass;
