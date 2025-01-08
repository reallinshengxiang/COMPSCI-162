# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(smfs-starve-1) begin
(smfs-starve-1) Spawning competitor threads...
(smfs-starve-1) Done spawning competitor threads.
(smfs-starve-1) Low-priority thread got to run!
(smfs-starve-1) end
EOF
pass;
