# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(pcb-syn) begin
(pcb-syn) Main starting
(pcb-syn) Main finished
(pcb-syn) end
pcb-syn: exit(0)
EOF
pass;
