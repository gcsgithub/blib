#ifndef __BLIB_H__
#define __BLIB_H__
/*
 * @(#)$Id: blib.h,v 1.6 2013/01/21 16:54:06 mark Exp $
 * $Log: blib.h,v $
 * Revision 1.6  2013/01/21 16:54:06  mark
 * MG add date_width to global structure for support of -w and variable date width
 *
 * Revision 1.5  2011/04/11 03:51:05  mark
 * add includelogs
 *
 * Revision 1.4  2010/11/16 04:10:58  mark
 * rc1
 *
 * Revision 1.3  2008/10/20  13:01:36  mark
 * checkpoint
 *
 * Revision 1.2  2008/09/27 13:11:21  mark
 * initial checkin
 *
 * Revision 1.1  2008/09/27 13:05:59  mark
 * Initial revision
 *
 *  blib.h
 *  blib
 *
 *  Created by mark on 26/09/2008.
 *  Copyright 2008 Garetech Computer Solutions. All rights reserved.
 *
 */

#ifndef __APPLE__
char *strsep(char **stringp, const char *delim);
#endif /* ! __APPLE__ */

#include    <arpa/inet.h>
#include    <ctype.h>
#include    <errno.h>
#include    <math.h>
#include    <netdb.h>
#include    <netinet/in.h>
#include    <regex.h>
#include    <setjmp.h>
#include    <signal.h>
#include    <stdarg.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <sys/fcntl.h>
#include    <sys/param.h>
#include    <sys/socket.h>
#include    <sys/types.h>
#include    <sys/uio.h>
#include    <sys/wait.h>
#include    <syslog.h>
#include    <unistd.h>
#ifndef __hpux
#include    <getopt.h>
#endif
#include    <locale.h>
#include    <sys/stat.h>
#include    <fcntl.h>
#include    <limits.h>


#include "strsep.h"
#include "util.h"
#include "fileio.h"

#define    NL   (char *) NULL

typedef struct blib_global_s blib_global_t;
struct blib_global_s {
    char    	 *progid;
    int	       	 debug;
    int	   	     quiet;
    int	   	     verbose;
    fio_t	     *blib_log;
    uint32_t	 volumes_inlib;
    uint32_t	 volumes_free;
    uint32_t 	 volumes_allocated;
    uint32_t	 volumes_other;
    files_t	     *includelogs;
    unsigned int date_width;
};

// ${BLIB_VOLUME}|${BLIB_FILENO}|${BLIB_STATE}|${BLIB_MEDIA}|${BLIB_USAGE}|${BLIB_GROUP}|${BLIB_LOCATION}|${BLIB_LIBRARYDATE}|${BLIB_RECORDDATE}|${BLIB_OFFSITEDATE}|${BLIB_EXPIREDATE}|${BLIB_DESC}

#define	COPYRIGHT	"Copyright (c) 2008-2011 Garetech Computer Solutions Pty Ltd"
#endif /* __BLIB_H__ */
