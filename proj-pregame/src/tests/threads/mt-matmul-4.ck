# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(mt-matmul-4) begin
(mt-matmul-4) Executing blocked matmul with 4 threads...
(mt-matmul-4) Matrix results match expected values.
(mt-matmul-4) end
EOF
pass;
