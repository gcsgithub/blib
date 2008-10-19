#ifndef __EXECUTE_CMDS_H__
#define	__EXECUTE_CMDS_H__
/*
 * @(#) $Id:$
 * $Log:$
 *
 *  execute_cmds.h
 *  blib
 *
 *  Created by mark on 28/09/2008.
 *  Copyright 2008 Garetech Computer Solutions. All rights reserved.
 *
 */
#include <stdarg.h>
#include "blib.h"
#include "data_access.h"
#include "parseslashcmd.h"


int	execute_cmds(cmd_t *cmds);
int	modify_rec(filt_t *filtrec, dbrec_t *volrec, cmd_t *cmdp);
int	display_volume(int flag, dbrec_t *volrec);
char	*doenv(int flag, char *symbol, valtype_t val_type,void *val);
int	filter_rec(filt_t *filtrec, dbrec_t *rec);

#endif /* __EXECUTE_CMDS_H__ */
