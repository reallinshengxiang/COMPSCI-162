# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(alarm-delay) begin
(alarm-delay) Thread with delay 100 (expected delay was 100, actual delay was 100).
(alarm-delay) Thread with delay 200 (expected delay was 200, actual delay was 200).
(alarm-delay) Thread with delay 300 (expected delay was 300, actual delay was 300).
(alarm-delay) Thread with delay 400 (expected delay was 400, actual delay was 400).
(alarm-delay) Thread with delay 500 (expected delay was 500, actual delay was 500).
(alarm-delay) Thread with delay 600 (expected delay was 600, actual delay was 600).
(alarm-delay) Thread with delay 700 (expected delay was 700, actual delay was 700).
(alarm-delay) Thread with delay 800 (expected delay was 800, actual delay was 800).
(alarm-delay) Thread with delay 900 (expected delay was 900, actual delay was 900).
(alarm-delay) Thread with delay 1000 (expected delay was 1000, actual delay was 1000).
(alarm-delay) end
EOF
pass;
