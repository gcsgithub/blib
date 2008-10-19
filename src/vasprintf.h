#ifndef __VASPRINTF_H__
#define __VASPRINTF_H__

/*
 * $Log:$
 *
 *  vasprintf.h
 *  blib
 *
 *  Created by mark on 15/10/2008.
 *  Copyright 2008 Garetech Computer Solutions. All rights reserved.
 *
 */
#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <stdarg.h>
#include <stdlib.h>
#ifdef NEEDASPRINTF
int	my_asprintf(char **buf,  char *fmt, ... );
int	my_vasprintf(char **buf,  char *fmt, va_list args );
#define	VASPRINTF   my_vasprintf
#define	ASPRINTF    my_asprintf
#else
#define	VASPRINTF   vasprintf
#define	ASPRINTF    asprintf
#endif /* NEEDASPRINTF */
#endif /* __VASPRINTF_H__ */
