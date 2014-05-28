A command line wrapper around the mincore(2) system call to examine file page caches.

Examples demonstrating usage:

$ # initially, file-to-examine does not have any content in the page cache:
$ pagecache file-to-examine
Page cache for 'file-to-examine' (file size=99007, page size=4096):
0000000000000000000000000
Summary: 0/25 (0%) pages cached

$ # read two first pages from file, after which these will probably be cached
$ dd if=file-to-examine of=/dev/null bs=8096 count=1 2> /dev/null

$ # verify that these two pages have been cached
$ pagecache file-to-examine
Page cache for 'file-to-examine' (file size=99007, page size=4096):
1100000000000000000000000
Summary: 2/25 (8%) pages cached

