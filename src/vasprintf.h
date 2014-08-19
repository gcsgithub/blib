#ifndef __VASPRINTF_H__
#define __VASPRINTF_H__

/*
 * @(#) $Id: vasprintf.h,v 1.2 2010/11/16 04:11:18 mark Exp mark $
 *
 * $Log: vasprintf.h,v $
 * Revision 1.2  2010/11/16 04:11:18  mark
 * rc1
 *
 * Revision 2.5  2010/04/07 08:34:52  mark
 * fix error with hangling args list
 *
 * Revision 2.4  2010/01/09 09:32:19  mark
 * RC4
 *
 * Revision 2.3  2009/12/31 05:54:13  mark
 * RC3
 *
 * Revision 1.10  2009/12/29 02:12:09  mark
 * RC2
 *
 * Revision 1.9  2009/12/28 16:20:45  mark
 * threadsRC1
 *
 * Revision 1.8  2009/01/20 02:11:54  mark
 * b4thread
 *
 * Revision 1.7  2009/01/07 23:09:52  mark
 * RC4
 *
 * Revision 1.6  2009/01/06 21:29:07  mark
 * RC3
 *
 * Revision 1.5  2009/01/06 01:19:55  mark
 * RC2
 *
 * Revision 1.4  2009/01/05 23:25:59  mark
 * MallocDBG
 *
 * Revision 1.3  2009/01/04 17:48:49  mark
 * RC1
 *
 * Revision 1.2  2009/01/01 23:51:20  mark
 * fix ident
 *
 * Revision 1.1  2008/12/15 06:56:36  mark
 * Initial revision
 *
 * Revision 1.1  2008/10/19  22:19:01  root
 * Initial revision
 *
 *
 *  vasprintf.h
 *  blib
 *
 *  Created by mark on 15/10/2008.
 *  Copyright 2008 Garetech Computer Solutions. All rights reserved.
 *
 */

#include "util.h"

#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <stdarg.h>
#include <stdlib.h>
#ifdef NEED_ASPRINTF
int	my_asprintf(char **buf,  char *fmt, ... );
int	my_vasprintf(char **buf,  char *fmt, va_list args );
#define	VASPRINTF   my_vasprintf
#define	ASPRINTF    my_asprintf
#else
#define	VASPRINTF   vasprintf
#define	ASPRINTF    asprintf
#endif /* NEED_ASPRINTF */
#endif /* __VASPRINTF_H__ */
