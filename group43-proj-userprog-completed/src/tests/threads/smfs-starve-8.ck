# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(smfs-starve-8) begin
(smfs-starve-8) Spawning competitor threads...
(smfs-starve-8) Done spawning competitor threads.
(smfs-starve-8) Low-priority thread got to run!
(smfs-starve-8) end
EOF
pass;
