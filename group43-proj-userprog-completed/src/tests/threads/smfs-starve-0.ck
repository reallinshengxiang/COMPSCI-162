# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(smfs-starve-0) begin
(smfs-starve-0) Spawning competitor threads...
(smfs-starve-0) Done spawning competitor threads.
(smfs-starve-0) Low-priority thread got to run!
(smfs-starve-0) end
EOF
pass;
