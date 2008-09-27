#ifndef __BLIB_H__
#define __BLIB_H__
/*
 * @(#)$Id:$
 * @(#)$Log:$
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
#include    <ndbm.h>
#include    <sys/stat.h>

#include "strsep.h"
#include "util.h"
#include "parseslashcmd.h"

#define    NL   (char *) NULL

typedef struct blib_global_s blib_global_t;
struct blib_global_s {
    char    *blibdb_name;   // getenv("BLIBDB")
    char    *blib_group;    // getenv("MYBLIB_GROUP") or hostname
    char    *default_media; // getenv("DAILYMEDIA") or "TZ89"
    char    *library_name;  // getenv("TAPELIB") or {nodename}TL1
};

void setup_blib(blib_global_t *blib_gp);

// ${BLIB_VOLUME}|${BLIB_FILENO}|${BLIB_STATE}|${BLIB_MEDIA}|${BLIB_USAGE}|${BLIB_GROUP}|${BLIB_LOCATION}|${BLIB_LIBRARYDATE}|${BLIB_RECORDDATE}|${BLIB_OFFSITEDATE}|${BLIB_EXPIREDATE}|${BLIB_DESC}

typedef struct blib_vol_s   blib_vol_t;
struct blib_vol_s {
    char    *blib_volume;   // should be 1-13 chars typically a barcode would be 7 chars XXX999X
    int	    blib_fileno;    // current max file no used
    char    blib_state;	    // A,F ALLOCATED, FREE
    char    *blib_media;    // eg TZ89
    int	    blib_usage;	    // record of how many times we have written this tape during its life time
    char    *blib_group;    // what group aka pool does this take belong to
    char    *blib_location; // where is it
    time_t  blib_library_date; // when did this volume get added to blib should represent purchase date
    time_t  blib_recorddate;	// what was the date we wrote this backup
    time_t  blib_offsitedate;	// nominally the same as record date +1days
    time_t  blib_expiredate;	// date the tape will be ready to expire and reuse
    char    *blib_desc;	    // description of this backup
    
};
typedef	struct blib_evt_s blib_evt_t;
struct blib_evt_s {
    blib_vol_t	*blib_vol_p;	// pointer to the blib volume structure
    time_t  blib_recorddate;	// this will be the completion time for this event not the same of that in the volume structure
    int	    fileno;		// file number on tape of this file set
    char    *fsetname;		// name of this fileset eg "/usr"
};


#endif /* __BLIB_H__ */
