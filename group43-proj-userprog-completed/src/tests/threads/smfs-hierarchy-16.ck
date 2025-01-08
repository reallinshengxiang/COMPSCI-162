# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(smfs-hierarchy-16) begin
(smfs-hierarchy-16) Spawning counter threads...
(smfs-hierarchy-16) Done spawning counter threads.
(smfs-hierarchy-16) counters[0] < counters[1]
(smfs-hierarchy-16) counters[1] < counters[2]
(smfs-hierarchy-16) counters[2] < counters[3]
(smfs-hierarchy-16) counters[3] < counters[4]
(smfs-hierarchy-16) counters[4] < counters[5]
(smfs-hierarchy-16) counters[5] < counters[6]
(smfs-hierarchy-16) counters[6] < counters[7]
(smfs-hierarchy-16) end
EOF
pass;
