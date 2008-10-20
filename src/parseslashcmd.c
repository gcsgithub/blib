static char *rcsid="@(#) $Id: parseslashcmd.c,v 1.2 2008/09/27 13:11:22 mark Exp mark $";
/*
 * $Log: parseslashcmd.c,v $
 * Revision 1.2  2008/09/27 13:11:22  mark
 * initial checkin
 *
 *
 *  parseslashcmd.c
 *  blib
 *
 *  Created by mark on 26/09/2008.
 *  Copyright 2008 Garetech Computer Solutions. All rights reserved.
 *
 */

#include "parseslashcmd.h"
cmdqual_t CMDQUALS[]  = {
{CMD_ERR,	DIS,	NULL		    ,NULL	     , VAL_STR   , REQVAL_NONE  , NULL},
{CMD_ADD,	CMD,	"/add"		    ,"v_label"	     , VAL_STR   , REQVAL_REQ  , VALIDLABEL},
{CMD_DISPLAY,  	CMD,	"/display"	    ,"v_label"	     , VAL_STR   , REQVAL_REQ  , VALIDLABEL},
{CMD_MODIFY,  	CMD,	"/modify"	    ,"v_label"	     , VAL_STR   , REQVAL_REQ  , VALIDLABEL},
{CMD_REMOVE,   	CMD,	"/remove"	    ,"v_label"	     , VAL_STR   , REQVAL_REQ  , VALIDLABEL}, 
{CMD_REPORT,   	CMD,	"/report"	    ,NULL	     , VAL_NONE  , REQVAL_NONE , NULL},
{CMD_REPORTFRE, CMD,	"/reportfree"	    ,NULL	     , VAL_NONE  , REQVAL_NONE , NULL},
{CMD_REPORTEXP, CMD,	"/reportexpired"    ,NULL	     , VAL_NONE  , REQVAL_NONE , NULL},
{CMD_DOEXPIRE,	CMD,	"/runexpiration"    ,NULL	     , VAL_INT	 , REQVAL_OPT , NULL},
{CMD_EXPORT,	CMD,	"/export"	    ,NULL	     , VAL_STR   , REQVAL_OPT  , NULL}, // export database to {file}
{CMD_IMPORT,	CMD,	"/import"	    ,NULL	     , VAL_STR   , REQVAL_OPT  , NULL}, // import database from {file}
{QUAL_MEDIA,   	QUAL,	"/media"	    ,"v_media"	     , VAL_STR   , REQVAL_REQ  , NULL}, // = TZ89
{QUAL_USAGE,    QUAL,	"/usage"	    ,"v_usage"       , VAL_INT   , REQVAL_NONE , NULL},
{QUAL_FILENO,   QUAL,	"/fileno"	    ,"v_fileno"      , VAL_INT   , REQVAL_REQ  , NULL},
{QUAL_GROUP,    QUAL,	"/group"	    ,"v_group"	     , VAL_STR   , REQVAL_REQ  , NULL},
{QUAL_LOCATION, QUAL,	"/location"	    ,"v_location"    , VAL_STR   , REQVAL_REQ  , NULL},
{QUAL_INCFILENO,QUAL,	"/incfileno"	    ,"v_fileno"	     , VAL_INT   , REQVAL_NONE , NULL},
{QUAL_STATE,	QUAL,	"/state"	    ,"v_state"	     , VAL_STATE , REQVAL_REQ  , NULL},
//{QUAL_LIBRARY,  QUAL,	"/library"	    ,"v_librarydate" , VAL_DATE  , REQVAL_REQ  , NULL},
{QUAL_RECORD,   QUAL,	"/record"	    ,"v_recorddate"  , VAL_DATE  , REQVAL_REQ  , NULL},
{QUAL_OFFSITE,  QUAL,	"/offsite"	    ,"v_offsitedate" , VAL_DATE  , REQVAL_REQ  , NULL},
{QUAL_EXPIRE,   QUAL,	"/expire"	    ,"v_expiredate"  , VAL_DATE  , REQVAL_REQ  , NULL},
{QUAL_BYTESONT, QUAL,   "/bytesontape"	    ,"v_bytesontape" ,VAL_INT64  , REQVAL_REQ  , NULL},
{QUAL_DESC,     QUAL,	"/desc"		    ,"v_desc"        , VAL_STR   , REQVAL_REQ  , NULL},
{QUAL_ADDFSET,  QUAL,	"/addfset"	    ,NULL            , VAL_STR   , REQVAL_REQ  , NULL},
{QUAL_NEW,	QUAL,	"/new"		    ,NULL            , VAL_NONE  , REQVAL_NONE , NULL},  // only valid for /import
{CMD_END,	DIS,	NULL		    ,NULL            , VAL_NONE  , REQVAL_NONE , NULL}
};
#define	    MAX_CMD_WIDTH   16
static char *ver()
{
    return(rcsid);
}

cmd_t *parseslashcmd(int argc, char *argv[])
{
    // for each argv pull out any /parm[=value] and link them into a chain to return to our caller
    cmd_t *hd, *newcmd, *cmdata;	// head of chain cmd
    char    *cmdp;
    int	cmderr;
    int	idx;
    
    hd = (cmd_t *) NULL;
    cmderr=0;
    
    if (argc > 0 ) {
	idx=1;
	while ((idx<argc) && (!cmderr)) {
	    cmdp=argv[idx];
	    while (newcmd = getcmd(&cmdp)) {
		if (newcmd->param->cmdid == CMD_ERR) {
		    fprintf(stderr, "Syntax error: \"%s\"\n", argv[idx]);
		    hd = NULL;
		    cmderr=TRUE;
		    break;
		} else {
		    if (hd == (cmd_t *) NULL) {
			hd = newcmd;
		    } else {
			cmdata = hd;
			while (cmdata->next) cmdata = cmdata->next; // skip to last entry in the chain
			newcmd->prev = cmdata;
			cmdata->next = newcmd;
		    }
		}
	    }
	    idx++;
	}
    }
    return(checksyntax(hd));
}

cmd_t   *getcmd(char **cmdpp)
{
    char    *cmdp, *stp;
    cmd_t   *rval;
    cmdqual_t *cmdqual;
    char    cmd_val[16384];
    char    qchar;
    cmp_e   cmpflg;
    
    bzero(cmd_val,sizeof(cmd_val));
    cmpflg = CMP_NONE;
    
    rval = (cmd_t *) NULL;
    cmdp = *cmdpp;
    cmdp = skipwspace(cmdp);
    if ((cmdp ) && (*cmdp)) {
	if (*cmdp != '/' ) {
	    fprintf(stderr, "Error no command found expect / found \"%c\"/n", *cmdp);
	    cmdqual = &CMDQUALS[CMD_ERR];
	} else {
	    cmdqual = lookup_cmdqual(&cmdp);
	}
	
	if (cmdqual->cmdid != CMD_ERR) { // if we didnt hit end of table
	    cmdp = skipwspace(cmdp);
	    cmpflg = get_cmp(&cmdp);
	    if ( cmpflg > CMP_NONE) {
		if (cmdqual->val_opt == REQVAL_NONE ) {
		    fprintf(stderr, "Syntax error cmd %s does not support a value yet one is provided\n", cmdqual->cmdtxt);
		    return(NULL);
		}
		cmdp = skipwspace(cmdp);	// effectively left trim
		if (cmdp && ( (*cmdp == '\"' ) || (*cmdp == '\''))) {
		    qchar = *cmdp; cmdp++;
		    stp = index(cmdp,qchar);	// index to closing quote
		    memcpy(cmd_val,cmdp,(stp-cmdp));
		    cmdp = stp+1;		// point char after quote
		    cmdp = skipwspace(cmdp);	// skip any whitespace  after the closing quote
		    if (*cmdp) {		// if we not at the end of line
			if (*cmdp != '/' ) {	// then its better be / for start of next /cmd
			    if ((stp = index(cmdp,'/')) != NULL) {// its not / so index to /
				cmdp = stp;	// point to the new cmd
			    }
			    fprintf(stderr, "Trailing trash after closing quote ignored \"%s\"\n", cmdp);//TODO
			}
		    } 
		} else {
		    strcpy(cmd_val,cmdp);		// take the rest of the line
		    cmdp += strlen(cmd_val);	// point to the null
		}
		rtrim(cmd_val,strlen(cmd_val));		// right trim white space
	    }
	}
	rval = newcmd(cmdqual,cmd_val, cmpflg);
	*cmdpp = cmdp;
    }
    return(rval);
}

cmp_e	    get_cmp(char **cmdpp)
{
    cmp_e rval;
    char *cmdp;
    
    const char *cmpc="=><!,.";
    char    *sp,cmp[3];
    int	    idx;
    rval = CMP_NONE;
    
    if (!cmdpp) {
	return(rval);
    }
    cmp[0] = cmp[1] = cmp[2] = '\0';
    idx=0;
    cmdp = *cmdpp;
    
    if (cmdp && *cmdp) {
	if ((sp=index(cmpc,*cmdp)) != NULL) {
	    cmp[idx++] = *sp;
	    cmdp++;
	    if ((sp=index(cmpc,*cmdp)) != NULL) {
		switch(*sp) {
		    case ',':
			cmp[idx++] = '<';
			break;
		    case '.':
			cmp[idx++] = '>';
			break;
		    default:
			cmp[idx++] = *sp;
			break;
		}
		
		cmdp++;
	    }
	}
	*cmdpp = cmdp;
	
	switch(cmp[0]) {
	    case '\0':
		rval = CMP_NONE;
		break;
	    case '=':
		rval = CMP_EQ;			    // =?
		break;
	    case '>':
	    case '.':
		rval = CMP_GT;			    // >?
		break;
	    case '<':
	    case ',':
		rval = CMP_LT;			    // <?
		break;
	    case '!':
		rval = CMP_NE;			    // !?
		break;
	    default:
		rval = CMP_ERR;			    // WTF
		break;
	}
	if (rval > CMP_NONE) {
	    switch(cmp[1]) {
		case '\0':				    // =, <, >, !
		    // we good we have it already from cmp[0]
		    break;
		case '=':
		    switch(rval) {
			case CMP_NONE:
			    rval = CMP_EQ; // not possible but
			    break;
			case CMP_EQ:		    // ==
			    break;
			case CMP_GT:		    // >=
			    rval = CMP_GE;
			    break;
			case CMP_LT:		    // <=
			    rval = CMP_LE;
			    break;
			case CMP_NE:		    // !=
			    break;
			default:
			    rval = CMP_ERR;		    // WTF
			    break;
		    }
		    break;
		case '>':
		case '.':
		    if (rval == CMP_EQ) rval = CMP_GE;  // =>
		    else		rval = CMP_ERR;
		    break;
		case '<':
		case ',':
		    if (rval == CMP_EQ) rval = CMP_LE;  // =<
		    else		rval = CMP_ERR;
		    
		    break;
		case '!':
		    if (rval == CMP_EQ) rval = CMP_NE;  // =!
		    else		rval = CMP_ERR;
		    
		    break;
		default:
		    rval = CMP_ERR;			    // WTF
		    break;
	    }
	}
	if (rval == CMP_ERR) {
	    fprintf(stderr, "Syntax error \"%s\" must be one of =, =<, =>, !=\n", cmp);
	}
    }
    return(rval);
}


cmd_t *newcmd(cmdqual_t *cmdqual, char *val, cmp_e cmpflg)
{
    cmd_t   *rval;
    int	    len_val;
    char    v_state_char, c;
    
    if ((rval = malloc(sizeof(cmd_t))) == (cmd_t *) NULL ) {
	fprintf(stderr, "Error allocating memory in %s %d\n", __PRETTY_FUNCTION__, __LINE__);
	exit(ENOMEM);
    }
    rval->next = NULL;
    rval->prev = NULL;
    rval->param= cmdqual;
    
    if (val)	len_val = strlen(val);
    else	len_val = 0;
    
    if (len_val == 0 ) { // no value given
	if (cmdqual->val_opt == REQVAL_REQ ) { // do we require one ?
	    fprintf(stderr, "Syntax error, command %s requires a value, none was provided\n", cmdqual->cmdtxt);
	    rval->param = &CMDQUALS[CMD_ERR];
	    return(rval);
	}
    } else if (cmdqual->val_opt == REQVAL_NONE)  {
	fprintf(stderr, "Syntax error, no value provided for cmd %s when one is required\n", cmdqual->cmdtxt);
	rval->param = &CMDQUALS[CMD_ERR];
	return(rval);
    }
    if (cmdqual->validchar) { // do we have a character set to validate, yes then do it
	c = invalidchars(val, cmdqual->validchar);
	if ( c ) {
	    fprintf(stderr, "%c is not a valid character \"%s\"\n", c, val);
	    rval->param = &CMDQUALS[CMD_ERR];
	    return(rval);
	}
    }
    switch(cmdqual->val_type) {
	case    VAL_STR:
	    rval->val = newstr(val);
	    break;
	case    VAL_STATE:
	    if (strcasecmp((char *) val, "ALLOCATED") == 0 ) v_state_char = 'A';
	    else if (strcasecmp((char *) val, "FREE") == 0 ) v_state_char = 'F';
	    else {
		fprintf(stderr, "Syntax Error: state must be either \"ALLOCATED\" or \"FREE\" not %s\n", val);
		rval->param = &CMDQUALS[CMD_ERR];
		return(rval);
	    }
	    *(char *) &rval->val = v_state_char;
	    break;
	case    VAL_INT:
	    rval->val = newintstr(val);
	    break;
	case VAL_DATE:
	    rval->val = newdate(val);
	    break;
    }
    rval->cmpflg = cmpflg;
    return(rval);
}

char	*copy_cmd(char *dst, char *src)
{
    char    c, *sp;
    if (!dst || !src) return(NULL);
    sp = dst; // jus so we can see it
    *dst++ = *src++; // copy the first char should be / we already check right!
    do {
	c=*src++;
	switch (c) {
	    case	'/':
	    case	'=':
	    case	'<':
	    case	'.':
	    case	'>':
	    case	',':
	    case	'!':
	    case	' ':
	    case	'\t':
	    case	'\0':
		--src;	// leave them looking at us
		c='\0';
		break;
	    default:
		break;
	}
	*dst++ = c;
    }  while(c);
    
    return(src);
}

cmdqual_t *lookup_cmdqual(char **cmdpp)
{
    int idx;
    char *cmdp;
    char cmdbuf[MAX_CMD_WIDTH];
    
    cmdp = *cmdpp;
    idx=CMD_ERR+1; 
    copy_cmd(cmdbuf, cmdp);
    while (CMDQUALS[idx].cmdid < CMD_END) {
	if (strcasecmp(cmdbuf, CMDQUALS[idx].cmdtxt) == 0 ) {
	    cmdp += (strlen(CMDQUALS[idx].cmdtxt)); // move the point to then char after the matched cmd/qual text
	    *cmdpp = cmdp;
	    return (&CMDQUALS[idx]);
	}
	idx++;
    }
    return (&CMDQUALS[CMD_ERR]);
}

void	    dump_cmd(cmd_t *hd)
{
    cmd_t   *ptr;
    
    ptr=hd;
    while (ptr) {
	display_cmd(ptr);
	ptr = ptr->next;
    }
}

bool_t	    have_qual(cmd_t *hd, cmdqual_e qual)
{
    cmd_t   *ptr;
    bool_t  rval;
    
    rval = NO;
    ptr=hd;
    while (ptr) {
	if (ptr->param->cmdid == qual) {
	    rval = YES;
	    break;
	}
	ptr = ptr->next;
    }
    return(rval);
}

void	    display_cmd(cmd_t *cmd)
{
    char *cp;
    
    
    fprintf(stderr, "param:\n");
    fprintf(stderr, "\tcmdid:%d\n", cmd->param->cmdid);
    switch(cmd->param->cmdtype) {
	case DIS:
	    cp="DIS";
	    break;
	case CMD:
	    cp="CMD";
	    break;
	case QUAL:
	    cp="QUA";
	    break;
    }
    fprintf(stderr, "\tcmdtype:%s txt: %s", cp, cmd->param->cmdtxt);
    switch (cmd->param->val_type) {
	case VAL_NONE:
	    fprintf(stderr, "\n");
	    break;
	case    VAL_STR:
	    fprintf(stderr, " = %s\n", cmd->val);
	    break;
	case    VAL_STATE:
	    switch(*(char *)cmd->val) {
		case    'F':
		    fprintf(stderr, " = %s\n", "FREE");
		    break;
		case    'A':
		    fprintf(stderr, " = %s\n", "ALLOCATED");
		    break;
		default:
		    fprintf(stderr, " = %s\n", "UNKNOWN");
		    break;
	    }    
	    break;	    
	case    VAL_INT:
	    fprintf(stderr, " = %d\n", *( int *) cmd->val);
	    break;
	case    VAL_INT64:
	    fprintf(stderr, " = %llu\n", *( uint64_t *) cmd->val);
	    break;
	case VAL_DATE:
	    fprintf(stderr ," = %s\n", fmtctime(*(time_t *) cmd->val));
	    break;
	    
    }
}

cmd_t	    *checksyntax(cmd_t *hd)
{
    // Current checks: only one command given, /new only with /import, also look for any error cmds
    
    int	cmd_count;
    cmd_t   *rval;
    cmd_t   *ptr;
    cmd_t   *first_cmd, *cmd_new;
    bool_t  first_cmdmsgdone;
    
    cmd_count = 0;
    
    
    if (!hd) return((cmd_t *) NULL);		// just give up now
    
    first_cmdmsgdone = NO;
    cmd_new = first_cmd = (cmd_t *) NULL;
    rval = hd;
    ptr=hd;
    while (ptr) {
	if ((ptr->param->cmdid == CMD_ERR) || (ptr->cmpflg == CMP_ERR)) {
	    rval = (cmd_t *) NULL;
	} 
	if (ptr->param->cmdtype == CMD) {
	    if (first_cmd == (cmd_t *) NULL ) {
		first_cmd = ptr;	// keep a track of first cmd
	    } else {
		if (first_cmdmsgdone == NO ) {
		    first_cmdmsgdone = YES;
		    fprintf(stderr, "Only one command allowed:-\n\t%s\n\t%s\n", first_cmd->param->cmdtxt, ptr->param->cmdtxt);
		    rval = 0;
		} else {
		    fprintf(stderr, "\t%s\n", ptr->param->cmdtxt);
		}
	    }
	    
	}
	if (ptr->param->cmdid == QUAL_NEW ) {
	    cmd_new = ptr;
	}
	ptr = ptr->next;
    }
    if (cmd_new ) {
	if (first_cmd)
	    switch(first_cmd->param->cmdid) {
		case CMD_IMPORT:    // new only valid for import
		    break;
		default:
		    fprintf(stderr, "/new only valid for when command is /import\n");
		    rval = (cmd_t *) NULL;
		    break;
	    }
    }
    return(rval);
    
}
