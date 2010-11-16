#ifndef __TIMEFUNC_H__
#define __TIMEFUNC_H__
/*
 *  timefunc.h
 *  blib
 *
 *  Created by mark on 31/10/2010.
 *  Copyright 2010 Garetech Computer Solutions. All rights reserved.
 * $Log:$
 *
 */
#include "util.h"
#include <time.h>
typedef struct tm tm_t;


#define	D_NEVER	"NotSet"
#define D_VMS0	"17-NOV-1858 00:00:00.00"
#define TIMELIT "ctime:"

typedef struct {
    char str[64];
} datestr_t;


time_t	  *newdate_gmt(char *val);
time_t	  scandate_gmt(char *datestr);
char	  *fmtctime(time_t ctime);
time_t	  now(void);
time_t	  nowgm(void);

#ifdef __NEED_TIMEGM__
time_t    timegm(struct tm *tm)
#endif /* __NEED_TIMEGM__ */

datestr_t *copy_datestr(datestr_t *dst, datestr_t *src);
void	  printlitdate(FILE *fd, time_t ctim);

#endif /* __TIMEFUNC_H__ */