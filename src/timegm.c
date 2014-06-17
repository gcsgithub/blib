static const char *rcsid="@(#) $Id: timegm.c,v 1.1 2010/11/22 06:00:31 mark Exp $";

/* 
** $Log: timegm.c,v $
** Revision 1.1  2010/11/22 06:00:31  mark
** Initial revision
**
 */
#include <time.h>
#include <stdlib.h>

static const char *version()
{
    return(rcsid);
}

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
