# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(synch-many) begin
(synch-many) Main starting
(synch-many) Main acquired locks and downed semaphores
(synch-many) Thread started
(synch-many) Main releasing all locks and semaphores
(synch-many) Thread acquired all locks and downed semaphores
(synch-many) Thread finished
(synch-many) Main finishing
(synch-many) end
synch-many: exit(0)
EOF
pass;
