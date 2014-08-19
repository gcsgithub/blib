static const char *rcsid="@(#)$Id: caltime.c,v 1.3 2008/05/13 04:40:25 root Exp $";
/* Calc time functions
 ** $Log: caltime.c,v $
 ** Revision 1.3  2008/05/13 04:40:25  root
 ** add -c to display c time as well
 **
 ** Revision 1.2  2000/09/06 02:48:37  garrettm
 ** add Month Sep :)
 **
 * Revision 1.1  2000/04/07  05:39:54  garrettm
 * Initial revision
 *
 **
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>


#ifndef FALSE
#define FALSE   0
#endif
#ifndef TRUE
#define TRUE !FALSE
#endif


#include <time.h>
#include <sys/time.h>

typedef __int64_t     usecs_t;

typedef struct {
    struct timeval  tv;
    struct timezone tz;
} timeofday_t;

typedef enum {
    TF_YYYYmmmDDHHMMSS,
    TF_DAY_DDmmm_YYYY_HHMMSS,
    TF_YYYY_mm_DD_HHMMSS,
    TF_YYYY_mm_DD_HHMMSS_CCC,
    TF_VMS, 
    TF_CSV
} fmt_time_e;

char    *fmt_timeval(fmt_time_e fmt, timeofday_t *tod);
char    *timeofday(fmt_time_e fmt);


typedef unsigned long long llu_t;
typedef unsigned long	lu_t;


char    *newstr(char *fmt,...);

static const char *version(void);
static void        usage(const char *prg);

static const char *version(void)
{
    return(rcsid+9);
}

void    usage(const char *prg)
{
    fprintf(stderr,"# Usage: %s [-c] <delta days>\n", prg);
    exit(EINVAL);
}

int CTIME_DBG;

int main(int argc,char *argv[])
{
    int         daydelta  = 0;
    int         showctime = 0;
    int         verbose   = 0;
    int         dousage   = 0;
    const char  *progid;
    char        timbuf[64];
    int         found_delta=0;
    
    char        *timstr;
    timeofday_t tod;

    const char        *arg;
    
    progid = *argv++;
    for (; (arg = *argv); argv++) {

        if (arg[0] == '-') {
            switch(arg[1]) {
                case 'c':
                    showctime++;
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
                    if (isdigit(arg[1])) {
                        daydelta= atoi(arg);
                        found_delta++;
                    }
                    else {
                        dousage++;
                    }
                    break;
            }
        }
        else {
            if (isdigit(arg[0])) {
                daydelta= atoi(arg);
                found_delta++;
            }
        }
    }

    
    if (dousage || argc <1 || !found_delta) {
        usage(progid); // will exit via usage
        exit(EINVAL);
    }
    
    gettimeofday(&tod.tv, &tod.tz);
    
    tod.tv.tv_sec += (daydelta * 24 * 60 * 60 );
    

    timstr = fmt_timeval(TF_VMS, &tod);
    
    if (showctime) {
        int 	hsecs_int = (int) (((((double) tod.tv.tv_usec / 1000000.0) + 0.005)*100.0));
        double	hsecs     = hsecs_int / 100.0;
        sprintf(timbuf, "%.02f", hsecs);
        fprintf(stdout, "%ld%s ", tod.tv.tv_sec, timbuf+1);
    }
    fprintf(stdout,"%s\n", timstr);
    
}

char *fmt_timeval(fmt_time_e fmt, timeofday_t *tod)
{
    const   char *mths[] = {"Jan","Feb","Mar","Apr","May","Jun", "Jul","Aug","Sep","Oct","Nov","Dec"};
    const   char *days[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    
    struct  tm  tm;
    struct  tm  *tmp = &tm;
    struct  timeval *tv;
    
    char         timebuf[64];
    char         msecs_str[64];
    double       msecs;
    unsigned int msecs_int;
    
    char         hsecs_str[64];
    double       hsecs;
    unsigned int hsecs_int;
    
    if (tod) {
        tv = &tod->tv;
        
        localtime_r(&tv->tv_sec, tmp);
        
        msecs_int = (int) (((((double) tv->tv_usec / 1000000.0) + 0.0005)*1000.0));
        msecs     = msecs_int / 1000.0;
        snprintf(msecs_str,sizeof(msecs_str), "%.3f", msecs);
        
        hsecs_int = (int) (((((double) tv->tv_usec / 1000000.0) + 0.005)*100.0));
        hsecs     = hsecs_int / 100.0;
        snprintf(hsecs_str,sizeof(hsecs_str), "%.2f", hsecs);
        
        switch (fmt ) {
            case TF_YYYYmmmDDHHMMSS:
                sprintf(timebuf,"%04d-%s-%02d:%02d:%02d:%02d", tmp->tm_year+1900,
                        mths[tmp->tm_mon], tmp->tm_mday,
                        tmp->tm_hour,tmp->tm_min,tmp->tm_sec);
                break;
            case TF_DAY_DDmmm_YYYY_HHMMSS:
#ifdef __alpha__
                sprintf(timebuf,"%3s, %02d %3s %04d %02d:%02d:%02d %+04d (%s)",
                        days[tm->tm_wday],
                        tm->tm_mday,
                        mths[tm->tm_mon],
                        tm->tm_year+1900,
                        tm->tm_hour,
                        tm->tm_min,
                        tm->tm_sec,
                        (tm->tm_gmtoff/36),
                        tm->tm_zone
                        );
#else
                sprintf(timebuf,"%3s, %02d %3s %04d %02d:%02d:%02d",
                        days[tmp->tm_wday],
                        tmp->tm_mday,
                        mths[tmp->tm_mon],
                        tmp->tm_year+1900,
                        tmp->tm_hour,
                        tmp->tm_min,
                        tmp->tm_sec
                        );
#endif
                break;
            case TF_YYYY_mm_DD_HHMMSS:
                sprintf(timebuf,"%04d-%02d-%02d:%02d:%02d:%02d",
                        tmp->tm_year+1900,
                        tmp->tm_mon+1,
                        tmp->tm_mday,
                        tmp->tm_hour,tmp->tm_min,tmp->tm_sec);
                break;
            case TF_YYYY_mm_DD_HHMMSS_CCC:
                sprintf(timebuf,"%04d-%02d-%02d:%02d:%02d:%02d.%s",
                        tmp->tm_year+1900,
                        tmp->tm_mon+1,
                        tmp->tm_mday,
                        tmp->tm_hour,tmp->tm_min,tmp->tm_sec,
                        msecs_str+2);
                
                break;
            case TF_VMS:
                sprintf(timebuf,"%02d-%3s-%04d:%02d:%02d:%02d.%s",
                        tmp->tm_mday,
                        mths[tmp->tm_mon],
                        tmp->tm_year+1900,
                        tmp->tm_hour,
                        tmp->tm_min,
                        tmp->tm_sec,
                        hsecs_str+2);
                break;
                
            case TF_CSV: // yyyy-MM-dd HH:mm:ss
                sprintf(timebuf,"%04d-%02d-%02d %02d:%02d:%02d",
                        tmp->tm_year+1900,
                        tmp->tm_mon+1,
                        tmp->tm_mday,
                        tmp->tm_hour,
                        tmp->tm_min,
                        tmp->tm_sec);
                break;
        }
        return(strdup(timebuf));
    }
    else {
        return(strdup("nulltime"));
    }
}


