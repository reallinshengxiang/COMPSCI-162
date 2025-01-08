# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(priority-starve-sema) begin
(priority-starve-sema) The medium-priority thread has been given a chance to run!
(priority-starve-sema) The low-priority thread has been given a chance to run!
(priority-starve-sema) The medium-priority thread has been preempted.
(priority-starve-sema) Thread main done!
(priority-starve-sema) end
EOF
pass;
