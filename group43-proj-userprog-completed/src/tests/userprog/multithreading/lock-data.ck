# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(lock-data) begin
(lock-data) PASS
(lock-data) end
lock-data: exit(0)
EOF
pass;
