# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(fp-kernel-e) begin
(fp-kernel-e) Computing e...
(fp-kernel-e) Success!
fp-kernel-e: exit(162)
EOF
pass;
