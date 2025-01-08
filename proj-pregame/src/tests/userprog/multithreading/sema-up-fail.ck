# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(sema-up-fail) begin
sema-up-fail: exit(1)
EOF
pass;
