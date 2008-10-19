#ifndef __DATA_ACCESS_H__
#define	__DATA_ACCESS_H__
/*
 * @(#) $Id:$
 * $Log:$
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
#include    <getopt.h>
#include    <locale.h>
#include    <sys/stat.h>
typedef struct stat stat_t;

#include    <fcntl.h>
#include    <limits.h>
#include    <sqlite3.h>
#include    "blib.h"
#include    "util.h"


typedef struct {
    mode_t	  saved_umask;
    bool_t	  open;
    int		  status;
    char	  *errmsg;
    char	  *fnm;
    sqlite3	  *dbf;
    sqlite3_stmt  *stmt;
    int		  sqllen;
    char	  *sqltxt;
} DBH;

typedef enum {
    FND_EQUAL, 
    FND_FIRST,
    FND_NEXT
} find_type_t;


typedef struct dbrec_s dbrec_t;
struct dbrec_s {
    char	v_label[13];	// the volume label eg ABC123D
    char	v_state;
    char	v_media[13];
    int		v_usage;
    int		v_fileno;	    // max fileno on tape 0..fileno
    char	v_group[256];
    char	v_location[64];
    time_t	v_librarydate;
    time_t	v_recorddate;
    time_t	v_offsitedate;
    time_t	v_expiredate;
    uint64_t	v_bytesontape;
    char	v_desc[256];
};

typedef struct filt_s filt_t;
struct filt_s {
    cmd_t	*v_label;
    cmd_t	*v_state;
    cmd_t	*v_media;
    cmd_t	*v_usage;
    cmd_t	*v_fileno;
    cmd_t	*v_group;
    cmd_t	*v_location;
    cmd_t	*v_librarydate;
    cmd_t	*v_recorddate;
    cmd_t	*v_offsitedate;
    cmd_t	*v_expiredate;
    cmd_t	*v_bytesontape;
    cmd_t	*v_desc;
};


typedef struct dbevt_s	dbevt_t;
struct dbevt_s {
    char	e_label[13];	// the volume label eg ABC123D
    int		e_fileno;
    time_t	e_recorddate;
    uint64_t	e_bytesinfset;
    char	e_fsetname[256];
};

int	db_newdb(DBH **dbh, char *fnm);
int	db_open(DBH **dbh, char *fnm);
int	db_close(DBH *dbh);
void	db_finish(DBH *dbh);

int	db_insert_dbrec(DBH *dbh, dbrec_t *rec);
int	db_find_dbrec(DBH *dbh,dbrec_t *key, dbrec_t *rec, find_type_t flag);
int	db_update_dbrec(DBH *dbh,filt_t *filtrec, dbrec_t *rec);
int	db_delete_dbrec(DBH *dbh, dbrec_t *key);
dbrec_t *default_dbrec(dbrec_t *rec);

int	db_insert_dbevt(DBH *dbh, dbevt_t *rec);
int	db_find_dbevt(DBH *dbh,dbevt_t *key, dbevt_t *rec, find_type_t flag);
int	db_update_dbevt(DBH *dbh, dbevt_t *rec);
int	db_delete_dbevt(DBH *dbh, dbevt_t *key);

int	copy_volume_results(DBH *dbh, dbrec_t *rec);
int	dbcheck(DBH *dbh, char *errmsg, ...);
int	do_upd(DBH *dbh, char *v_label, cmd_t *cmd );

#endif /* __DATA_ACCESS_H__ */
