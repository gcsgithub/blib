/*
 *  parseslashcmd.c
 *  blib
 *
 *  Created by mark on 26/09/2008.
 *  Copyright 2008 Garetech Computer Solutions. All rights reserved.
 *
 */

#include "parseslashcmd.h"
static cmdqual_t CMDQUALS[]  = {
{CMD_ADD,	CMD,	"/add",	     VAL_STR },
{CMD_DISPLAY,  	CMD,	"/display",  VAL_STR },
{CMD_MODIFY,  	CMD,	"/modify",   VAL_STR },
{CMD_REMOVE,   	CMD,	"/remove",   VAL_STR }, 
{CMD_REPORT,   	CMD,	"/report",   VAL_NONE},
{QUAL_MEDIA,   	QUAL,	"/media",    VAL_STR }, // = TZ89
{QUAL_GROUP,    QUAL,	"/group",    VAL_STR },
{QUAL_LOCATION, QUAL,	"/location", VAL_STR },
{QUAL_INCFILENO,QUAL,	"/incfileno",VAL_NONE},
{QUAL_STATE,	QUAL,	"/state",    VAL_STR },
{QUAL_RECORD,   QUAL,	"/record",   VAL_DATE},
{QUAL_EXPIRE,   QUAL,	"/expire",   VAL_DATE},
{QUAL_OFFSITE,  QUAL,	"/offsite",  VAL_DATE},
{QUAL_USAGE,    QUAL,	"/usage",    VAL_NONE},
{QUAL_FILENO,   QUAL,	"/fileno",   VAL_INT},
{QUAL_DESC,     QUAL,	"/desc=",    VAL_STR},
{QUAL_ADDFSET,  QUAL,	"/addfset",  VAL_STR},
{CMD_END,	DIS,	NULL,	     VAL_NONE}
};

cmd_t *parseslashcmd(int argc, char *argv[])
{
    // for each argv pull out any /parm[=value] and link them into a chain to return to our caller
    cmd_t *hd, *newcmd, *cmdptr;	// head of chain cmd
    char    *cmdp, cmdline[8192];
    int	    cmdlen;
    int	idx;
    hd = (cmd_t *) NULL;
    if (argc > 0 ) {
	strncpy(cmdline,argv[0], sizeof(cmdline));
	cmdlen=strlen(cmdline);
	for (idx=1;idx<argc;idx++) {
	    strncat(cmdline,argv[idx], sizeof(cmdline)-cmdlen);
	    cmdlen=strlen(cmdline);
	}
	cmdp = cmdline;
	
	while (newcmd = getcmd(&cmdp)) {  
	    if (hd == (cmd_t *) NULL) hd = newcmd;
	    else {
		cmdptr = hd->next;
		while (cmdptr->next) cmdptr = cmdptr->next; // skip to last entry in the chain
		newcmd->prev = cmdptr;
		cmdptr->next = newcmd;
	    }
	}
    }
    return(hd);
    
}



cmd_t   *getcmd(char **cmdpp)
{
    char    *cmdp, *stp;
    cmd_t   *rval;
    cmdqual_t *cmdqual;
    char    buf[16384];
    
    
    cmdp = *cmdpp;
    skipwspace(cmdp);
    if (cmdp ) {
	if (*cmdp != '/' ) {
	    fprintf(stderr, "Error no command found expect / found \"%c\"/n", *cmdp);
	    free(rval);
	    return(NULL);
	}
    }
    cmdqual = find_cmdqual(&cmdp);
    bzero(buf,sizeof(buf));
    if (cmdqual->cmdid != CMD_END) {
	skipwspace(cmdp);
	if ((cmdqual->val_type != VAL_NONE ) && (*cmdp != '=')) {
	    fprintf(stderr, "Syntax error missing = after cmd %s\n", cmdqual->cmdtxt);
	    return(NULL);
	} else if (*cmdp == '=') {
	    cmdp++; // skip over =
	    if (cmdqual->val_type == VAL_NONE ) {
		fprintf(stderr, "Syntax error cmd %s does not support a value yet one is provided\n", cmdqual->cmdtxt);
		return(NULL);
	    }
	    skipwspace(cmdp);
	    stp = strstr(cmdp,"/");		// index to start of next param
	    if (stp != (char *) NULL ) {	// found a next one
		memcpy(buf,cmdp,(stp-cmdp));
		cmdp=stp;			// leave them with the new command
	    } else {
		strcpy(buf,cmdp);		// no new command so take the rest of the line
		cmdp += strlen(buf);	// point to the null
	    }
	}
    }
    rval = newcmd(cmdqual,buf);
    *cmdpp = cmdp;
    return(rval);
}

cmd_t *newcmd(cmdqual_t *cmdqual, char *val)
{
    cmd_t   *rval;
    
    if ((rval = malloc(sizeof(cmd_t))) == (cmd_t *) NULL ) {
	fprintf(stderr, "Error allocating memory in %s %d\n", __PRETTY_FUNCTION__, __LINE__);
	exit(ENOMEM);
    }
    rval->next = NULL;
    rval->prev = NULL;
    rval->param= cmdqual;
    switch(cmdqual->val_type) {
	case    VAL_STR:
	    rval->val = newstr(val);
	    break;
	case    VAL_INT:
	    rval->val = newint(val);
	    break;
	case VAL_DATE:
	    rval->val = newdate(val);
	    break;
    };
    return(rval);
}



cmdqual_t *find_cmdqual(char **cmdpp)
{
    int idx;
    char *cmdp;
    cmdp = *cmdpp;
    while (CMDQUALS[idx].cmdid != CMD_END) {
	if (strncasecmp(cmdp, CMDQUALS[idx].cmdtxt,strlen(CMDQUALS[idx].cmdtxt)) == 0 ) {
	    cmdp += (strlen(CMDQUALS[idx].cmdtxt)+1); // move the point to then char after the matched cmd/qual text
	    *cmdpp = cmdp;
	    return (&CMDQUALS[idx]);
	}
	idx++;
    }
    return (&CMDQUALS[idx]);
}
