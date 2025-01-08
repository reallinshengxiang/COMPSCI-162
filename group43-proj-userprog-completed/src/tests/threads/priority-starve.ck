# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(priority-starve) begin
(priority-starve) The medium-priority thread has been given a chance to run!
(priority-starve) The medium-priority thread has been preempted.
(priority-starve) Thread main done!
(priority-starve) end
EOF
pass;
