#ifndef __BLIB_H__
#define __BLIB_H__
/*
 * @(#)$Id: blib.h,v 1.2 2008/09/27 13:11:21 mark Exp mark $
 * $Log: blib.h,v $
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
#define _OSF_SOURCE     /* gets htons defines from machine/endian.h via in.h */
#endif /* __APPLE__ */

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
#include    <time.h>
typedef struct tm       tm_t;
#include    <unistd.h>
#include    <getopt.h>
#include    <locale.h>
#include    <sys/stat.h>
#include    <fcntl.h>
#include    <limits.h>


#include "strsep.h"
#include "util.h"
#include "parseslashcmd.h"
#include "execute_cmds.h"
#include "data_access.h"

#define    NL   (char *) NULL

typedef struct blib_global_s blib_global_t;
struct blib_global_s {
    char    *blibdb_name;   // getenv("BLIBDB")
    char    *blib_group;    // getenv("MYBLIB_GROUP") or hostname
    char    *default_media; // getenv("DAILYMEDIA") or "TZ89"
    char    *library_name;  // getenv("TAPELIB") or {nodename}TL1
    char    *progid;
    int	    debug;
    int	    quiet;
    int	    verbose;
};

void setup_blib(blib_global_t *blib_gp);

// ${BLIB_VOLUME}|${BLIB_FILENO}|${BLIB_STATE}|${BLIB_MEDIA}|${BLIB_USAGE}|${BLIB_GROUP}|${BLIB_LOCATION}|${BLIB_LIBRARYDATE}|${BLIB_RECORDDATE}|${BLIB_OFFSITEDATE}|${BLIB_EXPIREDATE}|${BLIB_DESC}

#endif /* __BLIB_H__ */
