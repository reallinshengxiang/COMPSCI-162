# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(file-join) begin
(file-join) Main starting
(file-join) All threads complete
file-join: exit(0)
EOF
pass;
