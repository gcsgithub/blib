# @(#) $Id:$
# $Log:$
#
#CFLAGS = -I. -g -D_XOPEN_SOURCE -D_XOPEN_SOURCE_EXTENDED
#CFLAGS = -I. -g -D_XOPEN_SOURCE=500
#CFLAGS = -I. -I/usr/local/include -g -DNEED_ASPRINTF -arch ppc -arch i386
CFLAGS = -I. -I/usr/local/include -g -arch i386 -arch x86_64
CFLAGS_DEPEND = -I. -I/usr/local/include -g 
LDFLAGS = -lm -L/usr/local/lib -lsqlite3

TARGET = blib

HDRS = blib.h data_access.h do_cmd_reportbackup.h execute_cmds.h fileio.h list.h mail_utils.h \
       parseslashcmd.h split.h strsep.h util.h vasprintf.h copycheck.h mkargv.h timefunc.h

SRCS =  blib.c  data_access.c execute_cmds.c list.c parseslashcmd.c split.c strsep.c util.c vasprintf.c \
       	do_cmd_reportbackup.c fileio.c mail_utils.c copycheck.c mkargv.c timefunc.c

OBJS =  blib.o  data_access.o execute_cmds.o list.o parseslashcmd.o split.o strsep.o util.o vasprintf.o \
	do_cmd_reportbackup.o fileio.o mail_utils.o copycheck.o mkargv.o timefunc.o


all:	$(TARGET)

install: all
	install -c -g tape -m 750 blib /usr/local/etc



$(TARGET): $(OBJS)
	$(CC) $(CFLAGS)	 $(OBJS) -o $(TARGET) $(LDFLAGS)


$(OBJS): $(SRC)

.c.o:
	$(CC) $(CFLAGS) -c $*.c

clean:
	rm -f *.o $(TARGET)

veryclean:
	rm -f *.o $(TARGET) tags .depend

tags: $(SRCS) $(HDRS)
	ctags -wt $(SRCS) $(HDRS)

depend:	$(SRCS) $(HDRS)
	mkdep $(CFLAGS_DEPEND) $(SRCS)
