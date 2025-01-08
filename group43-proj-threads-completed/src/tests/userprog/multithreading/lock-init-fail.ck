# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(lock-init-fail) begin
(lock-init-fail) Success!
(lock-init-fail) end
lock-init-fail: exit(0)
EOF
pass;
