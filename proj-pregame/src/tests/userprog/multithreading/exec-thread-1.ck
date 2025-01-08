# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(exec-thread-1) Creating child thread
(exec-thread-1) Child thread starting
exec-thread-1: exit(0)
(exec-thread-1) Child thread finishing
exec-thread-1: exit(0)
EOF
pass;
