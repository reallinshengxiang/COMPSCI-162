# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(mt-matmul-16) begin
(mt-matmul-16) Executing blocked matmul with 16 threads...
(mt-matmul-16) Matrix results match expected values.
(mt-matmul-16) end
EOF
pass;
