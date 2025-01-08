# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(st-matmul) begin
(st-matmul) Executing single-threaded matmul...
(st-matmul) Matrix results match expected values.
(st-matmul) end
EOF
pass;
