#ifndef __EXECUTE_CMDS_H__
#define	__EXECUTE_CMDS_H__
/*
 * @(#) $Id: execute_cmds.h,v 1.6 2013/01/20 10:08:06 mark Exp mark $
 * $Log: execute_cmds.h,v $
 * Revision 1.6  2013/01/20 10:08:06  mark
 * MG add overrides to display_backup_volumes to allow control of head and foot, and display as FREE eventhough we havent free'd it yet
 *
 * Revision 1.5  2011/04/15 03:40:03  mark
 * add /errcount
 *
 * Revision 1.4  2011/04/11 03:52:57  mark
 * generally fix OSrval's, fix records being added with invalid bck_id, add /verify
 *
 * Revision 1.3  2010/11/24 00:58:15  root
 * add function modify_filter_rec
 *
 * Revision 1.2  2010/11/16 04:10:51  mark
 * rc1
 *
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
int     modify_vol(cmd_t *thecmd, filt_t  *filtrec, vol_t *volrec, cmd_t *qual_ptr);
int     make_filter_rec(cmd_t *qual_ptr, filt_t  *filtrec);
int     filter_rec(filt_t *filtrec, vol_t *rec);

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
void    do_cmd_verifydb(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh);
void    do_cmd_counterrors(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh);

int     usetape(dbh_t *dbh, blabel_t *label, bckid_t bck_id );
int     can_use(dbh_t *dbh, blabel_t *label, bckid_t bck_id);
int     getstate(state_t *state);
void	dump_vol_obj(vol_obj_t *volobjrec);
void	display_backup(fio_t *outfd, bckid_t bckid, cmp_e flag, dbh_t *dbh);
int	    display_backup_volumes(dbh_t *dbh,backups_t *bck_rec, fio_t *outfd, int headfoot, int showasfreed);
void    display_objects(fio_t *outfd, objname_t *objname, dbh_t *dbh);
int     display_backup_volumes_for_object(dbh_t *dbh, bckobj_t *bckobjrec,backups_t *bckrec , fio_t *outfd);

#endif /* __EXECUTE_CMDS_H__ */
