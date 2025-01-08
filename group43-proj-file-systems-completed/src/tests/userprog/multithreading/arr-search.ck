# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(arr-search) begin
(arr-search) Main starting
(arr-search) Main finished
(arr-search) end
arr-search: exit(0)
EOF
pass;
