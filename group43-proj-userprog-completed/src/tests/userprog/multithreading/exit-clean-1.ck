# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(exit-clean) Root process starting
(exit-clean) Spawning child threads
exit-clean-1: exit(162)
(exit-clean) Root process continuing
(exit-clean) Success!
exit-clean-1: exit(0)
EOF
pass;
