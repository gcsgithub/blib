#ifndef __MKARGV_H__
#define __MKARGV_H__
/*
 * @(#) $Id:$
 *
 *  mkargv.h
 *  blib
 *
 *  Created by mark on 26/10/2010.
 *  Copyright 2010 Garetech Computer Solutions. All rights reserved.
 *
 * $Log:$
 *
 */
#include "split.h"

int mkargv(char ***argv, char *line);
ssize_t mksinglespaced(char *line, char sep);

#endif /* __MKARGV_H__ */