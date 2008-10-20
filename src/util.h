#ifndef __UTIL_H__
#define __UTIL_H__
/*
 * @(#) $Id: util.h,v 1.2 2008/09/27 13:11:22 mark Exp mark $
 * $Log: util.h,v $
 * Revision 1.2  2008/09/27 13:11:22  mark
 * initial checkin
 *
 * util.h
 * blib
 *
 *  Created by mark on 26/09/2008.
 *  Copyright 2008 Garetech Computer Solutions. All rights reserved.
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <sys/param.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#include "vasprintf.h"

#ifndef __PRETTY_FUNCTION__
#define	__PRETTY_FUNCTION__ __LINE__
#endif

typedef enum  {
    UNKNOWN, YES, NO
} bool_t;

#define	D_NEVER	"Never"
#define D_VMS0	"17-NOV-1858 00:00:00.00"



char    *zapcrlf(char *bp);
char    *newstr(char *fmt,...);
int	*newintstr(char *val);
int	*newint(int val);
time_t	*newdate(char *val);
time_t	scandate(char *datestr);
char	*get_hostname(char *defhostname);
char	*skipwspace(char *str);
char	*fmtctime(time_t ctime);
time_t	now(void);
void    nzfree(void *ptr);
char	*ltrim(char *,int);
char	*rtrim(char *,int);
int	replace_dynstr(char **dynptr, char *newval);
char	invalidchars(char *str, const char *validchars);

#endif /* __UTIL_H__ */
