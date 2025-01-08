# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(create-simple) begin
(create-simple) Main started.
(create-simple) Thread ran.
(create-simple) Main finished.
(create-simple) end
create-simple: exit(0)
EOF
pass;
