# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(sema-wait-many) begin
(sema-wait-many) Main starting
(sema-wait-many) 1 threads are now ready
(sema-wait-many) 2 threads are now ready
(sema-wait-many) 3 threads are now ready
(sema-wait-many) 4 threads are now ready
(sema-wait-many) 5 threads are now ready
(sema-wait-many) 6 threads are now ready
(sema-wait-many) 7 threads are now ready
(sema-wait-many) 8 threads are now ready
(sema-wait-many) 9 threads are now ready
(sema-wait-many) 10 threads are now ready
(sema-wait-many) Main finishing
(sema-wait-many) end
sema-wait-many: exit(0)
EOF
pass;
