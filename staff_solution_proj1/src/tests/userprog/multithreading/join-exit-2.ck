# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(join-exit-2) begin
(join-exit-2) Main starting
(join-exit-2) Thread starting
join-exit-2: exit(162)
EOF
pass;
