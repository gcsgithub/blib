#ifdef NEEDASPRINTF
static char *rcsid = "@(#)$Id:$";
/*
 * $Log:$
 *
 *  vasprintf.c
 *  blib
 *
 *  Created by mark on 15/10/2008.
 *  Copyright 2008 Garetech Computer Solutions. All rights reserved.
 *
 */

#include "vasprintf.h"
static char *ver()
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
int    rlen;
// int	blen;
// char	bbuf[32768];
FILE *fdnull;
char *rval;
int err;
int	fmtlen;
    
    rval = (char *) NULL;
    rlen = -1;
    if ((fdnull = fopen("/dev/null", "w")) != (FILE *) NULL ) { // possibly not very efficent but it works ;)
	rlen = vfprintf(fdnull, fmt, args);
	fclose(fdnull);
	
//	blen = vsnprintf(bbuf,sizeof(bbuf), fmt, args);
//	fprintf(stderr, "my_asprintf:rlen:%d  blen: %d \"%s\"\n",rlen,  blen, bbuf);
	
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

#endif /* NEEDASPRINTF */
