static const char *rcsid="@(#) $Id: list.c,v 1.2 2010/11/16 04:10:20 mark Exp mark $";
/*
 * $Log: list.c,v $
 * Revision 1.2  2010/11/16 04:10:20  mark
 * rc1
 *
 * Revision 1.1  2008/10/19  22:18:59  root
 * Initial revision
 *
 *
 *  list.c
 *  blib
 *
 *  Created by mark on 10/05/1998
 *  Copyright 1998-2008 Garetech Computer Solutions. All rights reserved.
 *
 */

#include "list.h"
#include "timefunc.h"

static const char *version()
{
    return(rcsid);
}


list_t *list_insert_tail(list_t **lis, entry_t *ent)
{
    entry_t *oldtail;
    list_t  *list;
    if (lis == (list_t **) NULL ) {
        fprintf(stderr, "Attempt to call " __ATLINE__ " with null point to list\n");
        return((list_t *) NULL);
    }
    if (ent == (entry_t *) NULL ) {
        fprintf(stderr, "Attempt to insert null list entry in " __ATLINE__ "\n");
        return(*lis);
    }
    list = *lis; // get the address of there pointer to list
    if (list == (list_t *) NULL ) { // no current list
        if ((list = (list_t *) malloc(sizeof(list_t))) == (list_t *) NULL ) {
            fprintf(stderr, "#BLIB:  Error allocating list in  " __ATLINE__"\n");
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
            if (free_entry) (free_entry)(ent); // run the users free program
	    else 		free(ent);	// or just free then entry assumes the user has grabed the ->e pointer
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
        fprintf(stderr, "Entry: \"%s\"\n", (char *) ent->e);
        ent=ent->np;
    }
}

entry_t *new_entry(void *val)
{
    entry_t *ent;
    
    if ((ent = (entry_t *) malloc(sizeof(entry_t))) == (entry_t *) NULL ) {
        fprintf(stderr, "#BLIB:  Error allocating memory for entry in " __ATLINE__ "\n");
    } else {
        ent->e = val;
        ent->np = ent->pp = (entry_t *) NULL;
    }
    return(ent);	
}



entry_t *snprintf_ent(int *err, char *val, size_t len,char *fmt, entry_t *ent, char *errmsg,...)
{
    va_list args;
    va_start(args,errmsg);
    
    if (ent) {
        if (ent->e) {
            if (val) {
                snprintf(val, len,fmt,ent->e);
                ent = ent->np;
            } else {
                fprintf(stderr, "%s called with null pointer for return value\n", __PRETTY_FUNCTION__);
                (*err)++;
            }
        } else {
            bzero(val, len);
            vfprintf(stderr, errmsg, args);
            va_end(args);
            (*err)++;
        }
    } else {
        vfprintf(stderr, errmsg, args);
        va_end(args);
        (*err)++;
    }
    return(ent);
}


entry_t *atoi_ent(int *err,unsigned int *val, entry_t *ent, char *errmsg,...)
{
    va_list args;
    int    rval;
    va_start(args,errmsg);
    
    
    if (ent) {
        if (ent->e) rval = atol(ent->e);
        else        rval = 0;
        ent = ent->np;
    } else {
        rval = 0;
        vfprintf(stderr, errmsg, args);
        va_end(args);
        (*err)++;
    }
    if (val) {
        *val = rval;
    } else {
        fprintf(stderr, "%s called with null pointer for return value\n", __PRETTY_FUNCTION__);
        (*err)++;
    }
    return(ent);
}



