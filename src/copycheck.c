static const char *rcsid="@(#) $Id: copycheck.c,v 1.2 2011/04/11 03:51:27 mark Exp mark $";
/*
//  copycheck.c
//  blib
//
//  Created by mark on 18/10/2010.
//  Copyright (c) 2010 Garetech Computer Solutions. All rights reserved.
// $Log: copycheck.c,v $
// Revision 1.2  2011/04/11 03:51:27  mark
// add fmt_state
//
// Revision 1.1  2010/11/16 04:05:32  root
// Initial revision
//
*/

#include "copycheck.h"
#include "timefunc.h"

static const char *version()
{
    return(rcsid);
}


char	*copy_label(blabel_t *dst, char *src)
{
    if (src && dst ) {
    	strncpy(dst->str, src,LABEL_SIZ);
    	dst->str[LABEL_SIZ]='\0';
    }
    return(dst->str);
}

int	cmp_labels(blabel_t *l1, blabel_t *l2)
{
    return(strncmp(l1->str, l2->str, sizeof(blabel_t)));
}

int	cmp_objname(objname_t *l1, objname_t *l2)
{
    return(strncmp(l1->str, l2->str, sizeof(blabel_t)));	   
}

char	*copy_desc(desc_t *dst, char *src)
{
    if (src && dst ) {
        strncpy(dst->str, src,DESC_SIZ);
        dst->str[DESC_SIZ]='\0';
    }
    return(dst->str);
}

char	*copy_node(node_t *dst, char *src)
{
    if (src && dst ) {
        strncpy(dst->str, src,NODE_SIZ);
        dst->str[NODE_SIZ]='\0';
    }
    return(dst->str);
}


char	*copy_media(media_t *dst, char *src)
{
    if (src && dst ) {
        strncpy(dst->str, src, MEDIA_NAM_SIZ);
    	dst->str[MEDIA_NAM_SIZ]='\0';
    }
    return(dst->str);
}

char	*copy_state(state_t *dst, char *src)
{
    if (src && dst ) {
        strncpy(dst->str, src, STATE_SIZ);
    	dst->str[STATE_SIZ]='\0';
    }
    return(dst->str);
}

char	*copy_groupname(groupname_t *dst, char *src)
{
    if (src && dst ) {
        strncpy(dst->str, src, GROUPNAM_SIZ);
        dst->str[GROUPNAM_SIZ]='\0';
    }
    return(dst->str);
}

char	*copy_location(location_t *dst, char *src)
{
    if (src && dst ) {
        strncpy(dst->str, src, LOCNAM_SIZ);
        dst->str[LOCNAM_SIZ]='\0';
    }
    return(dst->str);
}

char	*copy_objname(objname_t *dst, char *src)
{ // BUGfix: better protection of src and dst null values, though dst->str better be properly sized, cant tell from here
    if (dst) {
        if (src) {
            strncpy(dst->str, src, OBJ_NAME_SIZ);
            dst->str[OBJ_NAME_SIZ]='\0';
        }
        else {
            bzero(dst->str, OBJ_NAME_SIZ);
        }
        return(dst->str);
    }
    else {
        return(NULL);
    }
}

char	*copy_errmsg(errmsg_t *dst, char *src)
{
    if (src && dst ) {
        strncpy(dst->str, src, ERRMSG_SIZ);
    	dst->str[ERRMSG_SIZ]='\0';
    }
    return(dst->str);
}

vol_t	*default_volume(vol_t *rec)
{
    if (rec) {
        bzero(rec,sizeof(vol_t));
        // set up default values
        copy_state(&rec->state,  "F");
        copy_media(&rec->media, get_default(QUAL_MEDIA));
        rec->usage		= 0;
        copy_groupname(&rec->groupname, get_default(QUAL_GROUP));
        copy_location(&rec->location, get_default(QUAL_LOCATION));
        rec->librarydate	= nowgm();;
        rec->offsitedate	= 0;
    }
    return(rec);
    
}

const char    *fmt_state(char *state)
{
    const char *rval;
    
    switch(*(char *)state) {
        case    'F':
            rval = "FREE";
            break;
        case    'A':
            rval = "ALLOCATED";
            break;
        default:
            rval = "UNKNOWN";
            break;
    }
    return(rval);    
}

