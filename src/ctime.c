//
//  Created by mark on 19/01/2013.
//  Copyright (c) 2013 mark. All rights reserved.
//

static char *rcsid="@(#) $Id: ctime.c,v 1.1 2013/01/20 08:42:57 mark Exp mark $";
/*
 * $Log: ctime.c,v $
 * Revision 1.1  2013/01/20 08:42:57  mark
 * Initial revision
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <stdarg.h>
#include <locale.h>
#include <time.h>

#include "vasprintf.h"

//typedef unsigned long long llu_t;
//typedef unsigned long	lu_t;


void    usage(const char *prg);
char    *newstr(char *fmt,...);
char    *fmt_tm(struct  tm *tmptr, size_t timestr_len, char *timestr);
char    *fmtctime(time_t now);
char    *fmtctime_gmt(time_t gmt);

static const char *version(void);

static const char *version(void)
{
    return(rcsid+9);
}


void    usage(const char *prg)
{
    fprintf(stderr,"# Usage: %s [-c] [-g] [-l] <ctime>\n", prg);
    fprintf(stderr,
            "#  -c  show the ctime value\n"
            "#  -g  show the time in GMT\n"
            "#  -l  show the localtime which is the default so this is only relevant to add to -g\n");
    exit(EINVAL);
}

static int CTIME_DBG;

int main(int argc,  char *argv[] /* , char *envp[] */)
{
    //int             err;
    int             dousage;
    char            c;
    extern char     *optarg;
    extern int      optind;
    char            *progid;
    time_t          ct;
    int             usecs;
    int             showctime, showgmt, showlocaltime, verbose;
    char            *timlocstr, *timgmtstr;
    char            *dotptr;
    
    
    (void)setlocale(LC_ALL, "");
    tzset();
    progid = newstr(argv[0]);
    
    verbose    = showctime  = showgmt    = showlocaltime = 0;
    timgmtstr  = timlocstr  = NULL;
    CTIME_DBG  = 0;
    dousage    = 0;
    
    optarg = NULL;
    while (!dousage && ((c = getopt(argc, argv, "cgldvVh?")) != -1)) {
        switch (c) {
            case 'c':
                showctime++;
                break;
                
            case 'g':
                showgmt++;
                break;
                
            case 'l':
                showlocaltime++;
                break;
                
 
            case 'V':
                fprintf(stderr, "# Version: %s\n", version());
                exit(0);
                break;
                
            case 'd':
                CTIME_DBG++; // fall though and increase verbose
            case 'v':
                verbose++;
                break;
                
            case 'h':
                usage(progid);
                exit(0);
                break;
                
            default:
                dousage++;
                break;
        }
    }
    
    argc -= optind; 
    argv += optind;
    
    if (dousage || argc <1) {
        usage(progid); // will exit via usage
        exit(EINVAL);
    }
    
    sscanf(argv[0],"%ld", &ct);
    if ((dotptr = index(argv[0], '.'))) {
	   dotptr++;
	   sscanf(dotptr, "%d", &usecs);
    }
    else {
	    usecs = 0;
    }
    
    if ((showlocaltime == 0) && ( showgmt == 0)) {
        showlocaltime++;  // show localtime by default if neither is selected
    }
    if (showlocaltime) {
        timlocstr = fmtctime(ct);
    }
    if (showgmt) {
        timgmtstr = fmtctime_gmt(ct);
    }
    
    if (showctime) {
        fprintf(stdout, "%s%ld.%02d ", (verbose)?"(ctim) ":"", ct, usecs);
    }
    
    if (timlocstr && timgmtstr) {
        fprintf(stdout,"%s%s.%02d :%s%s.%02d\n",
                    (verbose)?"(loc) ":"",  timlocstr, usecs,
                    (verbose)?"(gmt) ":"",  timgmtstr, usecs);
    }
    else if (timlocstr) {
        fprintf(stdout,"%s%s.%02d\n", (verbose)?"(loc) ":"",  timlocstr, usecs);
    }
    else {
        fprintf(stdout,"%s%s.%02d\n", (verbose)?"(gmt) ":"",  timgmtstr, usecs);
    }
    
}

char *fmtctime(time_t now)
{
    struct  tm      nowtm;
    static  char nowstr[32];
    
    bzero(&nowtm,sizeof(nowtm));
    nowtm.tm_isdst = -1;
    localtime_r(&now,&nowtm);
    
    return(fmt_tm(&nowtm, sizeof(nowstr), nowstr));
}

char *fmtctime_gmt(time_t gmt)
{
    struct  tm      nowgmt_tm;
    static  char nowstr[32];
    
    bzero(&nowgmt_tm,sizeof(nowgmt_tm));
    gmtime_r(&gmt,&nowgmt_tm);
    
    return(fmt_tm(&nowgmt_tm,  sizeof(nowstr), nowstr));
}



char *fmt_tm(struct  tm *tmptr, size_t timestr_len, char *timestr)
{
    
    if ( strftime(timestr,timestr_len,"%d-%b-%Y %T", tmptr ) == (size_t) 0) {
        return(NULL);
    }
    return ( timestr );
}


char    *newstr(char *fmt,...)
{
    va_list args;
    char    *dynstr;
    size_t  slen;
    
    if (!fmt) {
        fmt="";	// just in case they pass a null pointer
    }
    va_start(args,fmt);
    slen = VASPRINTF(&dynstr, fmt,args);
    va_end(args);
    if ( dynstr  == (char *) NULL ) {
        fprintf(stderr,"# Error allocating string %s size %llu\n", fmt,(llu_t) slen);
        errno = ENOMEM;
        exit(errno);
    }
    return(dynstr);
}
