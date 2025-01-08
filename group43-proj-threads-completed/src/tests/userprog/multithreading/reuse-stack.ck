# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(reuse-stack) begin
(reuse-stack) Main started
(reuse-stack) Threads done
(reuse-stack) Main finished
(reuse-stack) end
reuse-stack: exit(0)
EOF
pass;
