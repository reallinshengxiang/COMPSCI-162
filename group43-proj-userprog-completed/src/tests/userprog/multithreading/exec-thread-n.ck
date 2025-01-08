# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(exec-thread-n) Root process starting
(exec-thread-n) Spawning child threads
(exec-thread-n) Child thread ran
(exec-thread-n) Child thread ran
(exec-thread-n) Child thread ran
(exec-thread-n) Child thread ran
(exec-thread-n) Child thread ran
(exec-thread-n) Child thread ran
(exec-thread-n) Child thread ran
(exec-thread-n) Child thread ran
(exec-thread-n) Child thread ran
(exec-thread-n) Child thread ran
(exec-thread-n) Finished waiting on children
exec-thread-n: exit(0)
(exec-thread-n) Root process finishing
exec-thread-n: exit(0)
EOF
pass;
