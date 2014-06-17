#ifndef __UTIL_H__
#define __UTIL_H__
/*
 * @(#) $Id: util.h,v 1.4 2010/11/16 04:10:54 mark Exp $
 * $Log: util.h,v $
 * Revision 1.4  2010/11/16 04:10:54  mark
 * rc1
 *
 * Revision 1.3  2008/10/20  13:01:39  mark
 * checkpoint
 *
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

#include <stdarg.h>
#include <stdint.h>
#include "vasprintf.h"

#ifndef __PRETTY_FUNCTION__
#define	__PRETTY_FUNCTION__ __FILE__
#endif
#define cpp_str(x)  #x
#define __ATLINE__ __PRETTY_FUNCTION__ ":"  cpp_str(__LINE__)

#ifdef __alpha
#define	 atoll atol
#endif /* __alpha */

typedef enum  {
    UNKNOWN, YES, NO
} bool_t;




// convient typedefs for printf formats llu and lu, important for 32/64 bit coding
// printf knows how to format long and long long and these types will convert as appropriote to match
// reguardless of the input data size or size of long
typedef unsigned long long llu_t;
typedef unsigned long	lu_t;

char    *zapcrlf(char *bp);
char    *newstr(char *fmt,...);
int	*newintstr(char *val);
int	*newint(int val);

uint64_t *newuint64(uint64_t val);
uint64_t *newuint64str(char *val);
char	*get_hostname(char *defhostname);
char	*skipwspace(char *str);
void    nzfree(char **p2ptr);
char	*ltrim(char *,int);
char	*rtrim(char *,int);
int	replace_dynstr(char **dynptr, char *newval);
char	invalidchars(char *str, const char *validchars);
char	*cvt2uppercase(char *str);
char	*pstr(char *str, char *def);
void	dodbg(void);
int	safe_inc_int(int *iptr);
char	*shrink_string_by_middle(char *dst, int dstlen, char *src);
char	*strdupz(char *str);

#endif /* __UTIL_H__ */
