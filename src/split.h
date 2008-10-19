#ifndef __SPLIT_H__
#define	__SPLIT_H__
/*
 * @(#) $Id:$
 * $Log:$
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

#define	SPLIT_OK	0	// no errors
#define	SPLIT_BADSEP    1	// " or ' used as sep
#define	SPLIT_ENOMEM	2	// out of memory during malloc
#define	SPLIT_MISSQUOT	3
#define	SPLIT_TRAILJNK	4



#endif /* __SPLIT_H__ */