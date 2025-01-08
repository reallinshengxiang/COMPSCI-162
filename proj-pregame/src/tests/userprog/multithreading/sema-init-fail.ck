# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(sema-init-fail) begin
(sema-init-fail) Success! NULL initialization prevented
(sema-init-fail) Success! Negative initialized prevented
(sema-init-fail) end
sema-init-fail: exit(0)
EOF
pass;
