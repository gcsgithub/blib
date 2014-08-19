#
#
#CFLAGS = -I. -g -D_XOPEN_SOURCE -D_XOPEN_SOURCE_EXTENDED
#CFLAGS = -I. -g -D_XOPEN_SOURCE=500

# Tru64
#CFLAGS = -I. -I/usr/local/include -g -DNEED_ASPRINTF -pthread -D_OSF_SOURCE -D_XOPEN_SOURCE_EXTENDED
#LDFLAGS = -lm -pthread

DEFS=-DNEED_ASPRINTF -DNEED_STRSEP  -DNEED_TIMEGM
CFLAGS = -I. -I/usr/local/include -g -arch i386 -arch x86_64 $(DEFS)
CFLAGS_DEPEND = -I. -I/usr/local/include -g  $(DEFS)
LDFLAGS = -lm

TARGETS = blib sqlite3 caltime ctime

HDRS = blib.h data_access.h do_cmd_reportbackup.h execute_cmds.h fileio.h list.h mail_utils.h \
              parseslashcmd.h split.h strsep.h util.h vasprintf.h copycheck.h mkargv.h timefunc.h

SRCS =  blib.c  data_access.c execute_cmds.c list.c parseslashcmd.c split.c strsep.c util.c vasprintf.c \
	        do_cmd_reportbackup.c fileio.c mail_utils.c copycheck.c mkargv.c timefunc.c timegm.c

OBJS =  blib.o  data_access.o execute_cmds.o list.o parseslashcmd.o split.o strsep.o util.o vasprintf.o \
	do_cmd_reportbackup.o fileio.o mail_utils.o copycheck.o mkargv.o timefunc.o sqlite3.o timegm.o

SQLITE_OBJS = sqlite3.o shell.o

###########################################
all:	$(TARGETS)
###########################################

.SUFFIXES: .c

.c.o:
	$(CC) $(CFLAGS) -c  $<


caltime: caltime.o
	$(CC) $(CFLAGS)  $< -o $@ $(LDFLAGS)

ctime: ctime.c vasprintf.o
	$(CC) $(CFLAGS)  $< vasprintf.o -o $@ $(LDFLAGS)

sqlite3.o: sqlite/sqlite3.c
	        $(CC) $(CFLAGS)  -c $<

shell.o: sqlite/shell.c
	        $(CC) $(CFLAGS) -DHAVE_READLINE=1 -c $< -o $@

sqlite3: $(SQLITE_OBJS)
	        $(CC) $(CFLAGS) $(SQLITE_OBJS) -o $@  $(LDFLAGS) -lreadline

blib: $(OBJS)
	$(CC) $(CFLAGS)	 $(OBJS) -o $@ $(LDFLAGS)


#$(OBJS): $(SRC)

#########################################
install:
	for exe in $(TARGETS) ; do \
		install -c -m 755 $$exe /usr/local/bin ; \
	done

clean:
	rm -f *.o $(TARGETS)

veryclean:
	rm -f *.o $(TARGETS) tags .depend

tags: $(SRCS) $(HDRS)
	ctags -wt $(SRCS) $(HDRS)

depend:	$(SRCS) $(HDRS) sqlite/shell.c          sqlite/sqlite3.c        sqlite/sqlite3.h        sqlite/sqlite3ext.h
	mkdep $(CFLAGS_DEPEND) $(SRCS)

-include .depend
