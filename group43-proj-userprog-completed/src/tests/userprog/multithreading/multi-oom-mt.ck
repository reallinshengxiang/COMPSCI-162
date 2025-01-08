# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, IGNORE_EXIT_CODES => 1, [<<'EOF']);
(multi-oom-mt) begin
(multi-oom-mt) success. program forked 10 times.
(multi-oom-mt) end
EOF
pass;
