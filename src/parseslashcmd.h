#ifndef __PARSESLASHCMD_H__
#define __PARSESLASHCMD_H__
/*
 * @(#) $Id: parseslashcmd.h,v 1.5 2011/04/11 03:54:29 mark Exp mark $
 *  $Log: parseslashcmd.h,v $
 *  Revision 1.5  2011/04/11 03:54:29  mark
 *  add include log stuff
 *  add verify styff
 *
 *  Revision 1.4  2010/11/16 04:10:47  mark
 *  rc1
 *
 * Revision 1.3  2008/10/20  13:01:37  mark
 * checkpoint
 *
 *  Revision 1.2  2008/09/27 13:11:22  mark
 *  initial checkin
 *
 *
 *  parseslashcmd.h
 *  blib
 *
 *  Created by mark on 26/09/2008.
 *  Copyright 2008 Garetech Computer Solutions. All rights reserved.
 *
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <inttypes.h>

#include "util.h"
#include "fileio.h"

#define NL  (char *) NULL
#ifndef TRUE
#define	TRUE	-1
#endif
#ifndef FALSE
#define	FALSE	0
#endif

#if defined (__alpha) || defined (__ALPHA)
#define	INT64_MAX   18446744073709551615U
#endif

#define	VALID3OF9   "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-.$/+%* "
#define	VALIDLABEL  "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-"	// reduced 3of9
#define	VALIDNO	    "0123456789"
#define	VALIDSTR    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz/\\!@#$%^&*()_+-=[]\{}|;':\",./<>?"
#define DEF_STYSHT  "/usr/local/etc/dat/blib/fmtbckrep.stylesheet"


typedef enum  {
    CMD_ERR=0,
    CMD_HELP=128,
    CMD_ENV,
    CMD_ADD,
    CMD_DISPLAY,
    CMD_MODIFY,
    CMD_REMOVE,
    
    CMD_REPORTFRE,
    CMD_REPORTEXP,
    CMD_DOEXPIRE,
    
    CMD_REPORT,
    CMD_REPLAY,
    
    CMD_NEWBCK,
    CMD_STARTBCK, 
    CMD_CHG_VOL,
    CMD_ENDBCK,
    CMD_ERRBCK, 
    CMD_FINBCK,
    CMD_DELBCK, 
    CMD_MODBCK, 
    CMD_REPBCK, 
    CMD_LISTBCK,
    CMD_LISTOBJ, 
    CMD_VERIFY, 
    
    QUAL_NEW,
    QUAL_NOLOG,
    QUAL_LOG,
    QUAL_SINCE, 
    QUAL_UNTIL, 
    QUAL_STATE,
    QUAL_MEDIA,
    QUAL_USAGE,
    QUAL_GROUP,
    QUAL_LOCATION,
    QUAL_DATABASE,
    QUAL_LIBDATE,
    QUAL_RECORD,
    QUAL_OFFSITE,
    QUAL_EXPIRE,
    QUAL_SIZE,
    QUAL_DESC,
    QUAL_NODE,
    QUAL_OBJINS,
    QUAL_BCKID,
    QUAL_LABEL, 
    QUAL_ENDBCK,	// pecullar to /modifybackup and /change_volume
    QUAL_HTML,		// report in html instead of plain text - mostly for reports especially daily
    QUAL_XML,		// TODO: xml
    QUAL_STYSHT,
    QUAL_MAIL,
    QUAL_OUTPUT,	// open this file for output instead of stdout
    QUAL_INCLOG,
    
    CMD_END
} cmdqual_e;

typedef enum  {
    VT_NONE=0,
    VT_STR,
    VT_LABEL,
    VT_FILENAM,
    VT_STATE,	// single char coded as A = ALLOCATED, F = FREE
    VT_INT,
    VT_INT64, 
    VT_DATE
} valtype_e;

typedef enum  {
    REQVAL_NONE=0,
    REQVAL_OPT,
    REQVAL_REQ
} reqval_e;

typedef enum  {
    DIS=0,
    CMD,
    QUAL
} cmdtype_e;

typedef enum {
    CMP_ERR=-1,
    CMP_NONE=0,
    CMP_LT,
    CMP_LE,
    CMP_EQ,
    CMP_GE,
    CMP_GT,
    CMP_NE,
    CMP_OPT
} cmp_e;

typedef enum {
    VAL_UNDEF=0, VAL_NULL, VAL_SET, VAL_DEF
} val_e;

typedef enum {
    FMT_TEXT, FMT_XHTML
} fmt_type_e;

typedef enum {
    DB_NONE=0,
    DB_RO,
    DB_WO,
    DB_RW
} dbaccess_e;

typedef struct cmdqual_s cmdqual_t;
struct cmdqual_s {
    cmdqual_e   cmdid;
    cmdtype_e	cmdtype;
    dbaccess_e	dbaccess;
    char	*cmdtxt;
    char	*sql_fldnam;
    valtype_e	val_type;
    reqval_e	val_opt;
    char	*validchar;
    char	*defval;
    char	*helptxt;
};

typedef struct cmd_s cmd_t;
struct cmd_s {
    cmd_t	*prev;
    cmd_t	*next;
    cmdqual_t   *param;
    cmp_e	cmpflg;
    val_e	valset;
    void	*val;
};

typedef struct filt_s filt_t;
struct filt_s {
    cmd_t	*label;
    cmd_t	*state;
    cmd_t	*media;
    cmd_t	*usage;
    cmd_t	*groupname;
    cmd_t	*location;
    cmd_t	*librarydate;
    cmd_t	*offsitedate;
};


cmd_t	    *parseslashcmd(char *cmdline);
cmd_t	    *getcmd(char **cmdp);
cmd_t	    *insert_cmd_tail(cmd_t **head, cmd_t *cmdent);
cmdqual_t   *lookup_cmdqual(char **cmdp);
cmdqual_t   *lookup_cmdqual_id(cmdqual_e cmdid);
cmd_t	    *newcmd(cmdqual_t *cmdqual, char *val, cmp_e cmpflg);
void	    dump_cmd(cmd_t *hd);
void	    display_cmd(FILE *fd, cmd_t *cmd);
cmd_t	    *have_qual_unlink(cmd_t **hd, cmdqual_e qual);
cmd_t	    *have_qual(cmd_t **hd, cmdqual_e qual);
int	    addqual(cmd_t **qualptr, cmdqual_e qtype, void *val);
cmd_t	    *checksyntax(cmd_t *hd);
cmp_e	    get_cmp(char **cmdp);
void	    do_cmd_help(FILE *fd);

void	    setup_defaults();
void	    log_cmd(fio_t *logfd, cmd_t *hd);
char	    *lookup_state(char *state_char);
char	    *get_state(char *statestr);
cmdqual_t   *set_default(cmdqual_e qual, char *defval);
char	    *get_default(cmdqual_e qual);
char	    *getlognamefromdbname(char *dbname);


char	    *mkcmdline(int argc, char *argv[]);
#endif /* __PARSESLASHCMD_H__ */
