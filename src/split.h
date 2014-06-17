#ifndef __SPLIT_H__
#define	__SPLIT_H__
/*
 * @(#) $Id: split.h,v 1.2 2010/11/16 04:11:12 mark Exp $
 * $Log: split.h,v $
 * Revision 1.2  2010/11/16 04:11:12  mark
 * rc1
 *
 * Revision 1.1  2008/10/19  22:18:59  root
 * Initial revision
 *
 *
 *  split.h
 *  blib
 *
 *  Created by mark on 09/10/2008.
 *  Copyright 2008 Garetech Computer Solutions. All rights reserved.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "list.h"
#include "util.h"
#define NL  (char *) NULL
#ifndef TRUE
#define	TRUE	-1
#endif
#ifndef FALSE
#define	FALSE	0
#endif

int	split(char *buf,char *sep, list_t **flds);
char 	*newfld(char *fldptr);

#define	SPLIT_OK	0	// no errors
#define	SPLIT_BADSEP    1	// " or ' used as sep
#define	SPLIT_ENOMEM	2	// out of memory during malloc, just exit no point trying
#define	SPLIT_MISSQUOT	3
#define	SPLIT_TRAILJNK	4



#endif /* __SPLIT_H__ */