#ifndef __DATA_ACCESS_H__
#define	__DATA_ACCESS_H__
/*
 * @(#) $Id: data_access.h,v 1.3 2011/04/11 03:51:55 mark Exp mark $
 * $Log: data_access.h,v $
 * Revision 1.3  2011/04/11 03:51:55  mark
 * generally fix OSrval's, fix records being added with invalid bck_id, add /verify
 *
 * Revision 1.2  2010/11/16 04:11:14  mark
 * rc1
 *
 * Revision 1.1  2008/10/19  22:18:58  root
 * Initial revision
 *
 *  data_access.h
 *  blib
 *
 *  Created by mark on 08/10/2008.
 *  Copyright 2008 Garetech Computer Solutions. All rights reserved.
 *
 */

#include    <arpa/inet.h>
#include    <ctype.h>
#include    <errno.h>
#include    <math.h>
#include    <netdb.h>
#include    <netinet/in.h>
#include    <regex.h>
#include    <setjmp.h>
#include    <signal.h>
#include    <stdarg.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <sys/fcntl.h>
#include    <sys/param.h>
#include    <sys/socket.h>
#include    <sys/types.h>
#include    <sys/uio.h>
#include    <sys/wait.h>
#include    <syslog.h>
#include    <time.h>
#include    <unistd.h>
#ifndef __hpux
#include    <getopt.h>
#endif
#include    <locale.h>
#include    <sys/stat.h>
typedef struct stat stat_t;

#include    <fcntl.h>
#include    <limits.h>

#define	    BLIBDB_OK		SQLITE_OK
#define     BLIBDB_ROW		SQLITE_ROW
#define	    BLIBDB_DONE		SQLITE_DONE
#define     BLIBDB_CONSTRAINT	SQLITE_CONSTRAINT

#include    "blib.h"
#include    "util.h"
#include    "list.h"
#include    "parseslashcmd.h"
#include    "data_structures.h"


int     db_busy_handler(void *tag,int lockoccurence);
int     db_newdb(dbh_t **dbh, char *fnm);
int     db_open(dbh_t **dbh, char *fnm);
int     db_close(dbh_t *dbh);
void    db_finish(dbh_t *dbh);
int     dbcheck(dbh_t *dbh, char *errmsg, ...);

sqlcmd_t *sqlstack_push(dbh_t *dbh);
sqlcmd_t *sqlstack_pop(dbh_t *dbh);

int     db_prepare_sql(dbh_t *dbh, sqlcmd_t *sqlcmd, char *sqltextfld, ...);
int 	db_finalize_stmt(dbh_t *dbh);

objid_t db_get_obj_instance(dbh_t *dbh, bckid_t bck_id, objname_t *objname );

int     db_exec_sql(dbh_t *dbh, char *sqltext);
int     db_exec_sql_flds(dbh_t *dbh, char *sqltext, list_t *flds);
int	db_exec_sql_flds_pushpop(dbh_t *dbh, char *sqltext, list_t *flds);
int     db_exec_sql_flds_pop(dbh_t *dbh, char *sqltext, list_t *flds);

int     db_exec_sql_bckid(dbh_t *dbh, char *sqltext, bckid_t bck_id);

dbfld_t *db_fldsmklist(list_t **fldhead,const char *fldname,  fld_type_t fldtype, void *fldptr);
void	 db_fldsfreelist(list_t **listp);

int	 db_fldsdump(list_t *flds);
void	 db_flds_display(dbfld_t *fld);
dbfld_t *db_flds_byname(list_t *flds, const char *fldname);
char	*db_flds_textbyname(list_t *flds, const char *fldname);
uint64_t db_flds_int64byname(list_t *flds, const char *fldname);

dbfld_t *db_flds_new(void);
int 	 db_flds_bind(dbh_t *dbh, sqlcmd_t *sqlcmd);
int 	 db_columns(dbh_t *dbh);

const char *fldtype_name(fld_type_t fldtype);
fld_type_t db_fldtype_from_valtype(valtype_e valtype);

int     db_find(dbh_t *dbh, char *sqltext, list_t *key, void *results, int (*copyresult)(dbh_t *dbh, void *rsp), find_type_t flag);
int     db_find_vol_free(dbh_t *dbh, vol_t *rec, find_type_t flag);
int     db_find_vol_expired(dbh_t *dbh, vol_t *rec, find_type_t flag);
int     db_find_volume_free(dbh_t *dbh, vol_t *rec, find_type_t flag);
int     db_find_volume_expired(dbh_t *dbh, vol_t *rec, find_type_t flag);
int     db_find_volumes_label(dbh_t *dbh,vol_t *key, vol_t *rec, find_type_t flag);
int     db_find_volume_bylabel(dbh_t *dbh,blabel_t *label, vol_t *rec, find_type_t flag);
int     db_find_vol_obj_id_notbckid_label(dbh_t *dbh, list_t *key_bck_id, vol_obj_t *volobj2del,find_type_t flag);
int     db_find_current_volobj(dbh_t *dbh, vol_obj_t *volobjrec);
time_t	db_lookup_endofbackup(dbh_t *dbh,bckid_t bck_id);
int	db_find_backups_orderbckid(dbh_t *dbh, backups_t *bckrec, bckid_t bckid, find_type_t flag);
int	db_find_backups_by_expire(dbh_t *dbh, backups_t *bckrec, find_type_t flag);
int     db_find_bck_objects_by_bckid(dbh_t *dbh, bckid_t bck_id,bckobj_t *bckobjrec, find_type_t flag);
int     db_find_vol_obj_from_objects(dbh_t *dbh, bckobj_t *key, vol_obj_t *volobjrec, find_type_t flag);
int	db_find_bck_errors(dbh_t *dbh, vol_obj_t *volobjkey, bck_errors_t *bckerrrec, find_type_t flag);
int     db_find_volumes_by_bckid(dbh_t *dbh, bckid_t bck_id, vol_t *volrec, find_type_t flag);
int     db_find_backups_by_bck_id(dbh_t *dbh, bckid_t key_bckid, backups_t *bck_rec);
int	db_find_bck_objects_by_name(dbh_t *dbh, objname_t *objname, bckobj_t *bckobjrec, find_type_t flag);
int	db_find_vol_obj_for_bck_object(dbh_t *dbh, bckobj_t *bckobjrec, vol_obj_t *volobjrec, find_type_t flag);
int 	db_find_volume_for_vol_obj(dbh_t *dbh, vol_obj_t *volobjrec, vol_t *volrec, find_type_t flag);

int     do_upd_vol(dbh_t *dbh, char *label, cmd_t *cmd );
int     db_setvolume_free(dbh_t *dbh, char *label);
int     db_setvolume_used(dbh_t *dbh, blabel_t *label, bckid_t bckid);
void	db_update_volume(dbh_t *dbh,filt_t *filtrec, vol_t *rec);
int     db_end_vol_obj(dbh_t *dbh, vol_obj_t *volobjrec); // update end and size
bcount_t db_vol_obj_sumsize(dbh_t *dbh, vol_obj_t *volobjrec);
int     db_update_bck_object_size_end(dbh_t *dbh, bckid_t bckid, objname_t *objname, objid_t obj_instance,  time_t end, bcount_t totalsize);
int     db_inc_volume_usage(dbh_t *dbh, bckid_t bck_id);
int     db_update_backups_end(dbh_t *dbh, bckid_t bck_id, time_t end);
int     db_update_backups(dbh_t *dbh,backups_t *bckrec);


int     db_delete_volume(dbh_t *dbh, vol_t *vol2del);
int     db_delete_backup_id(dbh_t *dbh, bckid_t bck_id);



int     db_insert_volumes(dbh_t *dbh, vol_t *rec);
int     db_insert_backups(dbh_t *dbh, backups_t *bck_rec);
objid_t	db_insert_bck_objects(dbh_t *dbh, bckobj_t *bckobjrec);
int     db_insert_vol_obj(dbh_t *dbh, vol_obj_t *volobjrec);
int     db_insert_bckerror(dbh_t *dbh, bck_errors_t *bckerror);

int 	db_count_vol_obj_label(dbh_t *dbh, bckid_t bck_id, blabel_t *label);

bcount_t db_count_volumes_in_backup_id(dbh_t *dbh, bckid_t bck_id);
bcount_t db_count_bck_errors(dbh_t *dbh, vol_obj_t *key);

#ifdef db_clear_old_back_for_label_tmp
int     db_clear_old_back_for_label(dbh_t *dbh, vol_obj_t *volobjrec);
#endif /* db_clear_old_back_for_label */

int     copy_results_volume(dbh_t      *dbh, void *recp);
int     copy_results_backup(dbh_t      *dbh, void *recp);
int     copy_results_bck_objects(dbh_t *dbh, void *recp);
int     copy_results_vol_obj(dbh_t     *dbh, void *recp);
int     copy_results_bck_errors(dbh_t  *dbh, void *recp);


bcount_t db_get_size(dbh_t *dbh, vol_t *volrec);
int      db_get_duration(dbh_t *dbh, vol_t *volrec);


bcount_t  db_count_sqltext(dbh_t *dbh, char *sqltext);
int	db_read_bck_errors_fault1(dbh_t *dbh, bck_errors_t *bckerrrec, find_type_t flag);
int	db_read_bck_errors_fault2(dbh_t *dbh, bck_errors_t *bckerrrec, find_type_t flag);
void db_display_bck_errors(FILE *fd, bck_errors_t *bckerrrec);



int     db_verify(fio_t *outfd, dbh_t *dbh);




#endif /* __DATA_ACCESS_H__ */
