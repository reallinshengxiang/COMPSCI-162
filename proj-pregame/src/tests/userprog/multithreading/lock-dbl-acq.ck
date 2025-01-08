# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(lock-dbl-acq) begin
(lock-dbl-acq) First acquire succeeded
lock-dbl-acq: exit(1)
EOF
pass;
