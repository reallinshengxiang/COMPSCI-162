# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(exit-clean) Root process starting
(exit-clean) Spawning child threads
exit-clean-2: exit(-1)
(exit-clean) Root process continuing
(exit-clean) Success!
exit-clean-2: exit(0)
EOF
pass;
