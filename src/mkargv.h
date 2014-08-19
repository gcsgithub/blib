#ifndef __MKARGV_H__
#define __MKARGV_H__
/*
 * @(#) $Id: mkargv.h,v 1.1 2010/11/16 04:03:46 root Exp mark $
 *
 *  mkargv.h
 *  blib
 *
 *  Created by mark on 26/10/2010.
 *  Copyright 2010 Garetech Computer Solutions. All rights reserved.
 *
 * $Log: mkargv.h,v $
 * Revision 1.1  2010/11/16 04:03:46  root
 * Initial revision
 *
 *
 */
#include "split.h"

int mkargv(char ***argv, char *line);
ssize_t mksinglespaced(char *line, char sep);

#endif /* __MKARGV_H__ */
