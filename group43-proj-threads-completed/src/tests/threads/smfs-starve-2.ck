# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(smfs-starve-2) begin
(smfs-starve-2) Spawning competitor threads...
(smfs-starve-2) Done spawning competitor threads.
(smfs-starve-2) Low-priority thread got to run!
(smfs-starve-2) end
EOF
pass;
