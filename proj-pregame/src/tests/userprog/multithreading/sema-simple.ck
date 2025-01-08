# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(sema-simple) begin
(sema-simple) PASS
(sema-simple) end
sema-simple: exit(0)
EOF
pass;
