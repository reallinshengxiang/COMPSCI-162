# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(smfs-hierarchy-256) begin
(smfs-hierarchy-256) Spawning counter threads...
(smfs-hierarchy-256) Done spawning counter threads.
(smfs-hierarchy-256) counters[0] < counters[1]
(smfs-hierarchy-256) counters[1] < counters[2]
(smfs-hierarchy-256) counters[2] < counters[3]
(smfs-hierarchy-256) counters[3] < counters[4]
(smfs-hierarchy-256) counters[4] < counters[5]
(smfs-hierarchy-256) counters[5] < counters[6]
(smfs-hierarchy-256) counters[6] < counters[7]
(smfs-hierarchy-256) end
EOF
pass;
