#ifndef __LIST_H__
#define __LIST_H__
/*
 * @(#) $Id:$
 * $Log:$
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
#include <time.h>
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

list_t *list_insert(list_t **lis, entry_t *ent);
int	list_delete(list_t *lis, entry_t *ent,int (*free_entry)(entry_t *ent));
int	free_list(list_t **lis,int (*free_entry)(entry_t *ent));
void	dump_list(list_t *lis);
entry_t *new_entry(void *val);
entry_t *snprintf_ent(char *val, size_t len,char *fmt, entry_t *ent, char *errmsg, ...);
entry_t *atoi_ent(     int *val, entry_t *ent, char *errmsg, ...);
entry_t	*time_ent(  time_t *val, entry_t *ent, char *errmsg, ...);

#endif /* __LIST_H__ */
