#ifndef __TIMEFUNC_H__
#define __TIMEFUNC_H__
/*
 *  timefunc.h
 *  blib
 *
 *  Created by mark on 31/10/2010.
 *  Copyright 2010 Garetech Computer Solutions. All rights reserved.
 * $Log: timefunc.h,v $
 * Revision 1.3  2013/01/20 10:16:48  mark
 * >> MG change time_t to blib_tim_t for new decimal time and required function changes
 *
 * Revision 1.2  2011/04/14 02:34:23  mark
 * added copy_datestr_time used in reporting error lines in html
 *
 * Revision 1.1  2010/11/16 04:04:08  root
 * Initial revision
 *
 *
 */
#include "util.h"
#include <time.h>
#include <sys/time.h>

typedef struct tm tm_t;
typedef double blib_tim_t;


#define	D_NEVER	"NotSet"
#define D_VMS0	"17-NOV-1858 00:00:00.00"
#define TIMELIT "ctime:"

typedef struct {
    char str[64];
} datestr_t;

datestr_t     *time_cvt_blib_to_str(blib_tim_t ctime);
blib_tim_t    time_cvt_str_to_blib(datestr_t *str);
blib_tim_t	  now(void);
blib_tim_t	  nowgm(void);

#ifdef __NEED_TIMEGM__
blib_tim_t    timegm(struct tm *tm)
#endif /* __NEED_TIMEGM__ */

datestr_t   *copy_datestr(datestr_t *dst, datestr_t *src);
datestr_t   *copy_datestr_time(datestr_t *dst, datestr_t *src);
void        printlitdate(FILE *fd, blib_tim_t ctim);


blib_tim_t  *new_time_blib_from_str(datestr_t *str);
datestr_t   *time_cvt_blib_to_str(blib_tim_t ctime);
blib_tim_t   time_cvt_str_to_blib(datestr_t *str);

blib_tim_t  time_cvt_tv_to_blib(struct timeval *tv);
void        time_cvt_blib_to_tv(blib_tim_t seconds, struct timeval *tv);

#endif /* __TIMEFUNC_H__ */