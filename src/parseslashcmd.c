static char *rcsid="@(#) $Id: parseslashcmd.c,v 1.12 2014/06/17 05:08:54 mark Exp mark $";
/*
 * $Log: parseslashcmd.c,v $
 * Revision 1.12  2014/06/17 05:08:54  mark
 * formating
 *
 * Revision 1.11  2013/01/21 16:54:54  mark
 * MG fix old int cast from nowgm(), update help text for new decimal date
 *
 * Revision 1.10  2013/01/20 10:29:44  mark
 * MG changes for new decimal time switch from time_t to blib_tim_t
 * fix memory leak
 *
 * Revision 1.9  2011/04/15 03:40:09  mark
 * add /errcount
 *
 * Revision 1.8  2011/04/14 02:33:33  mark
 * optional compare ?= ?= for including logs only on error
 *
 * Revision 1.7  2011/04/12 00:36:57  mark
 * make internal error more descriptive to what values caused it
 *
 * Revision 1.6  2011/04/11 03:54:00  mark
 * generally fix OSrval's, fix records being added with invalid bck_id, add /verify
 *
 * Revision 1.5  2010/11/22 05:52:59  mark
 * print original cmdline not cmdp when reporting Syntax error
 * update help text for /report
 *
 * Revision 1.4  2010/11/16 04:09:52  mark
 * rc1
 *
 * Revision 1.3  2008/10/20  13:01:37  mark
 * checkpoint
 *
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
#include "blib.h"
#include "timefunc.h"

#define	    MAX_CMD_WIDTH   16

cmdqual_t CMDQUALS[]  = {
    // cmdid        ,cmdtype,dbaccess         ,cmdtxt      ,sql_fldnam ,val_type  ,val_opt     ,validchar ,default   ,helptxt
    {CMD_ERR        ,DIS  ,DB_NONE   ,NULL                 ,NULL	        ,VT_STR	   ,REQVAL_NONE ,NULL      ,NULL      ,NULL},
    {CMD_HELP       ,CMD  ,DB_NONE   ,"/help"             ,NULL           ,VT_NONE   ,REQVAL_NONE ,NULL      ,NULL      ,"This help text"},
    {CMD_ENV        ,CMD  ,DB_RO     ,"/env"              ,NULL           ,VT_NONE   ,REQVAL_NONE ,NULL      ,NULL      ,"Display environment in effect"},
    {CMD_ADD        ,CMD  ,DB_WO     ,"/add"              ,"label"       ,VT_LABEL  ,REQVAL_REQ  ,VALIDLABEL,NULL      ,"Add a new volume name to database"},
    {CMD_DISPLAY    ,CMD  ,DB_RO     ,"/display"	      ,"label"       ,VT_LABEL  ,REQVAL_REQ  ,VALIDLABEL,NULL      ,"Display existing volume and any fsets"},
    {CMD_MODIFY     ,CMD  ,DB_RW     ,"/modify"	          ,"label"       ,VT_LABEL  ,REQVAL_REQ  ,VALIDLABEL,NULL      ,"Modify an existing volume"},
    {CMD_REMOVE     ,CMD  ,DB_RW     ,"/remove"           ,"label"       ,VT_LABEL  ,REQVAL_REQ  ,VALIDLABEL,NULL      ,"Remove and existing volume and any fsets from database"} ,
    {CMD_REPORTFRE  ,CMD  ,DB_RO     ,"/reportfree"       ,NULL	       ,VT_INT	   ,REQVAL_OPT  ,NULL      ,NULL      ,"Report free volumes for compatibility"},
    {CMD_REPORTEXP  ,CMD  ,DB_RO     ,"/reportexpired"    ,NULL	       ,VT_INT	   ,REQVAL_OPT  ,NULL      ,NULL      ,"Report expired volumes for compatibility"},
    {CMD_DOEXPIRE   ,CMD  ,DB_RW     ,"/runexpiration"    ,NULL          ,VT_INT	   ,REQVAL_OPT  ,NULL      ,NULL      ,"free up all or up too n expired tapes"},
    {CMD_REPORT     ,CMD  ,DB_RO     ,"/report"	          ,NULL	       ,VT_NONE   ,REQVAL_NONE ,NULL      ,NULL      ,"Report all or filtered list of volumes in database"},
    {CMD_REPLAY     ,CMD  ,DB_RW     ,"/replaylog"	      ,NULL	       ,VT_FILENAM,REQVAL_REQ  ,NULL      ,NULL      ,"Replay log file [/new to begin a new database]"},
    {CMD_NEWBCK     ,CMD  ,DB_WO     ,"/newbackup"	      ,NULL	       ,VT_NONE   ,REQVAL_NONE ,NULL      ,NULL      ,"Establish a new backup and return its bck_id"},
    {CMD_STARTBCK   ,CMD  ,DB_WO     ,"/startbackup"      ,"objname"   ,VT_STR    ,REQVAL_REQ  ,NULL      ,NULL      ,"start backup of object on a given /label="},
    {CMD_CHG_VOL    ,CMD  ,DB_RW     ,"/change_volume"    ,"objname"   ,VT_STR    ,REQVAL_REQ  ,NULL      ,NULL      ,"close current volume of object start new on /label="},
    {CMD_ENDBCK     ,CMD  ,DB_WO     ,"/endbackup"	      ,"objname",VT_STR    ,REQVAL_REQ  ,NULL      ,NULL      ,"finish backup of object"},
    {CMD_ERRBCK     ,CMD  ,DB_WO     ,"/errbackup"	    ,"objname",   VT_STR    ,REQVAL_REQ  ,NULL      ,NULL      ,"report an error during backup against /errbackup= /bck_id= /label="},
    {CMD_FINBCK     ,CMD  ,DB_RW     ,"/finishbackup"   ,NULL	      ,VT_STR    ,REQVAL_NONE ,NULL      ,NULL      ,"finish a backup id (verify and summary updates)"},
    {CMD_DELBCK     ,CMD  ,DB_RW     ,"/removebackup"   ,"bck_id"	 ,VT_INT64  ,REQVAL_REQ  ,NULL      ,NULL      ,"Remove all data refering to backup id given"},
    {CMD_MODBCK     ,CMD  ,DB_NONE   ,"/modifybackup"   ,"bck_id"    ,VT_INT64  ,REQVAL_REQ  ,NULL      ,NULL      ,"Modify backup info for given bck_id"},
    {CMD_REPBCK     ,CMD  ,DB_RO     ,"/reportbackup"   ,"bck_id"	 ,VT_INT64  ,REQVAL_OPT  ,NULL      ,NULL      ,"report backup info for given bck_id"},
    {CMD_LISTBCK    ,CMD  ,DB_RO     ,"/listbackups"    ,"bck_id"	 ,VT_INT64  ,REQVAL_OPT  ,NULL      ,NULL      ,"list all backups or given backupid or for a given /label="},
    {CMD_LISTOBJ    ,CMD  ,DB_RO     ,"/listobjects"    ,"objname"   ,VT_STR    ,REQVAL_OPT  ,NULL      ,NULL      ,"list all backups for all backups or a given object name"},
    {CMD_VERIFY     ,CMD  ,DB_RO     ,"/verify"         ,NULL        ,VT_NONE   ,REQVAL_NONE ,NULL      ,NULL      , "Verify internal conistancy of database tables"},
    {CMD_ERRCOUNT   ,CMD  ,DB_RO     ,"/errcount"       ,"bck_id"    ,VT_INT64  ,REQVAL_REQ  ,NULL      ,NULL      , "Return BLIB_ERRCOUNT for a given backup id"},
    {QUAL_NEW       ,QUAL ,DB_RW     ,"/new"	            ,NULL          ,VT_NONE	   , REQVAL_NONE , NULL      ,NULL      , "only valid for /import"},
    {QUAL_SINCE     ,QUAL ,DB_NONE   ,"/since"	       ,NULL          ,VT_DATE	   , REQVAL_REQ  , NULL      ,NULL      , "with /replaylog /since=ctime:n[.n] to replay log since n"},
    {QUAL_UNTIL     ,QUAL ,DB_NONE   ,"/until"	      ,NULL           ,VT_DATE	   , REQVAL_REQ  , NULL      ,NULL      , "with /replaylog /until=ctime:n[.n] to replay log since n"},
    {QUAL_LOG       ,QUAL   ,DB_NONE  ,"/log"	      ,NULL	      ,VT_FILENAM  , REQVAL_OPT  , NULL      ,NULL      , "log command to optional log file def: BLIB_LOG"},
    {QUAL_NOLOG     ,QUAL   ,DB_NONE  ,"/nolog"           ,NULL	      ,VT_NONE     , REQVAL_NONE , NULL      ,NULL      , "Do NOT log this command to BLIB_LOG"},
    {QUAL_STATE     ,QUAL   ,DB_NONE  ,"/state"	      ,"state"	      ,VT_STATE    , REQVAL_REQ  , NULL      ,"FREE"    , "Volume state Free or Allocated" },
    {QUAL_MEDIA     ,QUAL   ,DB_NONE  ,"/media"	      ,"media"	      ,VT_STR	   , REQVAL_REQ  , NULL      ,NULL      , "media type def: %s..." },
    {QUAL_USAGE     ,QUAL   ,DB_NONE  ,"/usage"	      ,"usage"        ,VT_INT	   , REQVAL_OPT  , NULL      ,"1"      , "Increment/Set usage counter"},
    {QUAL_GROUP     ,QUAL   ,DB_NONE  ,"/group"	      ,"groupname"    ,VT_STR	   , REQVAL_REQ  , NULL      ,NULL      , "The Backup group this volume belongs too MYBLIB_GROUP"},
    {QUAL_LOCATION  ,QUAL   ,DB_NONE  ,"/location"	      ,"location"     ,VT_STR	   , REQVAL_REQ  , NULL      ,NULL      , "where is it"},
    {QUAL_DATABASE  ,QUAL   ,DB_NONE  ,"/database"	      ,NULL           ,VT_FILENAM  , REQVAL_REQ  , NULL      ,NULL      , "name of the database BLIBDBS"},
    {QUAL_RECORD    ,QUAL   ,DB_NONE  ,"/record"	      ,"recorddate"   ,VT_DATE	   , REQVAL_REQ  , NULL      ,NULL      , "start time"},
    {QUAL_ENDBCK    ,QUAL   ,DB_NONE  ,"/end"	      ,"end"	      ,VT_DATE     , REQVAL_REQ  , NULL      ,NULL      , "end time"},
    {QUAL_LIBDATE   ,QUAL   ,DB_NONE  ,"/libdate"	      ,"librarydate"  ,VT_DATE	   , REQVAL_REQ  , NULL      ,NULL      , "date volume was added to library normally calculated"},
    {QUAL_OFFSITE   ,QUAL   ,DB_NONE  ,"/offsite"	      ,"offsitedate"  ,VT_DATE	   , REQVAL_REQ  , NULL      ,NULL      , "suggested Date the tape should be offsite by"},
    {QUAL_EXPIRE    ,QUAL   ,DB_NONE  ,"/expire"	      ,"expiredate"   ,VT_DATE	   , REQVAL_REQ  , NULL      ,NULL      , "Date the backup id will expire and return volumes to /state=FREE"},
    {QUAL_SIZE      ,QUAL   ,DB_NONE  ,"/size"	      ,"size"         ,VT_INT64    , REQVAL_REQ  , NULL      ,NULL      , "Bytes to record"},
    {QUAL_DESC      ,QUAL   ,DB_NONE  ,"/desc"	      ,"desc"         ,VT_STR      , REQVAL_REQ  , NULL      ,NULL      , "Description of the backup or error"},
    {QUAL_NODE      ,QUAL   ,DB_NONE  ,"/node"	      ,"node"         ,VT_STR	   , REQVAL_REQ  , NULL      ,NULL      , "name of node the backup was recorded on"},
    {QUAL_OBJINS    ,QUAL   ,DB_NONE  ,"/objinstance"     ,"obj_instance" ,VT_INT      , REQVAL_REQ  , NULL      ,NULL      , "BLIB_OBJINSTANCE returned by /startbackup usually 1"},
    {QUAL_BCKID     ,QUAL   ,DB_NONE  ,"/bck_id"	      ,"bck_id"       ,VT_INT64    , REQVAL_REQ  , NULL      ,NULL      , "bck_id that we intend working on"},
    {QUAL_LABEL     ,QUAL   ,DB_NONE  ,"/label"	      ,"label"	      ,VT_LABEL    , REQVAL_REQ  , NULL      ,NULL      , "Volume label to be used"},
    {QUAL_HTML      ,QUAL   ,DB_NONE  ,"/html"	      ,NULL	      ,VT_NONE     , REQVAL_NONE , NULL      ,NULL      , "output in html rather than plain text"},
    {QUAL_STYSHT    ,QUAL   ,DB_NONE  ,"/stylesheet"      ,NULL	      ,VT_FILENAM  , REQVAL_REQ  , NULL      ,NULL, "stylesheet to include for /html output"},
    {QUAL_MAIL      ,QUAL   ,DB_NONE  ,"/mail"	      ,NULL	      ,VT_STR      , REQVAL_REQ  , NULL      ,NULL      , "email backup report to address"},
    {QUAL_OUTPUT    ,QUAL   ,DB_NONE  ,"/output"	      ,NULL	      ,VT_FILENAM  , REQVAL_REQ  , NULL      ,NULL      , "output result to file"},
    {QUAL_INCLOG    ,QUAL   ,DB_NONE  ,"/includelog"      ,NULL	      ,VT_FILENAM  , REQVAL_REQ  , NULL      ,NULL      , "include log file in output"},
    
    {CMD_END        ,DIS    ,DB_NONE  ,NULL		      ,NULL           ,VT_NONE	   , REQVAL_NONE , NULL      ,NULL      , "End of the line"}
};

static char *ver()
{
    return(rcsid);
}

cmdqual_t *set_default(cmdqual_e qual, char *defval)
{
    cmdqual_t *cmd_entry;
    cmd_entry = lookup_cmdqual_id(qual);
    if (cmd_entry) {
        replace_dynstr(&cmd_entry->defval,  newstr(defval));
    } else {
        fprintf(stderr, "#BLIB:  Internal error could not find qualifier %d\n", qual);
        exit(EINVAL);
    }
    return(cmd_entry);
}

char	*get_default(cmdqual_e qual)
{
    cmdqual_t	*cmd_entry;
    char	    *defval;
    
    cmd_entry = lookup_cmdqual_id(qual);
    defval    = cmd_entry->defval;
    if (!defval) {
        defval="UNKNOWN";
    }
    return(defval);
}

void	setup_defaults()
{
    char    *sp;
    char    *this_host;
    char    buf[MAXPATHLEN+1];
    cmdqual_t *blib_group;
    
    
    this_host = get_hostname(NL);
    set_default(QUAL_NODE, this_host);
    
    //============================================================================
    if ((sp=getenv("MYBLIB_GROUP")) == NL ) {
        snprintf(buf, sizeof(buf),"%s", this_host);
        sp = buf;
    }
    blib_group = set_default(QUAL_GROUP, sp);
    
    //============================================================================
    if ((sp=getenv("BLIBDBS")) == NL ) {
        snprintf(buf, sizeof(buf),"/usr/local/etc/dat/blib/blib_%s.sqlite3", blib_group->defval);
        sp = buf;
    }
    set_default(QUAL_DATABASE, sp);
    
    if ((sp=getenv("BLIB_LOG")) == NL ) {
        sp = getlognamefromdbname(get_default(QUAL_DATABASE));
        strncpy(buf,sp,  sizeof(buf));
        free(sp);
        sp = buf;
    }
    set_default(QUAL_LOG, sp);
    
    
    //============================================================================
    if ((sp=getenv("DAILYMEDIA")) == NL ) {
        sp = "TZ89";
    }
    set_default(QUAL_MEDIA, sp);
    //============================================================================
    
    if ((sp=getenv("BLIB_NODE")) == NL ) {
        sp = this_host;
    }
    set_default(QUAL_NODE, sp);
    
    //============================================================================
    
    if ((sp=getenv("BLIB_STYLE")) == NL ) {
        sp = DEF_STYSHT;
    }
    set_default(QUAL_STYSHT, sp);
    
}

char *getlognamefromdbname(char *dbname)
{
    size_t len, logfnmlen;
    char *logfnm;
    char *decimalptr;
    
    if ((decimalptr=rindex(dbname, '.'))) {
        len=decimalptr-dbname;
    } else {
        len=strlen(dbname); // no . in name
    }
    
    logfnmlen=len+5;
    if ((logfnm=malloc(logfnmlen))) {
        memcpy(logfnm, dbname, len);
        logfnm[len]= '\0';
        strcat(logfnm, ".log");
    } else {
        fprintf(stderr, "Error allocating memory in " __PRETTY_FUNCTION__ "\n");
        exit(ENOMEM);
    }
    return(logfnm);
}

cmd_t *parseslashcmd(char *cmdline)
{
    // for each argv pull out any /parm[=value] and link them into a chain to return to our caller
    cmd_t *head;
    cmd_t *newcmd;	// head of chain cmd
    char  *cmdp;
    
    head = (cmd_t *) NULL;
    
    cmdp=cmdline;
    while ((newcmd = getcmd(&cmdp))) {  // work through each argv entry until exausted
        if (newcmd->param->cmdid == CMD_ERR) {
            fprintf(stderr, "#BLIB:  Syntax error no command found: \"%s\"\n", cmdline);
            free(newcmd);
            head = (cmd_t *) NULL;
            break;
        } else {
            insert_cmd_tail(&head, newcmd);
        }
    }
    
    
    return(checksyntax(head));
}

cmd_t	*insert_cmd_tail(cmd_t **head, cmd_t *cmdent)
{
    cmd_t *cmdptr;
    if (head) {
        if (*head == (cmd_t *) NULL) {
            *head = cmdent;
        } else {
            cmdptr = *head;
            while (cmdptr->next) cmdptr = cmdptr->next; // skip to last entry in the chain
            cmdent->prev = cmdptr;
            cmdptr->next = cmdent;
        }
    } else {
        fprintf(stderr, "#BLIB:  Internal error creating command list\n");
        exit(EINVAL);
    }
    return(cmdent);
}

cmd_t   *getcmd(char **cmdpp)
{// get the next command from the line cmdpp and return a new cmd_t * that matches the new command
    char      *cmdp;
    char      *fldstart;
    char      *fldend;
    cmd_t     *rval;
    cmdqual_t *cmdqual;
    char      cmd_val[16384];
    char      qchar;	// 1st level quote char
    char      qchar2;	// 2nd level quote char
    cmp_e     cmpflg;
    ssize_t   len;
    
    //
    // It is so simple, hammer that rod, bash it into place, weld it over the bowplane, with out it we cant survive, make him understand Luigi!
    //
    // The command line is full of element divided by slash thus an unquoted / is the start of a command
    //  so each command can be seen as from here (cmdp) to the next unquoted slash
    //  this function is called multiple times to grab each sucessive command from the buffer.
    //
    // if the 1st thing we see is a quote, then we will take it that the whole command is inside that quote
    //  eg.   '/desc=ABC 123 axy'
    //
    bzero(cmd_val,sizeof(cmd_val));
    cmpflg = CMP_NONE;
    
    qchar = qchar2 ='\0'; // no quote
    rval = (cmd_t *) NULL;
    if (cmdpp ) {
        cmdp = *cmdpp;
        if (cmdp) {
            fldstart = fldend = skipwspace(cmdp);
            
            if ((*fldstart == '\"') || (*fldstart == '\'')) { // this will be the shell quoting the command inside a eval eg.
                                                              //    '/desc="fred"'
                                                              // or '/desc=New Backup test 1'
                if (qchar == '\0') { // start of new quote
                    qchar = *fldstart++; // capture the quote state and skip forward
                    fldend = index(fldstart, qchar);
                    if (fldend == (char *) NULL ) {
                        fprintf(stderr, "#BLIB: missing closing quote %c\n", qchar);
                        cmdqual = &CMDQUALS[CMD_ERR];
                        goto getcmd_return_val;
                    }
                    // *fldend = '\0';
                    cmdp = fldend+1;	// point to char past quote 						//////////////////**************///////////////////
                    fldend--;		// point last char of field
                    
                } else {
                    fprintf(stderr, "#BLIB: inconcievable, quote char not null\n");
                    exit(EINVAL);
                }
            }
            
            if (*fldstart != '/' ) {
                if (*fldstart != '\0') {
                    fprintf(stderr, "#BLIB:  Error no command found expect / found \"%c\"\n", *fldstart);
                    cmdqual = &CMDQUALS[CMD_ERR];
                    goto getcmd_return_val;
                } else {
                    return(NULL);
                }
            } else {
                cmdqual = lookup_cmdqual(&fldstart); 		// move fldstart to 1st after a match
            }
            
            if (cmdqual->cmdid != CMD_ERR) { 			// if we didnt hit end of table
                fldstart = skipwspace(fldstart);		// I dont care about your dam quotes you not getting them around the cmp sign
                cmpflg = get_cmp(&fldstart);
                
                len = 0;
                if ( cmpflg > CMP_NONE) {			// was there a cmp sign ?
                    fldstart = skipwspace(fldstart);		// I still dont care about your quote status, effectively left trim
                    
                    if (fldstart && ( (*fldstart == '\"' ) || (*fldstart == '\''))) { // maybe an open
                        if (qchar2 == '\0') { 			// not already in a quote
                            qchar2 = *fldstart++; 		// capture and skip
                            fldend = index(fldstart, qchar2); 	// find the matching quote
                            
                            if (fldend != (char *) NULL ) {
                                if (fldend > cmdp ) { 		// if we havent already adjusted cmdp for fldend do it now
                                    cmdp = (fldend+1);  	// +1 to point past the qchar2 							//////////////////**************///////////////////
                                }
                                fldend--; 			// point back at the last char of the field rather than the now null'd quote
                            } else {
                                fprintf(stderr, "#BLIB: Error missing closing quote in command\n");
                                cmdqual = &CMDQUALS[CMD_ERR]; 	// all down hill from here
                                goto getcmd_return_val;
                            }
                        } else {
                            fprintf(stderr, "#BLIB: Error missmatched quotes in command, oh brother am I confused by your command\n");
                            cmdqual = &CMDQUALS[CMD_ERR];	// all down hill from here
                            goto getcmd_return_val;
                        }
                    } else {
                        if (qchar == '\0') {
                            // field value was not quoted so step until end of line or a wspace or a /
                            fldend = fldstart;
                            while ( fldend && (*fldend) && !index("/\t ", *fldend)) {
                                fldend++;
                            }
                            cmdp = fldend;								      //////////////////**************///////////////////
                            fldend--;
                        } else { // was a full quote on qchar
                                 // fldendwas already setup we gtg here with nothing needed to be done
                        }
                    }
                    if ((fldend == NULL) || (fldstart == NULL) || (fldstart > fldend)) {
                        fprintf(stderr, "#BLIB: " __PRETTY_FUNCTION__ " internal error ");
                        if (fldend == NULL) fprintf(stderr, " (fldend == NULL) ");
                        if (fldstart == NULL) fprintf(stderr, " (fldstart == NULL) ");
                        if (fldstart > fldend) fprintf(stderr, " fldstart > fldend ");
                        fputc('\n',stderr);
                        exit(ERANGE);
                    }
                    
                    len = fldend-fldstart+1; 			// length to the next slash
                    if (len >= sizeof(cmd_val)) {
                        fprintf(stderr, "#BLIB:  Error command line overflow\n");
                        exit(EINVAL);
                    }
                    memcpy(cmd_val,fldstart,len); // cmd_val is full of zero's always at start of this function
                    
                    rtrim(cmd_val,strlen(cmd_val));		// right trim white space
                }
                if (fldstart> cmdp) {
                    cmdp = fldstart; // no value so adjust the cmdp now					 //////////////////**************///////////////////
                }
                if (fldend > cmdp) {
                    cmdp = fldend;
                }
                
                switch (cmdqual->val_opt) {
                    case REQVAL_NONE:
                        if (len!=0) {
                            fprintf(stderr, "#BLIB:  Syntax error cmd %s does not support a value yet one is provided\n", cmdqual->cmdtxt);
                            cmdqual = &CMDQUALS[CMD_ERR]; 	// all down hill from here
                            goto getcmd_return_val;
                        }
                        break;
                    case REQVAL_REQ:
                        if (len==0) { 				// length of value as a string
                            fprintf(stderr, "#BLIB:  Syntax error cmd %s requires a value and none was provided\n", cmdqual->cmdtxt);
                            cmdqual = &CMDQUALS[CMD_ERR]; 	// all down hill from here
                            goto getcmd_return_val;
                        }
                        break;
                    case REQVAL_OPT:
                        break;
                }
                
            }
        getcmd_return_val:
            rval = newcmd(cmdqual,cmd_val, cmpflg);
            *cmdpp = cmdp;										 //////////////////**************///////////////////
        }
    }
    return(rval);
}

cmp_e get_cmp(char **cmdpp)
{
    cmp_e rval;
    char *cmdp;
    
    const char *cmpc="=><!,.?";
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
                        cmp[idx] = '<';
                        break;
                    case '.':
                        cmp[idx] = '>';
                        break;
                    default:
                        cmp[idx] = *sp;
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
                rval = CMP_EQ;			    // =
                break;
            case '>':
            case '.':
                rval = CMP_GT;			    // >
                break;
            case '<':
            case ',':
                rval = CMP_LT;			    // <
                break;
            case '!':
                rval = CMP_NE;			    // !
                break;
            case '?':                       // ?
                rval = CMP_OPT;
                break;
            default:
                rval = CMP_ERR;			    // WTF
                break;
        }
        
        switch(cmp[1])  {
            case '\0':				    // =, <, >, !
                                        // we good we have it already from cmp[0]
                break;
            case '?':
                if (rval == CMP_EQ) { // only ?= or =? are valid
                    rval = CMP_OPT;
                } else {
                    rval = CMP_ERR;
                }
                break;
                
            case '=':
                switch(rval) {
                    case CMP_NONE:
                        rval = CMP_EQ; // not possible but
                        break;
                    case CMP_EQ:		    // ==
                        rval = CMP_EQ;
                        break;
                    case CMP_GT:		    // >=
                        rval = CMP_GE;
                        break;
                    case CMP_LT:		    // <=
                        rval = CMP_LE;
                        break;
                    case CMP_NE:		    // !=
                        rval = CMP_NE;
                        break;
                    case CMP_OPT:           // ?=
                        rval = CMP_OPT;
                        break;
                    default:
                        rval = CMP_ERR;		    // WTF
                        break;
                }
                break;
                
            case '>':
            case '.':
                switch(rval) {
                    case    CMP_EQ:
                        rval = CMP_GE;  // =>
                        break;
                    case CMP_NE:
                        rval = CMP_LE;  // <=  "not >"
                        break;
                    default:
                        rval = CMP_ERR;
                        break;
                }
                break;
            case '<':
            case ',':
                switch(rval) {
                    case    CMP_EQ:
                        rval = CMP_LE;  // =<
                        break;
                    case CMP_NE:
                        rval = CMP_GE;  // >=  "not <"
                        break;
                    default:
                        rval = CMP_ERR;
                        break;
                }
                break;
                
            case '!':
                switch(rval) {
                    case    CMP_EQ:
                        rval = CMP_NE;  // =!
                        break;
                    case CMP_NE:
                        rval = CMP_EQ;  // =  "not not"
                        break;
                    case	CMP_LT: // <!  ie >=
                        rval = CMP_GE;
                        break;
                    case	CMP_GT: // >! ie <=
                        rval = CMP_LE;
                        break;
                    default:
                        rval = CMP_ERR;
                        break;
                }
                break;
                
            default:
                rval = CMP_ERR;			    // WTF
                break;
        }
        
        if (rval == CMP_ERR) {
            fprintf(stderr, "#BLIB:  Syntax error \"%s\" must be one of =    =<   =>   !=  !!  !< !>  \n", cmp);
        }
    }
    return(rval);
}

cmd_t  *cmd_new(cmdqual_t   *param, cmp_e	cmpflg)
{
    cmd_t *rval;
    
    if ((rval = malloc(sizeof(cmd_t))) == (cmd_t *) NULL ) {
        fprintf(stderr, "#BLIB:  Error allocating memory in " __ATLINE__ "\n");
        exit(ENOMEM);
    }
    bzero(rval, sizeof(cmd_t));
    
    rval->param  = param;
    rval->cmpflg = cmpflg;
    
    return(rval);
}

cmd_t *newcmd(cmdqual_t *cmdqual, char *val, cmp_e cmpflg)
{
    cmd_t   *rval;
    char    state_char, c;
    ssize_t	havevalue;
    
    rval = cmd_new(cmdqual, cmpflg);
    
    if (val)	{
        havevalue = strlen(val);
    } else	{
        havevalue = 0;
    }
    
    if (havevalue) { // have a value
        if (cmdqual->val_opt == REQVAL_NONE)  {
            fprintf(stderr, "#BLIB:  Syntax error, a value provided for cmd %s when no value required\n", cmdqual->cmdtxt);
            rval->param = &CMDQUALS[CMD_ERR];
            return(rval);
        }
    } else { /* do not havevalue */
        if (cmdqual->val_opt == REQVAL_REQ ) { // do we require one ?
            fprintf(stderr, "#BLIB:  Syntax error, command %s requires a value, none was provided\n", cmdqual->cmdtxt);
            rval->param = &CMDQUALS[CMD_ERR];
            return(rval);
        }
        rval->valset=VAL_NULL;
        return(rval);
    }
    
    if (cmdqual->validchar) { // do we have a character set to validate, yes then do it
        c = invalidchars(val, cmdqual->validchar);
        if ( c ) {
            fprintf(stderr, "#BLIB:  \'%c\' is not a valid character \"%s\"\n", c, val);
            rval->param = &CMDQUALS[CMD_ERR];
            return(rval);
        }
    }
    
    rval->valset=VAL_SET;
    
    switch(cmdqual->val_type) {
        case    VT_STR:
        case	VT_LABEL:
        case	VT_FILENAM:
            rval->val = newstr(val);
            break;
        case    VT_STATE:
            if (strcasecmp((char *) val, "ALLOCATED") == 0 ) {
                state_char = 'A';
            }
            else if (strcasecmp((char *) val, "FREE") == 0 ) {
                state_char = 'F';
            }
            else {
                fprintf(stderr, "#BLIB:  Syntax Error: state must be either \"ALLOCATED\" or \"FREE\" not %s\n", val);
                rval->param = &CMDQUALS[CMD_ERR];
                return(rval);
            }
            rval->val = newstr("%c", state_char);
            break;
        case    VT_INT:
            
            rval->val = newintstr(val);
            break;
        case    VT_INT64:
            rval->val = newuint64str(val);
            break;
            
        case 	VT_DATE:
            rval->val = new_time_blib_from_str((datestr_t *) val);
            break;
        case    VT_NONE:
            break;
    }
    return(rval);
}


cmd_t *newcmd_abs(cmdqual_t *cmdqual, void *val, cmp_e cmpflg)
{ // same as newcmd but with less validation and val is not a string, used internall to add to the qual list for log output
  // to include assumed/default and calculated values.
    
    cmd_t   *rval;
    
    rval = cmd_new(cmdqual, cmpflg);
    
    if (!val) {
        rval->valset = VAL_NULL;
    }
    rval->valset=VAL_SET;
    
    switch(cmdqual->val_type) {
        case    VT_STR:
        case	VT_LABEL:
        case	VT_FILENAM:
        case    VT_STATE:
            rval->val = newstr(val);
            break;
            
        case    VT_INT:
        case 	VT_DATE:
            rval->val = newintstr(val);
            break;
        case    VT_INT64:
            rval->val = newuint64str(val);
            break;
        default:
            fprintf(stderr, "#BLIB:  Impossible internal eerror val_type not value " __ATLINE__);
            exit(EINVAL);
            break;
    }
    return(rval);
}



char	*copy_cmd(char *dst, char *src)
{
    char    c;
    // char   *sp;
    if (!dst || !src) return(NULL);
    // sp = dst; // jus so we can see it
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
            case	'\'':
            case	'\"':
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

cmdqual_t *lookup_cmdqual_id(cmdqual_e cmdid)
{
    int idx;
    
    idx=CMD_ERR+1;
    
    while (CMDQUALS[idx].cmdid < CMD_END) {
        if (cmdid ==  CMDQUALS[idx].cmdid) {
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
        display_cmd(stderr, ptr);
        ptr = ptr->next;
    }
}

void	log_cmd(fio_t *logfd, cmd_t *hd)
{
    cmd_t *ptr;
    
    if (logfd && logfd->open ) {
        if (hd && hd->param && (hd->param->cmdid != CMD_REPLAY)) {
            fprintf(logfd->file, "%f: ", nowgm());
            ptr=hd;
            while(ptr) {
                display_cmd(logfd->file, ptr);
                ptr = ptr->next;
            }
            fputs("\n", logfd->file);
        }
    }
}

void	    display_cmd(FILE *outfd, cmd_t *cmd)
{
    char *cmpstr;
    
    fprintf(outfd, " %s", cmd->param->cmdtxt);
    if (cmd->valset==VAL_SET) {
        switch (cmd->cmpflg) {
            case CMP_LT:
                cmpstr=",";
                break;
            case CMP_LE:
                cmpstr=",=";
                break;
            case CMP_EQ:
                cmpstr="=";
                break;
            case CMP_GE:
                cmpstr=".=";
                break;
            case CMP_GT:
                cmpstr=".";
                break;
            case CMP_NE:
                cmpstr="!=";
                break;
            case CMP_OPT:
                cmpstr="?=";
                break;
            default:
                // includes CMP_NONE,
                cmpstr="";
                break;
        }
        fputs(cmpstr, outfd);
        
        switch (cmd->param->val_type) {
            case VT_NONE:
                break;
            case    VT_STR:
            case	VT_LABEL:
            case	VT_FILENAM:
                fprintf(outfd, "\"%s\"",(char *) cmd->val);
                break;
            case    VT_STATE:
                switch(*(char *)cmd->val) {
                    case    'F':
                        fprintf(outfd, "%s", "FREE");
                        break;
                    case    'A':
                        fprintf(outfd, "%s", "ALLOCATED");
                        break;
                    default:
                        fprintf(outfd, "%s", "UNKNOWN");
                        break;
                }
                break;
            case    VT_INT:
                fprintf(outfd, "%d", *( int *) cmd->val);
                break;
            case    VT_INT64:
                fprintf(outfd, "%llu", (llu_t) *( uint64_t *) cmd->val);
                break;
            case VT_DATE:
                //fprintf(outfd ,"%s", time_cvt_blib_to_str(*(blib_tim_t *) cmd->val));
                printlitdate(outfd, *( blib_tim_t *) cmd->val);
                
                break;
                
        }
    }
}

cmd_t *unlink_cmd(cmd_t **hd, cmd_t *qual)
{ // remove the found qualifer from the linked list
    cmd_t *prev;
    cmd_t *next;
    
    if (!hd || !qual) return(qual);
    prev = qual->prev;
    next = qual->next;
    
    if (prev == (cmd_t *) NULL) { // top or only entry in list
        if (next) {
            next->prev = (cmd_t *) NULL; // make next a have no previous
        }
        *hd=next; // reset the head pointer
        
    } else  if (next == (cmd_t *) NULL) { // bottom of list
        if (prev) { // which it must be since we already checked this
            prev->next = (cmd_t *) NULL; // make them the new bottom
        }
    } else {
        // must in the middle some place.
        prev->next = next;
        next->prev = prev;
    }
    qual->next = qual->prev = (cmd_t *) NULL;
    return(qual);
}

cmd_t *have_qual_unlink(cmd_t **hd, cmdqual_e qual)
{
    cmd_t  *ptr;
    cmd_t  *rval;
    
    rval = (cmd_t *) NULL;
    if (!hd) return(rval);
    
    ptr=*hd;
    while (ptr) {
        if (ptr->param->cmdid == qual) {
            rval = ptr;
            unlink_cmd(hd, ptr); // remove the found qualifer from the linked list
            break;
        }
        ptr = ptr->next;
    }
    return(rval);
}

cmd_t *have_qual(cmd_t **hd, cmdqual_e qual)
{
    cmd_t  *ptr;
    cmd_t  *rval;
    
    rval = (cmd_t *) NULL;
    if (!hd) return(rval);
    
    ptr=*hd;
    while (ptr) {
        if (ptr->param->cmdid == qual) {
            rval = ptr;
            break;
        }
        ptr = ptr->next;
    }
    return(rval);
}

cmd_t	    *checksyntax(cmd_t *hd)
{
    // Current checks: only one command given, /new only with /import, also look for any error cmds
    
    cmd_t   *rval;
    cmd_t   *ptr;
    cmd_t   *first_cmd, *cmd_new;
    bool_t  first_cmdmsgdone;
    
    if (!hd) {
        return((cmd_t *) NULL);		// just give up now, error should have already been reported
    }
    
    first_cmdmsgdone = NO;
    cmd_new = first_cmd = (cmd_t *) NULL;
    rval = hd;
    ptr=hd;
    // loop through the cmd/qual list make sure its not an error and that there is only 1 Command with 0 or more qualifiers
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
                    fprintf(stderr, "#BLIB:  Only one command allowed:-\n\t%s\n\t%s\n", first_cmd->param->cmdtxt, ptr->param->cmdtxt);
                    rval = 0;
                } else {
                    fprintf(stderr, "#BLIB:  \t%s\n", ptr->param->cmdtxt);
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
                case CMD_REPLAY:    // new only valid for import
                    break;
                default:
                    fprintf(stderr, "#BLIB:  /new only valid for when command is /replaylog[=<logfile>]\n");
                    rval = (cmd_t *) NULL;
                    break;
            }
    }
    return(rval);
}


void	do_cmd_help(FILE *fd)
{
    int	      idx;
    cmdqual_t *cmdp;
    char      *tail="";
    char      *defval;
    
    idx=1;
    
    while (CMDQUALS[idx].cmdid != CMD_END) {
        cmdp=&CMDQUALS[idx];
        if (cmdp->defval) 	defval = cmdp->defval;
        else			defval = cmdp->sql_fldnam;
        
        fprintf(fd, " %s", cmdp->cmdtxt);
        switch(cmdp->val_opt) {
            case REQVAL_NONE:
                tail="";
                break;
            case REQVAL_OPT:
                fprintf(fd,"[=");
                tail="]";
                break;
            case REQVAL_REQ:
                fprintf(fd,"=");
                tail="";
                break;
        }
        if (cmdp->val_opt != REQVAL_NONE) {
            switch(cmdp->val_type) {
                case    VT_NONE:
                    break;
                case    VT_STR:
                    fprintf(fd, "\"%s\"", (defval)?defval:"string" );
                    break;
                case	VT_LABEL:
                    fprintf(fd, "\"%s\"", (defval)?defval:"label" );
                    break;
                case	VT_FILENAM:
                    fprintf(fd, "\"%s\"", (defval)?defval:"filename");
                    
                    if (cmdp->val_opt == REQVAL_OPT ) tail="]";
                    break;
                case    VT_STATE:	// single char coded as A = ALLOCATED, F = FREE
                    fprintf(fd, "ALLOCATED|FREE");
                    break;
                case    VT_INT:
                    fprintf(fd, "0-%d", INT_MAX);
                    break;
                case    VT_INT64:
                    fprintf(fd, "0-%llu", INT64_MAX);
                    break;
                case    VT_DATE:
                    fprintf(fd, "dd-Mmm-YYYY:HH:MM:SS.CC");
                    break;
            }
        }
        
        if (cmdp->helptxt) {
            fprintf(fd,"%s\t\t- ",tail);
            if (defval) {
                fprintf(fd, cmdp->helptxt, defval);
            } else {
                fprintf(fd, "%s", cmdp->helptxt);
            }
            fputc('\n', fd);
        }
        idx++;
    }
    fprintf(fd, "\n\n\tWhen using command /report results can be filtered by most of the volume qualifiers eg. /state=FREE\n"
            "\tcomparisons other than equal are allowed to simplify keyboard input , can be used for < and . for >\n"
            "\t\totherwise < and > need to be escaped from the shell using them as input/output redireciton\n"
            "\tNOTE is the symbol !  ie NOT equal  /state!=FREE for state not equal to free this can also be just /state!FREE\n"
            "\tLess Than   ,  or <   /expire,$( caltime 0)  or \"/expire<$(caltime 0)\"\n"
            "\tLess than or Equal  ,= or <=\n"
            "\tEqual  = or ==\n"
            "\tNot Equal  !=\n"
            "\tGreater Than or equal .= or >=\n"
            "\tGreater than . or >\n"
            "\tOptional ?= or =? valid only for /includelogs will optional include the file only when errors are found\n"
            "\t in the backup beging reported\n"
            "Times maybe entered as absolute ctime values using the format /qual=ctime:%f\n", nowgm());
    
}


char *lookup_state(char *state_char)
{
    char *state;
    state="UNKNOWN";
    
    if (state_char) {
        switch(state_char[0]) {
            case 'A':
                state="ALLOCATED";
                break;
            case 'F':
                state="FREE";
                break;
        }
    }
    return(state);
}

char	*get_state(char *statestr)
{
    char    *state;
    
    if      (strcasecmp(statestr, "ALLOCATED" ) == 0) {
        state = "A";
    }
    else if (strcasecmp(statestr, "FREE"      ) == 0) {
        state = "F";\
    }
    else {
        state = "";
    }
    
    return(state);
}


int addqual(cmd_t **qualptr, cmdqual_e qtype, void *val)
{
    // create a new cmd_t for qtype and value then insert at end of qualptr chain
    // only if its not already there
    
    cmd_t	*ncmd;
    cmdqual_t	*qual;
    cmd_t	*fndqual;
    
    qual = lookup_cmdqual_id(qtype);
    if (qual && qual->cmdid != CMD_ERR) {
        fndqual = have_qual(qualptr, qtype);
        if (!fndqual) {
            ncmd = newcmd_abs(qual, val, CMP_EQ);
            insert_cmd_tail(qualptr, ncmd);
            return(1);
        } else {
            // fprintf(stderr, "#BLIB: attempt to add existing qualifier %s\n", fndqual->param->cmdtxt);
        }
    }
    return(0);
}

char	    *mkcmdline(int argc, char *argv[])
{
    char *rval;
    char *bufp;
    char *argvp;
    int idx;
    ssize_t	len;
    
    
    len = 0;
    for(idx=1;idx<argc;idx++) {
        len += (strlen(argv[idx])+2); // 2 because we going go quote it
    }
    if ((rval=malloc(len+1)) == (char *) NULL ) {
        fprintf(stderr, "#BLIB out of memory in " __ATLINE__ );
        exit(ENOMEM);
        
    }
    
    // string cat each of the argv[] elements into the malloc'd rval buffer
    bufp=rval;
    for(idx=1;idx<argc;idx++) {
        argvp = argv[idx];
        *bufp++ = '\''; // open single quote
        while(*argvp) {
            *bufp = *argvp;
            bufp++;
            argvp++;
        }
        *bufp++ = '\''; // close single quote
    }
    *bufp='\0'; // and terminate
    return(rval);
    
}


