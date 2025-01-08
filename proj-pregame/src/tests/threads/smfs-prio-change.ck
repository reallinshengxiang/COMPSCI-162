# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(smfs-prio-change) begin
(smfs-prio-change) Waking up alice.
(smfs-prio-change) Thread alice woke up.
(smfs-prio-change) Waking up bob.
(smfs-prio-change) Thread bob woke up.
(smfs-prio-change) Thread main exiting.
(smfs-prio-change) end
EOF
pass;
