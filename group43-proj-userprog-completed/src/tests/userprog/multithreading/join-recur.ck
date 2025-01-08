# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(join-recur) begin
(join-recur) Main starting
(join-recur) Thread 1 starting
(join-recur) Thread 2 starting
(join-recur) Thread 3 starting
(join-recur) Thread 4 starting
(join-recur) Thread 5 starting
(join-recur) Thread 6 starting
(join-recur) Thread 7 starting
(join-recur) Thread 8 starting
(join-recur) Thread 9 starting
(join-recur) Thread 10 starting
(join-recur) Thread 10 finishing
(join-recur) Thread 9 finishing
(join-recur) Thread 8 finishing
(join-recur) Thread 7 finishing
(join-recur) Thread 6 finishing
(join-recur) Thread 5 finishing
(join-recur) Thread 4 finishing
(join-recur) Thread 3 finishing
(join-recur) Thread 2 finishing
(join-recur) Thread 1 finishing
(join-recur) Main finishing
(join-recur) end
join-recur: exit(0)
EOF
pass;
