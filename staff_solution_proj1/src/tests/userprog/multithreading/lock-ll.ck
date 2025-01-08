# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(lock-ll) begin
(lock-ll) PASS
(lock-ll) end
lock-ll: exit(0)
EOF
pass;
