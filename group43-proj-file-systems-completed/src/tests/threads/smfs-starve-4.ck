# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(smfs-starve-4) begin
(smfs-starve-4) Spawning competitor threads...
(smfs-starve-4) Done spawning competitor threads.
(smfs-starve-4) Low-priority thread got to run!
(smfs-starve-4) end
EOF
pass;
