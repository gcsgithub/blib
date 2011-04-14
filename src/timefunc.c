static const char *rcsid="@(#) $Id: timefunc.c,v 1.1 2010/11/16 04:04:52 root Exp mark $";
/*
 *  timefunc.c
 *  blib
 *
 *  Created by mark on 31/10/2010.
 *  Copyright 2010 Garetech Computer Solutions. All rights reserved.
 *
 */

#include "timefunc.h"

static const char *ver()
{
    return(rcsid);
}

/////////////////////////////////////////////////////////////////////////////////////////////
time_t	*newdate_gmt(char *val)
{
    time_t  *dynstr;
    time_t   timeval;
    char    *timestr;
    
    ltrim(val, strlen(val));
    if (( dynstr = (time_t *) malloc(sizeof(time_t))) == (time_t *) NULL ) {
	fprintf(stderr,"#BLIB:  Out of memory allocating int %s\n", val);
	errno = ENOMEM;
	exit(errno);
    }
    
    timestr = val;
    if (memcmp(val,TIMELIT, sizeof(TIMELIT)-1)==0 ) {
	timestr = val+sizeof(TIMELIT)-1;
	timeval = atoi(timestr);
    } else {	
	timeval = scandate_gmt(timestr);
    }
    *(time_t *) dynstr = timeval;
    return(dynstr);
}

/////////////////////////////////////////////////////////////////////////////////////////////
time_t	scandate_gmt(char *datestr)
{
    tm_t    timetm;
    time_t  timec;
    int	err;
    
    timec=0;
    if (datestr && datestr[0] ) {
	bzero(&timetm, sizeof(tm_t));
	if ((strncasecmp(datestr, D_NEVER,sizeof(D_NEVER)) != 0 ) &&
	    (strncasecmp(datestr, D_VMS0, sizeof(D_VMS0)) !=0)) {
	    bzero(&timetm,sizeof(tm_t));
	    strptime(datestr, "%d-%b-%Y:%T", &timetm);
	    timetm.tm_isdst = -1; // ask mktime to work out the dst effects
	    if ( ( timec = timegm( &timetm)) == -1 ) {
		err = errno;
		if ( err != ERANGE ) {
		    fprintf(stderr,"#BLIB:  Error in parsing date %s :%s\n", datestr, strerror(err));
		    errno=err;
		    timec=0;
		}
	    }
	}
    }
    return(timec);
}

/////////////////////////////////////////////////////////////////////////////////////////////
time_t	now(void)
{      
    return( time((time_t*)0 ));
}

/////////////////////////////////////////////////////////////////////////////////////////////
time_t	nowgm(void)
{
    time_t nowtime;
    tm_t  *gm_now;
    
    nowtime=now();
    gm_now = gmtime(&nowtime);
    return( timegm(gm_now));
}

/////////////////////////////////////////////////////////////////////////////////////////////
datestr_t *copy_datestr(datestr_t *dst, datestr_t *src)
{
    if (dst) {
	if (src) {
	    strncpy(dst->str, src->str,sizeof(datestr_t));
	} else {
	    bzero(dst, sizeof(datestr_t));
	}
    }
    return(dst);
}

// 12-APR-2022:21:49:12.00
datestr_t *copy_datestr_time(datestr_t *dst, datestr_t *src)
{
    char *colon;
    char *dot;
    
    if (dst) {
        if (src) {
            colon = index(src->str, ':');
            if (colon) {
                colon++;
                strncpy(dst->str, colon,sizeof(datestr_t));
                dot = rindex(dst->str, '.');
                if (dot) *dot='\0';
            } else {
                colon = src->str;
                strncpy(dst->str, colon,sizeof(datestr_t));
            }
            
        } else {
            bzero(dst, sizeof(datestr_t));
        }
    }
    return(dst);
}


/////////////////////////////////////////////////////////////////////////////////////////////
#ifdef __NEED_TIMEGM__
time_t timegm(struct tm *tm)
{
    time_t ret;
    char *tz;
    
    tz = getenv("TZ");
    setenv("TZ", "", 1);
    tzset();
    
    ret = mktime(tm);
    
    if (tz) setenv("TZ", tz, 1);
    else    unsetenv("TZ");
    tzset();
    return ret;
}

#endif /* __NEED_TIMEGM__ */

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

char *fmtctime(time_t ctime)
{
    tm_t      timetm;
    static  datestr_t timestr;
    
    if (ctime == 0 ) {
    	snprintf((char *) &timestr,sizeof(timestr), D_NEVER);
    } else {
	bzero(&timetm,sizeof(tm_t));
	
	localtime_r(&ctime,&timetm);
	if ( strftime(timestr.str,sizeof(timestr),"%d-%b-%Y:%T.00",&timetm ) == (size_t) 0) {
	    return(NULL);
	}
	cvt2uppercase(timestr.str);
    }
    return ( timestr.str );
}

/////////////////////////////////////////////////////////////////////////////////////////////
void	printlitdate(FILE *fd, time_t ctim)
{

	fprintf(fd, "%s%d",TIMELIT , (int) ctim);

}