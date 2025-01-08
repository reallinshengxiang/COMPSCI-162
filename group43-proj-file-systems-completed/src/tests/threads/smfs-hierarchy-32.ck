# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(smfs-hierarchy-32) begin
(smfs-hierarchy-32) Spawning counter threads...
(smfs-hierarchy-32) Done spawning counter threads.
(smfs-hierarchy-32) counters[0] < counters[1]
(smfs-hierarchy-32) counters[1] < counters[2]
(smfs-hierarchy-32) counters[2] < counters[3]
(smfs-hierarchy-32) counters[3] < counters[4]
(smfs-hierarchy-32) counters[4] < counters[5]
(smfs-hierarchy-32) counters[5] < counters[6]
(smfs-hierarchy-32) counters[6] < counters[7]
(smfs-hierarchy-32) end
EOF
pass;
