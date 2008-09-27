#ifndef __PARSESLASHCMD_H__
#define __PARSESLASHCMD_H__
/*
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

#include "util.h"


typedef enum cmdqual_ed cmdqual_e;
enum cmdqual_ed {
    CMD_ADD=0,
    CMD_DISPLAY,
    CMD_MODIFY,
    CMD_REMOVE,
    CMD_REPORT,
    QUAL_MEDIA,
    QUAL_GROUP,
    QUAL_LOCATION,
    QUAL_INCFILENO,
    QUAL_STATE,
    QUAL_RECORD,
    QUAL_EXPIRE,
    QUAL_OFFSITE,
    QUAL_USAGE,
    QUAL_FILENO,
    QUAL_DESC,
    QUAL_ADDFSET,
    CMD_END
};

typedef enum valtype_s valtype_t;
enum valtype_s {
    VAL_NONE=0,
    VAL_STR,
    VAL_INT,
    VAL_DATE
};
typedef enum cmdtype_s cmdtype_t;
enum cmdtype_s {
    DIS=0,
    CMD,
    QUAL
};


typedef struct cmdqual_s cmdqual_t;
struct cmdqual_s {
    cmdqual_e   cmdid;
    cmdtype_t	cmdtype;
    char	*cmdtxt;
    valtype_t	val_type;
};

typedef struct cmd_s cmd_t;
struct cmd_s {
    cmd_t	*prev;
    cmd_t	*next;
    cmdqual_t   *param;
    void	*val;
};


cmd_t	    *parseslashcmd(int argc, char *argv[]);
cmd_t	    *getcmd(char **cmdp);
cmdqual_t   *find_cmdqual(char **cmdp);
cmd_t	    *newcmd(cmdqual_t *cmdqual, char *val);

#endif /* __PARSESLASHCMD_H__ */
