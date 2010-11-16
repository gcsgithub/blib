#include "vasprintf.h"
#ifdef NEED_ASPRINTF
static const char *rcsid = "@(#)$Id: vasprintf.c,v 2.5 2010/04/07 08:34:31 mark Exp mark $";
/*
 * $Log: vasprintf.c,v $
 * Revision 2.5  2010/04/07 08:34:31  mark
 * fix error with hangling args list
 *
 * Revision 2.4  2010/01/09 09:32:19  mark
 * RC4
 *
 * Revision 2.3  2009/12/31 05:54:13  mark
 * RC3
 *
 * Revision 1.10  2009/12/29 02:12:09  mark
 * RC2
 *
 * Revision 1.9  2009/12/28 16:20:45  mark
 * threadsRC1
 *
 * Revision 1.8  2009/01/20 02:11:54  mark
 * b4thread
 *
 * Revision 1.7  2009/01/07 23:09:51  mark
 * RC4
 *
 * Revision 1.6  2009/01/06 21:29:07  mark
 * RC3
 *
 * Revision 1.5  2009/01/06 01:19:55  mark
 * RC2
 *
 * Revision 1.4  2009/01/05 23:25:58  mark
 * MallocDBG
 *
 * Revision 1.3  2009/01/04 17:48:49  mark
 * RC1
 *
 * Revision 1.2  2008/12/29 21:09:29  mark
 * code complete debug/testing
 *
 * Revision 1.1  2008/12/15 06:56:36  mark
 * Initial revision
 *
 * Revision 1.1  2008/10/19  22:19:01  root
 * Initial revision
 *
 *
 *  vasprintf.c
 *  blib
 *
 *  Created by mark on 15/10/2008.
 *  Copyright 2008 Garetech Computer Solutions. All rights reserved.
 *
 */
#include "vasprintf.h"
static const char *ver()
{
    return(rcsid);
}

int my_asprintf(char **ret, char *fmt,  ... )
{
    va_list args;
    int rlen;
    
    rlen = -1;
    if (ret) {
        va_start(args,fmt);
        rlen = my_vasprintf(ret, fmt, args);
        va_end(args);
    }
    return(rlen);
}

int my_vasprintf(char **ret, char *fmt, va_list args)
{
    int	rlen;
    FILE	*fdnull;
    char	*rval;
    int	err;
    int	fmtlen;
    va_list tmpargs;
    
    
    rval = (char *) NULL;
    rlen = -1;
#ifdef __alpha
#define tmpargs args
#else
    va_copy(tmpargs, args); /* dont disturb the args list till we really use them */
#endif
    if ((fdnull = fopen("/dev/null", "w")) != (FILE *) NULL ) { /* possibly not very efficent but it works ;) */
        rlen = vfprintf(fdnull, fmt, tmpargs);
        fclose(fdnull);
        
        if (rlen == -1 ) {
            err=errno;
            fprintf(stderr, "Internal error in asprintf() error vfprintf /dev/null %d:%s\n", err, strerror(err));
            return(-1);	    
        }
        if ((rval = (char *) malloc(rlen+1)) == (char *) NULL ) {
            rlen = -1;
        } else {
            fmtlen = vsnprintf(rval,rlen+1, fmt, args);
        }
    } else {
        err=errno;
        fprintf(stderr, "Internal error in asprintf() error opening /dev/null %d:%s\n", err, strerror(err));
        return(-1);
    }
    if (ret) {
        *ret = rval;
    } else {
        if (rval) free(rval);
        rlen=-1;
    }
    return(rlen);
}

#endif /* NEED_ASPRINTF */
