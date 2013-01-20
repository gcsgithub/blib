static const char *rcsid="@(#) $Id: timefunc.c,v 1.2 2011/04/14 02:34:02 mark Exp mark $";
/*
 *  timefunc.c
 *  blib
 *
 *  Created by mark on 31/10/2010.
 *  Copyright 2010 Garetech Computer Solutions. All rights reserved.
 *
 */

#include "timefunc.h"

/////////////////////////
static const char *ver()
{
    return(rcsid);
}

//////////////////////////////////////////////
void	printlitdate(FILE *fd, blib_tim_t ctim)
{
    
	fprintf(fd, "%s%f",TIMELIT , ctim);
    
}

/////////////////////
blib_tim_t	now(void)
{
    blib_tim_t      rval;
    struct timeval  tv;
    struct timezone tz;
    struct tm       timetm;
    int             err;
    
    if (gettimeofday(&tv, &tz) < 0 ) { // value is in GMT
        err = errno;
        fprintf(stderr, "# gettimeofday failed: %d:%s\n",err, strerror(err));
        exit(err);
    }
    localtime_r(&tv.tv_sec, &timetm); // adjust GMT to localtime
    tv.tv_sec = mktime(&timetm);
    rval = time_cvt_tv_to_blib(&tv);
    
    return(rval);
}

/////////////////////////////////////////////////////////////////////////////////////////////
blib_tim_t	nowgm(void)
{
    blib_tim_t rval;
    struct timeval  tv;
    struct timezone tz;
    int err;
    
    if (gettimeofday(&tv, &tz) < 0 ) { // value is in GMT
        err = errno;
        fprintf(stderr, "# gettimeofday failed: %d:%s\n",err, strerror(err));
        exit(err);
    }
    
    rval = time_cvt_tv_to_blib(&tv);
    
    return(rval);
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


void time_cvt_blib_to_tv(blib_tim_t seconds, struct timeval *tv)
{
    blib_tim_t  remaindersecs;
    if (tv) {
        tv->tv_sec    = (int) seconds;
        remaindersecs = (seconds - (blib_tim_t) tv->tv_sec);
        tv->tv_usec   = (int) (remaindersecs *1000000.0);
    }
}

blib_tim_t time_cvt_tv_to_blib(struct timeval *tv)
{
    blib_tim_t rval;
    if (tv) {
        rval = (blib_tim_t) tv->tv_sec + ((blib_tim_t) tv->tv_usec / 1000000.0);
    }
    else {
        rval = 0.0;
    }
    return(rval);
}

datestr_t *time_cvt_blib_to_str(blib_tim_t tod)
{
    const   char *mths[] = {"Jan","Feb","Mar","Apr","May","Jun", "Jul","Aug","Sep","Oct","Nov","Dec"};
    // const   char *days[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    
    tm_t             timetm;
    static datestr_t timestr;
    struct timeval   tv;
    
    datestr_t    msecs_str;
    double       msecs;
    unsigned int msecs_int;
    
    datestr_t    hsecs_str;
    double       hsecs;
    unsigned int hsecs_int;
    
    
    if (tod == 0 ) {
    	snprintf(timestr.str,sizeof(timestr), D_NEVER);
    }
    else {
        
        time_cvt_blib_to_tv(tod, &tv);
        
        bzero(&timetm, sizeof(tm_t));
        localtime_r(&tv.tv_sec,&timetm);
        
        msecs_int = (int) (((((double) tv.tv_usec / 1000000.0) + 0.0005)*1000.0));
        msecs     = msecs_int / 1000.0;
        snprintf(msecs_str.str,sizeof(msecs_str), "%.3f", msecs);
        
        hsecs_int = (int) (((((double) tv.tv_usec / 1000000.0) + 0.005)*100.0));
        hsecs     = hsecs_int / 100.0;
        snprintf(hsecs_str.str,sizeof(hsecs_str), "%.2f", hsecs);
        
        
        sprintf(
                timestr.str,
                "%02d-%3s-%04d:%02d:%02d:%02d.%s",
                timetm.tm_mday,
                mths[timetm.tm_mon],
                timetm.tm_year+1900,
                timetm.tm_hour,
                timetm.tm_min,
                timetm.tm_sec,
                &hsecs_str.str[2]
                );
        
    }
    return(&timestr);
}

blib_tim_t time_cvt_str_to_blib(datestr_t *datestr)
{
    tm_t        timetm;
    blib_tim_t  rval;
    int         err;
    char        *decimalptr;
    int         hsec;
    
    rval=0;
    decimalptr = NULL;
    if (datestr && datestr->str[0] ) {
        bzero(&timetm, sizeof(tm_t));
        if ((strncasecmp(datestr->str, D_NEVER,sizeof(D_NEVER)) != 0 ) &&
            (strncasecmp(datestr->str, D_VMS0, sizeof(D_VMS0)) !=0)) {
            decimalptr = strptime(datestr->str, "%d-%b-%Y:%T", &timetm);
            // TODO: check this is correct
            timetm.tm_isdst = -1; // ask mktime to work out the dst effects
            if ( ( rval = mktime( &timetm)) == -1 ) {
                err = errno;
                if ( err != ERANGE ) {
                    fprintf(stderr,"#BLIB:  Error in parsing date %s :%s\n", datestr->str, strerror(err));
                    errno=err;
                    rval=0;
                }
            }
            else {
                if (decimalptr) {
                    if (*decimalptr == '.') {
                        decimalptr++;
                        sscanf(decimalptr, "%u", &hsec);
                        rval += ((blib_tim_t) hsec /100.0 );
                    }
                }
            }
        }
    }
    return(rval);
}

blib_tim_t  *new_time_blib_from_str(datestr_t *str)
{
    //
    // this is getting the value as if it was a GMT time
    //
    blib_tim_t *rval;
    if ((rval = malloc(sizeof(blib_tim_t))) == (blib_tim_t *) NULL) {
        fprintf(stderr, "# Error allocating memory for new time object\n");
        exit(ENOMEM);
    }
    *rval = time_cvt_str_to_blib(str);
    return(rval);
}


