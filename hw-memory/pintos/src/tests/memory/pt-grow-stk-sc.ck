# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_EXIT_CODES => 1, [<<'EOF']);
(pt-grow-stk-sc) begin
(pt-grow-stk-sc) open "sample.txt"
(pt-grow-stk-sc) read "sample.txt"
(pt-grow-stk-sc) compare read data against reference
(pt-grow-stk-sc) end
EOF
pass;
