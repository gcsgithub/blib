#ifndef __LIST_H__
#define __LIST_H__
/*
 * @(#) $Id: list.h,v 1.3 2013/01/20 10:15:26 mark Exp $
 * $Log: list.h,v $
 * Revision 1.3  2013/01/20 10:15:26  mark
 * MG remove time_ent()
 *
 * Revision 1.2  2010/11/16 04:11:16  mark
 * rc1
 *
 * Revision 1.1  2008/10/19  22:18:59  root
 * Initial revision
 *
 *
 *  list.h
 *  blib
 *
 *  Created by mark on 09/10/2008.
 *  Copyright 2008 Garetech Computer Solutions. All rights reserved.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <stdarg.h>
#include "timefunc.h"
#include "util.h"

typedef struct entry entry_t;
typedef struct list  list_t;

struct  entry {
    void	*e;
    entry_t	*pp;
    entry_t	*np;
};


struct  list {
    int     items;
    entry_t *head;
    entry_t *tail;
};

list_t *list_insert_tail(list_t **lis, entry_t *ent);
int	list_delete(list_t *lis, entry_t *ent,int (*free_entry)(entry_t *ent));
int	free_list(list_t **lis,int (*free_entry)(entry_t *ent));
void	dump_list(list_t *lis);
entry_t *new_entry(void *val);
entry_t *snprintf_ent(int *err, char *val, size_t len,char *fmt, entry_t *ent, char *errmsg, ...);
entry_t *atoi_ent(int *err,unsigned int *val, entry_t *ent, char *errmsg, ...);


#endif /* __LIST_H__ */
