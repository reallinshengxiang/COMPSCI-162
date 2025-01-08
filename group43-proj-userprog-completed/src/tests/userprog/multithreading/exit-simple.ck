# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(exit-simple) begin
(exit-simple) Main started.
(exit-simple) Thread ran.
(exit-simple) Main finished.
(exit-simple) end
exit-simple: exit(0)
EOF
pass;
