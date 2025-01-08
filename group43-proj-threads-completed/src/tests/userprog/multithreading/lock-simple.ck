# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(lock-simple) begin
(lock-simple) PASS
(lock-simple) end
lock-simple: exit(0)
EOF
pass;
