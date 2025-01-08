# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
do-not-much: exit(1)
EOF
pass;
