# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(join-fail) begin
(join-fail) Main starting
(join-fail) Thread finished
(join-fail) Finished joining
(join-fail) Main regained control
(join-fail) Finished joining
(join-fail) Main regained control
(join-fail) Finished self joining
(join-fail) Main finishing
(join-fail) end
join-fail: exit(0)
EOF
pass;
