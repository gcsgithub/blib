$ cc  blib.c
$ cc  data_access.c
$ cc  execute_cmds.c
$ cc  list.c
$ cc  parseslashcmd.c
$ cc  split.c
$ cc  util.c
$ cc  vasprintf.c
$ cc  do_cmd_reportbackup.c
$ cc  fileio.c
$ cc  mail_utils.c
$ cc  copycheck.c
$ cc  mkargv.c
$ cc  timefunc.c
$ cc  sqlite3.c
$ !
$ link /exe=blib.exe    blib.obj  data_access.obj execute_cmds.obj list.obj parseslashcmd.obj split.obj util.obj vasprintf.obj do_cmd_reportbackup.obj fileio.obj mail_utils.obj copycheck.obj mkargv.obj timefunc.obj sqlite3.obj /exe=blib.exe /THREADS_ENABLE
$! 
$ cc shell.c
$ link  sqlite3.o shell.o /exe=sqlite3.exe
$ cc  caltime.c
$ link   caltime.o /exe=caltime.exe
$ cc  ctime.c
$ link  vasprintf.obj /exe=ctime.exe
