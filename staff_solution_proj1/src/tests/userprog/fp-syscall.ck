# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(fp-syscall) begin
(fp-syscall) Computing e...
(fp-syscall) Success!
(fp-syscall) Kernel computation successful
fp-syscall: exit(162)
EOF
pass;
