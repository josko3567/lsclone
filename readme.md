`lsclone`
---------
Just a clone of the Linux `ls -al` written in ~300 lines of C.

Made for a college task.

Example
-------
``` hl_lines="1"
./lsclone
Found 10 entries in "/home/josko-k/pgit/lsclone", biggest entry is 18424 bytes.
drwxr-xr-x. 1 foo bar   124 Jan 23 18:18 .
drwxr-xr-x. 1 foo bar   152 Jan 23 15:10 ..
-rw-r--r--. 1 foo bar   203 Jan 23 18:18 makefile
drwxr-xr-x. 1 foo bar    26 Jan 23 14:23 .vscode
drwxr-xr-x. 1 foo bar    40 Jan 23 14:24 examples
-rw-r--r--. 1 foo bar    16 Jan 23 14:24 .gitignore
drwxr-xr-x. 1 foo bar    98 Jan 23 14:24 .git
-rw-r--r--. 1 foo bar   111 Jan 23 18:17 readme.md
-rw-r--r--. 1 foo bar  8865 Jan 23 17:24 lsclone.c
-rwxr-xr-x. 1 foo bar 18424 Jan 23 18:18 lsclone
```