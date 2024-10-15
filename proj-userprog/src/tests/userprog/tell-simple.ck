# -*- perl -*-
use strict;
use warnings;
use tests::tests;

check_expected ([<<'EOF']);
(tell-simple) begin
(tell-simple) create 'testfile.txt'
(tell-simple) open 'testfile.txt'
(tell-simple) reopen 'testfile.txt'
(tell-simple) tell at position 0
(tell-simple) tell after seek to position 10
(tell-simple) tell after read 5 bytes
(tell-simple) tell after seek to position 5
(tell-simple) tell at the end of the file
(tell-simple) end
tell-simple: exit(0)
EOF
pass;

