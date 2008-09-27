#ifndef __UTIL_H__
#define __UTIL_H__
/*
 *  util.h
 *  blib
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



char    *zapcrlf(char *bp);
char    *newstr(char *str);
int	*newint(char *val);
time_t	*newdate(char *val);
char	*get_hostname(char *defhostname);
char	*skipwspace(char *str);

#endif /* __UTIL_H__ */