# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_USER_FAULTS => 1, [<<'EOF']);
(create-many) begin
(create-many) Thread prints 162
(create-many) Thread prints 162
(create-many) Thread prints 162
(create-many) Thread prints 162
(create-many) Thread prints 162
(create-many) Thread prints 162
(create-many) Thread prints 162
(create-many) Thread prints 162
(create-many) Thread prints 162
(create-many) Thread prints 162
(create-many) Main finished
(create-many) end
create-many: exit(0)
EOF
pass;
