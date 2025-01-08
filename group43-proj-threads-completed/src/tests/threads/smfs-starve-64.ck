# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(smfs-starve-64) begin
(smfs-starve-64) Spawning competitor threads...
(smfs-starve-64) Done spawning competitor threads.
(smfs-starve-64) Low-priority thread got to run!
(smfs-starve-64) end
EOF
pass;
