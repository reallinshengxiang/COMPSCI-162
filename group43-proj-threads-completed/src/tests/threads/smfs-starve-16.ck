# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(smfs-starve-16) begin
(smfs-starve-16) Spawning competitor threads...
(smfs-starve-16) Done spawning competitor threads.
(smfs-starve-16) Low-priority thread got to run!
(smfs-starve-16) end
EOF
pass;
