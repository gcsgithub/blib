static char *rcsid="@(#) $Id:$";
/*
 * $Log:$
 *
 *  list.c
 *  blib
 *
 *  Created by mark on 10/05/1998
 *  Copyright 1998-2008 Garetech Computer Solutions. All rights reserved.
 *
 */

#include "list.h"
static char *ver()
{
    return(rcsid);
}


list_t *list_insert(list_t **lis, entry_t *ent)
{
    entry_t *oldtail;
    list_t  *list;
    if (lis == (list_t **) NULL ) {
	fprintf(stderr, "Attempt to call %d:%s with null point to list\n", __LINE__,__PRETTY_FUNCTION__ );
	return((list_t *) NULL);
    }
    if (ent == (entry_t *) NULL ) {
	fprintf(stderr, "Attempt to insert null list entry in %d:%s\n",__LINE__,__PRETTY_FUNCTION__);
	return(*lis);
    }
    list = *lis; // get the address of there pointer to list
    if (list == (list_t *) NULL ) { // no current list
	if ((list = (list_t *) malloc(sizeof(list_t))) == (list_t *) NULL ) {
	    fprintf(stderr, "Error allocating list in  %d:%s\n",__LINE__,__PRETTY_FUNCTION__);
	    return(list);
	} else {
	    bzero(list,sizeof(list_t));
	    *lis = list; // and give them back the new list we created
	}
    }
    
    if (list->head == (entry_t *) NULL) {      /* new list */
	list->head = list->tail = ent;
	ent->np = ent->pp = (entry_t *)NULL;
    } else {
	
	oldtail = list->tail;
	
	list->tail = ent;
	
	ent->pp = oldtail;
	
	ent->np = (entry_t *) NULL;
	
	oldtail->np = ent;
    }
    list->items++;
    return(list);
    
}


int	list_delete(list_t *lis, entry_t *ent, int (*free_entry)(entry_t *ent))
{
    entry_t *oneb4, *oneafter;
    int	rval;
    
    if (lis && ent) {
	oneb4 = ent->pp;
	oneafter = ent->np;
        
	if ( (   oneb4 == (entry_t *) NULL) &&
	    (oneafter == (entry_t *) NULL)) {
	    lis->head = lis->tail = (entry_t *) NULL;
	} else {
	    
	    if (oneb4 == (entry_t *) NULL) {
		lis->head = oneafter;
		oneafter->pp = (entry_t *)NULL;
	    } else {
		oneb4->np = oneafter;
	    }
	    
	    if (oneafter == (entry_t *) NULL) {
		lis->tail = oneb4;
		oneb4->np = (entry_t *)NULL;
	    } else {
		oneafter->pp = oneb4;
	    }
	}
	lis->items--;
	rval = (free_entry)(ent);
    } else {
	rval = -1;
    }
    return(rval);
}


int	free_list(list_t **lis,int (*free_entry)(entry_t *ent))
{
    list_t  *list;
    entry_t *ent, *pp;
    
    if (lis == (list_t **) NULL ) {
	fprintf(stderr, "Attempt to call %d:%s with null point to list\n", __LINE__,__PRETTY_FUNCTION__ );
	return(-1);
    }
    
    list = *lis; // get the address of there pointer to list
    if (list == (list_t *) NULL ) { // no current list
	return(0);  // all done
    }
    
    if (list->head != (entry_t *) NULL) { 
	pp = list->tail;
	while (pp) {
	    ent=pp;
	    pp=ent->pp;
	    (free_entry)(ent);
	}
	free(list);
	*lis = (list_t *) NULL;
    } 
    return(0);
}

void	dump_list(list_t *lis)
{
    entry_t *ent;
    ent = lis->head;
    while (ent) {
	fprintf(stderr, "Entry: \"%s\"\n", ent->e);
	ent=ent->np;
    }
}

entry_t *new_entry(void *val)
{
    entry_t *ent;
    
    if ((ent = (entry_t *) malloc(sizeof(entry_t))) == (entry_t *) NULL ) {
	fprintf(stderr, "Error allocating entry in  %d:%s\n",__LINE__,__PRETTY_FUNCTION__);
    } else {
	ent->e = val;
	ent->np = ent->pp = (entry_t *) NULL;
    }
    return(ent);	
}



entry_t *snprintf_ent(char *val, size_t len,char *fmt, entry_t *ent, char *errmsg,...)
{
    va_list args;
    va_start(args,errmsg);
    
    if (ent) {
	if (ent->e) {
	    snprintf(val, len,fmt,ent->e);
	    ent = ent->np;
	} else {
	    bzero(val, len);
	    vfprintf(stderr, errmsg, args);
	    va_end(args);
	}
    } else {
	vfprintf(stderr, errmsg, args);
	va_end(args);
    }
    return(ent);
}


entry_t *atoi_ent(int *val, entry_t *ent, char *errmsg,...)
{
    va_list args;
    int    rval;
    va_start(args,errmsg);
    
    
    if (ent) {
	if (ent->e) rval = atol(ent->e);
	ent = ent->np;
    } else {
	rval = 0;
	vfprintf(stderr, errmsg, args);
	va_end(args);
    }
    if (val) {
	*val = rval;
    } else {
	fprintf(stderr, "%s called with null pointer for return value\n", __PRETTY_FUNCTION__);
    }
    return(ent);
}

entry_t	*time_ent(time_t *val, entry_t *ent, char *errmsg,...)
{
    va_list args;
    time_t    rval;
    va_start(args,errmsg);
    
    
    if (ent) {
	if (ent->e) rval = scandate(ent->e);
	ent = ent->np;
    } else {
	rval = 0;
	vfprintf(stderr, errmsg, args);
	va_end(args);
    }
    if (val) {
	*val = rval;
    } else {
	fprintf(stderr, "%s called with null pointer for return value\n", __PRETTY_FUNCTION__);
    }
    return(ent);
}


