# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(sema-wait) begin
(sema-wait) Main starting
(sema-wait) Thread 1 starting
(sema-wait) Thread 2 starting
(sema-wait) Thread 3 starting
(sema-wait) Thread 4 starting
(sema-wait) Thread 5 starting
(sema-wait) Thread 6 starting
(sema-wait) Thread 7 starting
(sema-wait) Thread 8 starting
(sema-wait) Thread 9 starting
(sema-wait) Thread 10 starting
(sema-wait) Thread 10 finishing
(sema-wait) Thread 9 finishing
(sema-wait) Thread 8 finishing
(sema-wait) Thread 7 finishing
(sema-wait) Thread 6 finishing
(sema-wait) Thread 5 finishing
(sema-wait) Thread 4 finishing
(sema-wait) Thread 3 finishing
(sema-wait) Thread 2 finishing
(sema-wait) Thread 1 finishing
(sema-wait) Main finishing
(sema-wait) end
sema-wait: exit(0)
EOF
pass;
