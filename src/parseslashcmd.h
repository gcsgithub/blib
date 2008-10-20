#ifndef __PARSESLASHCMD_H__
#define __PARSESLASHCMD_H__
/*
 * @(#) $Id: parseslashcmd.h,v 1.2 2008/09/27 13:11:22 mark Exp mark $
 *  $Log: parseslashcmd.h,v $
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
#include "split.h"


typedef enum  {
    CMD_ERR=0,
    CMD_ADD,
    CMD_DISPLAY,
    CMD_MODIFY,
    CMD_REMOVE,
    CMD_REPORT,
    CMD_REPORTFRE,
    CMD_REPORTEXP,
    CMD_DOEXPIRE,
    CMD_EXPORT,
    CMD_IMPORT,
    QUAL_MEDIA,
    QUAL_USAGE,
    QUAL_FILENO,
    QUAL_GROUP,
    QUAL_LOCATION,
    QUAL_INCFILENO,
    QUAL_STATE,
    // QUAL_LIBRARY,
    QUAL_RECORD,
    QUAL_OFFSITE,
    QUAL_EXPIRE,
    QUAL_BYTESONT,
    QUAL_DESC,
    QUAL_ADDFSET,
    QUAL_NEW,
    CMD_END
} cmdqual_e;

typedef enum  {
    VAL_NONE=0,
    VAL_STR,
    VAL_STATE,	// single char coded as A = ALLOCATED, F = FREE
    VAL_INT,
    VAL_INT64, 
    VAL_DATE
} valtype_t;

typedef enum  {
    REQVAL_NONE=0,
    REQVAL_OPT,
    REQVAL_REQ
} reqval_t;

typedef enum  {
    DIS=0,
    CMD,
    QUAL
} cmdtype_t;


typedef struct cmdqual_s cmdqual_t;
struct cmdqual_s {
    cmdqual_e   cmdid;
    cmdtype_t	cmdtype;
    char	*cmdtxt;
    char	*sql_fldnam;
    valtype_t	val_type;
    reqval_t	val_opt;
    char	*validchar;
};

#define	VALID3OF9   "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-.$/+%* "
#define	VALIDLABEL  "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-"	// reduced 3of9
#define	VALIDNO	    "0123456789"
#define	VALIDSTR    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz/\\!@#$%^&*()_+-=[]\{}|;':\",./<>?"

typedef enum { // <=
   CMP_ERR=-1, CMP_NONE=0, CMP_LT, CMP_LE, CMP_EQ, CMP_GE, CMP_GT, CMP_NE 
} cmp_e;

typedef struct cmd_s cmd_t;
struct cmd_s {
    cmd_t	*prev;
    cmd_t	*next;
    cmdqual_t   *param;
    cmp_e	cmpflg;
    void	*val;
};

cmd_t	    *parseslashcmd(int argc, char *argv[]);
cmd_t	    *getcmd(char **cmdp);
cmdqual_t   *lookup_cmdqual(char **cmdp);
cmd_t	    *newcmd(cmdqual_t *cmdqual, char *val, cmp_e cmpflg);
void	    dump_cmd(cmd_t *hd);
void	    display_cmd(cmd_t *cmd);
bool_t	    have_qual(cmd_t *hd, cmdqual_e qual);
cmd_t	    *checksyntax(cmd_t *hd);
cmp_e	    get_cmp(char **cmdp);

#endif /* __PARSESLASHCMD_H__ */
