#ifndef __EXECUTE_CMDS_H__
#define	__EXECUTE_CMDS_H__
/*
 * @(#) $Id: execute_cmds.h,v 1.1 2008/10/19 22:18:59 root Exp mark $
 * $Log: execute_cmds.h,v $
 * Revision 1.1  2008/10/19  22:18:59  root
 * Initial revision
 *
 *
 *  execute_cmds.h
 *  blib
 *
 *  Created by mark on 28/09/2008.
 *  Copyright 2008 Garetech Computer Solutions. All rights reserved.
 *
 */
#include <stdarg.h>
#include "parseslashcmd.h"
#include "data_access.h"
#include "copycheck.h"
#include "mkargv.h"


dbh_t	*execute_cmds(dbh_t *dbh, cmd_t **cmds);
dbh_t	*process_command_line(dbh_t *dbh, char *cmdline);
int	modify_vol(cmd_t *thecmd, filt_t  *filtrec, vol_t *volrec, cmd_t *qual_ptr);
int	filter_rec(filt_t *filtrec, vol_t *rec);

void	do_cmd_env(fio_t *outfd);
void	do_cmd_add_volume(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr, dbh_t *dbh);
void 	do_cmd_modify_volume(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr, dbh_t *dbh);
void	do_cmd_report(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh);
void	do_cmd_report_free(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh);
void	do_cmd_report_exp(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh);
void	do_cmd_runexpiration(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh);
void 	do_cmd_replaylog(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh);

void	do_cmd_newbackup(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh);
void	do_cmd_startbackup(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh);
void	do_cmd_change_volume(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh);
void	do_cmd_endbackup(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh);
void	do_cmd_errbackup(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh);
void	do_cmd_finishbackup(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh);

void	do_cmd_removebackup(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh);
void	do_cmd_modifybackup(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh);
void	do_cmd_listbackups(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh);
void	do_cmd_listobjects(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh);
int	usetape(dbh_t *dbh, blabel_t *label, bckid_t bck_id );
int	can_use(dbh_t *dbh, blabel_t *label, bckid_t bck_id);
int	getstate(state_t *state);
void	dump_vol_obj(vol_obj_t *volobjrec);
void	display_backup(fio_t *outfd, bckid_t bckid, dbh_t *dbh);
int	display_backup_volumes(dbh_t *dbh,backups_t *bck_rec, fio_t *outfd);
void    display_objects(fio_t *outfd, objname_t *objname, dbh_t *dbh);
int     display_backup_volumes_for_object(dbh_t *dbh, bckobj_t *bckobjrec,backups_t *bckrec , fio_t *outfd);

#endif /* __EXECUTE_CMDS_H__ */
