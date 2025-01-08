# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected (IGNORE_EXIT_CODES => 1, [<<'EOF']);
+ (testcoalesce) begin
+ (testcoalesce) Checking block statistics before creating "a"
+ (testcoalesce) create "a"
+ hdb1 (filesys): 357 reads, 286 writes
+ hda2 (scratch): 297 reads, 2 writes
+ (testcoalesce) open "a"
+ (testcoalesce) Checking block statistics after reading "a"
+ hdb1 (filesys): 357 reads, 286 writes
+ hda2 (scratch): 297 reads, 2 writes
+ (testcoalesce) Checking block statistics after re-reading "a"
+ hdb1 (filesys): 357 reads, 286 writes
+ hda2 (scratch): 297 reads, 2 writes
+ (testcoalesce) close "a"
+ (testcoalesce) end
EOF
pass;
