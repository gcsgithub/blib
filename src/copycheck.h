#ifndef __COPYCHECK_H__
#define __COPYCHECK_H__
/*
// @(#) $Id: copycheck.h,v 1.2 2011/04/11 03:51:38 mark Exp $
//
//  copycheck.h
//  blib
//
//  Created by mark on 18/10/2010.
//  Copyright (c) 2010 Garetech Computer Solutions. All rights reserved.
// $Log: copycheck.h,v $
// Revision 1.2  2011/04/11 03:51:38  mark
// add fmt_state
//
// Revision 1.1  2010/11/16 04:04:35  root
// Initial revision
//
*/

#include "data_structures.h"
#include "parseslashcmd.h"
#include <strings.h>
#include <string.h>

char	*copy_label(blabel_t *dst, char *src);
int     cmp_labels(blabel_t *l1, blabel_t *l2);
char	*copy_desc(desc_t *dst, char *src);
char	*copy_node(node_t *dst, char *src);
char	*copy_state(state_t *dst, char *src);
char	*copy_media(media_t *dst, char *src);
char	*copy_groupname(groupname_t *dst, char *src);
char	*copy_location(location_t *dst,char *src);
char	*copy_objname(objname_t *dst, char *src);
int     cmp_objname(objname_t *l1, objname_t *l2);
char	*copy_errmsg(errmsg_t *dst, char  *src);
vol_t	*default_volume(vol_t *rec);
const char    *fmt_state(char *state);

#endif /* __COPYCHECK__H_ */

