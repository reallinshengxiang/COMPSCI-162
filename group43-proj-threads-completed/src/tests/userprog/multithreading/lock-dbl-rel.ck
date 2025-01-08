# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(lock-dbl-rel) begin
(lock-dbl-rel) First release succeeded
lock-dbl-rel: exit(1)
EOF
pass;
