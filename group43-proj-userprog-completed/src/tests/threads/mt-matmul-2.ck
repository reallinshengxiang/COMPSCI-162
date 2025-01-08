# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(mt-matmul-2) begin
(mt-matmul-2) Executing blocked matmul with 2 threads...
(mt-matmul-2) Matrix results match expected values.
(mt-matmul-2) end
EOF
pass;
