# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(smfs-starve-256) begin
(smfs-starve-256) Spawning competitor threads...
(smfs-starve-256) Done spawning competitor threads.
(smfs-starve-256) Low-priority thread got to run!
(smfs-starve-256) end
EOF
pass;
