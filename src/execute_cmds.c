static char *rcsid="@(#) $Id: execute_cmds.c,v 1.4 2010/11/24 00:55:40 mark Exp mark $";
/*
 * $Log: execute_cmds.c,v $
 * Revision 1.4  2010/11/24 00:55:40  mark
 * BUGFIX: reportexpire reportfree where very wrong based on older database structures
 * add func make_filter_rec to that these guys can set up a filter and use this inside modify_vol
 *
 * Revision 1.3  2010/11/16 04:08:34  mark
 * rc1
 *
 * Revision 1.2  2008/10/20  13:04:21  root
 * checkpoint
 *
 * Revision 1.1  2008/10/19  22:18:58  root
 * Initial revision
 *
 *
 *  execute_cmds.c
 *  blib
 *
 *  Created by mark on 28/09/2008.
 *  Copyright 2008 Garetech Computer Solutions. All rights reserved.
 * 
 */

#include "do_cmd_reportbackup.h"
#include "execute_cmds.h"
#include "timefunc.h"
#include "blib.h"

static char *ver()
{
    return(rcsid);
}

extern cmdqual_t CMDQUALS[];
extern blib_global_t BLIB;

int freeent(entry_t *ent)
{
    if (ent->e) free(ent->e);
    free(ent);
    return(0);
}

inline int getstate(state_t *state)
{
    // return true if free
    if ( state ) {
        return(state->str[0]);
    }
    return(0);
}

/*
 cmdline is free'd so caller must malloc it
 */
dbh_t *process_command_line(dbh_t *dbh, char *cmdline)
{
    dbh_t	*rval;
    cmd_t	*qual_log;
    cmd_t	*qual_nolog;
    cmd_t	*includelog;
    cmd_t	*cmds;
    char	*blib_log_fnm;
    
    
    if ((cmds = parseslashcmd(cmdline)) == (cmd_t *) NULL ) {
        exit(EINVAL);
    }
    
    qual_log   = have_qual_unlink(&cmds, QUAL_LOG);  	// this also make sure execute_cmds never sees /log
    qual_nolog = have_qual_unlink(&cmds, QUAL_NOLOG);  // this also make sure execute_cmds never sees /nolog
    
    while ( includelog = have_qual_unlink(&cmds, QUAL_INCLOG)) {
        new_files(&BLIB.includelogs, (char *) includelog->val);
    }
    
    if (qual_log && qual_nolog) {
        fprintf(stderr, "#BLIB:  Make up your mind do you want a log or not, both /log and /nolog found\n");
        exit(EINVAL);
    }
    
    if (!qual_nolog) {
        blib_log_fnm = get_default(QUAL_LOG);
        BLIB.blib_log = fio_alloc_open(blib_log_fnm, NULL, "a+", MAX_BUF);
        if (BLIB.blib_log->status) {
            fprintf(stderr, "#BLIB:  Error opening log file \"%s\", %s\n", blib_log_fnm,strerror(BLIB.blib_log->status));
            exit(BLIB.blib_log->status);
        }
    } else {
        BLIB.blib_log = fio_fd(stdout);
    }
    
    rval = execute_cmds(dbh, &cmds);
    log_cmd(BLIB.blib_log, cmds);
    fio_close_and_free(&BLIB.blib_log);
    free(cmdline);
    return(rval);
}

dbh_t *execute_cmds(dbh_t *dbh, cmd_t **cmds)
{
    cmd_t    *thecmd;
    cmd_t    *qual_ptr;
    cmd_t    *output_qual;
    cmd_t    *database_qual;
    
    int	     idx;
    int	     dbstatus;
    char     *fnm;
    char     *dbfnm;
    fio_t    *outfd;
    
    dbh= (dbh_t *) 0;
    
    if (output_qual=have_qual_unlink(cmds, QUAL_OUTPUT)) { // remove /output from qual chain if it exists
        fnm=(char *) output_qual->val;
        set_default(QUAL_OUTPUT, fnm);
        outfd=fio_alloc_open(fnm, NULL, "w", MAX_BUF);
        if (outfd->status) {
            fprintf(stderr, "#BLIB:  Error opening output file \"%s\"  %d:%s\n", fnm, outfd->status, strerror(outfd->status));
            exit(outfd->status);
        }
    } else {
        outfd=fio_fd(stdout);   // default output
    }
    
    if (dbh==(dbh_t *) NULL) {// not yet opened
        dbfnm = get_default(QUAL_DATABASE);
        if (database_qual = have_qual_unlink(cmds, QUAL_DATABASE)) { // remove /database from the qual chain if it exists
            dbfnm = (char *) database_qual->val;
            set_default(QUAL_DATABASE, dbfnm);
        }
        if (have_qual_unlink(cmds, QUAL_NEW)) {
            dbstatus = db_newdb(&dbh, dbfnm);
        } else {
            dbstatus = db_open(&dbh, dbfnm);
        }
        if (dbstatus != BLIBDB_OK ) {
            fprintf(stderr, "#BLIB:  Error opening database: ");
            if (dbh && dbh->errmsg ) fprintf(stderr, "%s\n", dbh->errmsg);
            else	fprintf(stderr, "#BLIB:  %d:%s\n", dbstatus,strerror(dbstatus));
            exit(dbstatus);   // if we didnt get the database to open the get out now
        }
    }
    
    thecmd = newcmd(lookup_cmdqual_id(CMD_ENV), NULL, CMP_NONE);
    
    if (cmds  && *cmds ) {
	    thecmd=*cmds;
    }
    
    qual_ptr=thecmd->next; // point to the first qualifier
    switch(thecmd->param->cmdid) {
        case    CMD_HELP:
            do_cmd_help(outfd->fd);
            break;
            
        case	CMD_ENV:
            do_cmd_env(outfd);
            break;
            
        case    CMD_DISPLAY:
        case    CMD_MODIFY:
        case    CMD_REMOVE:
            do_cmd_modify_volume(outfd, cmds, thecmd, qual_ptr, dbh);
            break;
            
        case    CMD_ADD:
            do_cmd_add_volume(outfd, cmds, thecmd, qual_ptr, dbh);
            break;
            
        case    CMD_REPORT:
            do_cmd_report(outfd,  cmds, thecmd, qual_ptr, dbh);
            break;
            
        case CMD_REPORTFRE:
            do_cmd_report_free(outfd,  cmds, thecmd, qual_ptr, dbh);
            break;
            
        case CMD_REPORTEXP:
            do_cmd_report_exp(outfd,  cmds, thecmd, qual_ptr, dbh);
            break;
            
        case CMD_DOEXPIRE:
            do_cmd_runexpiration(outfd,  cmds, thecmd, qual_ptr, dbh);
            break;
            
        case CMD_REPLAY:
            do_cmd_replaylog(outfd,  cmds, thecmd, qual_ptr, dbh);
            break;
            
        case CMD_NEWBCK:   
            do_cmd_newbackup(outfd,  cmds, thecmd, qual_ptr, dbh);
            break;
            
        case CMD_STARTBCK:
            do_cmd_startbackup(outfd,  cmds, thecmd, qual_ptr, dbh);
            break;
            
        case CMD_CHG_VOL:
            do_cmd_change_volume(outfd,  cmds, thecmd, qual_ptr, dbh);
            break;
            
    	case CMD_ENDBCK: 
            do_cmd_endbackup(outfd,  cmds, thecmd, qual_ptr, dbh);
            break;
            
        case CMD_ERRBCK:
            do_cmd_errbackup(outfd,  cmds, thecmd, qual_ptr, dbh);
            break;
            
        case CMD_FINBCK:
            do_cmd_finishbackup(outfd,  cmds, thecmd, qual_ptr, dbh);
            break;
            
        case CMD_DELBCK: 
            do_cmd_removebackup(outfd,  cmds, thecmd, qual_ptr, dbh);
            break;
            
        case CMD_MODBCK:
            do_cmd_modifybackup(outfd,  cmds, thecmd, qual_ptr, dbh);
            break;
            
        case CMD_REPBCK:
            do_cmd_reportbackup(outfd,  cmds, thecmd, qual_ptr, dbh);
            break;
            
        case CMD_LISTBCK:
            do_cmd_listbackups(outfd,  cmds, thecmd, qual_ptr, dbh);
            break;
        case CMD_LISTOBJ:
            do_cmd_listobjects(outfd,  cmds, thecmd, qual_ptr, dbh);
            break;
        case CMD_VERIFY:
            do_cmd_verifydb(outfd, cmds, thecmd, qual_ptr, dbh);
            break;
            
        default:
            fprintf(stderr, "#BLIB:  Error invalid initial command %s, initial command must be one of :-\n", thecmd->param->cmdtxt);
            idx=CMD_ERR+1;
            while(CMDQUALS[idx].cmdid != CMD_END) {
                if (CMDQUALS[idx].cmdtype == CMD ) fprintf(stderr, "#BLIB:  \t\t%s\n", CMDQUALS[idx].cmdtxt);
                idx++;
            }
            break;
    }
    
    if ( !outfd->useropenflag && outfd->fnm[0] ) {
        addqual(cmds, QUAL_OUTPUT, outfd->fnm);
    }
	
    fio_close_and_free(&outfd);
    
    return(dbh);
}    

int   modify_vol(cmd_t *thecmd, filt_t  *filtrec, vol_t *volrec, cmd_t *qual_ptr)
{
    int rval;
    
    rval = 0; // no error
    if ((rval = make_filter_rec(qual_ptr, filtrec)) >= 0 ) {
        if ((filtrec->media)  && (filtrec->media->valset == VAL_SET )) {
            copy_media(&volrec->media		, filtrec->media->val);
        }
        
    	if ((filtrec->groupname) && (filtrec->groupname->valset == VAL_SET)) {
            copy_groupname(&volrec->groupname	, filtrec->groupname->val);
        }
        
        if ((filtrec->location) && (filtrec->location->valset == VAL_SET)) {
            copy_location(&volrec->location	, filtrec->location->val);
        }
        
        if ((filtrec->state) && (filtrec->state->valset == VAL_SET)) {
            if (volrec->bck_id == 0 ) {
                copy_state(&volrec->state , filtrec->state->val);
            } else {
                fprintf(stderr, "#BLIB:  you may not modify STATE on volume %s as its part of a backup %llu\n", volrec->label.str, (llu_t) volrec->bck_id );
                exit(EPERM);
            }
        }
        
        if ((filtrec->offsitedate) && (filtrec->offsitedate->valset == VAL_SET)) {
            volrec->offsitedate = *(time_t *) filtrec->offsitedate->val;
        }
		
        if ((filtrec->librarydate) && (filtrec->librarydate->valset == VAL_SET)) {
            volrec->librarydate = *(time_t *) filtrec->librarydate->val;
        }
		
        if ((filtrec->usage) /* && (filtrec->usage->valset == VAL_SET) */) {
            if ((volrec->bck_id == 0 ) || (thecmd->param->dbaccess == DB_RO )) {
                if ( qual_ptr->valset == VAL_SET ) {
                    volrec->usage = *(int *) filtrec->usage->val;
                } else {
                    volrec->usage++;
                    // fill in the qualifier which had a missing value ie /usage with the new value calculated
                    qual_ptr->valset = VAL_SET;     // we have set a value make it so
                    replace_dynstr((char **) &qual_ptr->val,(char *) newint(volrec->usage));
                }
                filtrec->usage = qual_ptr;
            } else {
                fprintf(stderr, "#BLIB:  you may not modify /usage on volume %s as its part of a backup %llu\n", volrec->label.str,(llu_t) volrec->bck_id );
                exit(EPERM);
            }
        }
    }
    return(rval);
}

int   make_filter_rec(cmd_t *qual_ptr, filt_t  *filtrec)
{
    int rval;
    
    rval = 0; // no error
    bzero(filtrec, sizeof(filt_t));
    
    while(qual_ptr) {
        switch(qual_ptr->param->cmdid) {
            case QUAL_MEDIA:
                filtrec->media = qual_ptr;
                break;
            case QUAL_GROUP:
                filtrec->groupname = qual_ptr;
                break;
            case QUAL_LOCATION:
                filtrec->location = qual_ptr;
                break;
            case QUAL_STATE:
                filtrec->state = qual_ptr;
                break;
            case QUAL_OFFSITE:
                filtrec->offsitedate = qual_ptr;
                break;
            case QUAL_LIBDATE:
                filtrec->librarydate = qual_ptr;
                break;
            case QUAL_USAGE:
                filtrec->usage = qual_ptr;		
                break;
            case QUAL_RECORD:
                fprintf(stderr, "#BLIB:  /record date is not stored with the volume. Please see /reportbackups /listbackups the record date is with the backup id\n");
                rval=-1;
                break;
            default:
                fprintf(stderr, "#BLIB:  modify/filter volume does not support qualifier %s\n", qual_ptr->param->cmdtxt);
                rval=-1;
                break;
        }
        qual_ptr = qual_ptr->next;
    }
    return(rval);
}

char  *doenv(fio_t *outfio, char *symbol, valtype_e val_type,void *value)
{
    static char    buf[2048];
    char            *sp;
    char            val[2048];
    int             rlen, len, idx;
    
    val[0] = '\0';
    switch(val_type) {
        case VT_NONE:
            snprintf(val,sizeof(val), "");
            break;
        case    VT_STR:
        case	VT_LABEL:
        case	VT_FILENAM:
            snprintf(val,sizeof(val), "\047%s\047",(char *) value);
            break;
        case    VT_STATE:
            snprintf(val, sizeof(val),"%s", fmt_state((char *)value));
            break;	    
        case    VT_INT:
            snprintf(val,sizeof(val), "%d", *( int *) value);
            break;
        case    VT_INT64:
            snprintf(val,sizeof(val), "%llu", *( uint64_t *) value);
            break;
        case VT_DATE:
            snprintf(val,sizeof(val) ,"%s", fmtctime(*(time_t *) value));
            break;
            
    }
    
    rlen=sizeof(buf);
    
    snprintf(buf,rlen, "%s=", symbol);
    len = strlen(buf);
    sp = buf+len;
    // ??? rlen -= len;
    idx=0;
    while(val[idx]) {
#if 0
        switch(val[idx]) {
            case    ' ':
            case    '\t':
                *sp++ = '\\';
                break;
        }
#endif
        *sp++ = val[idx++];
    }
    *sp = '\0';
    
    if (buf[0]) {
        putenv(buf);
        fprintf(outfio->fd, "export %s;\n", buf);
    }
	
    return(buf);
}

int display_volume(fio_t *fd, vol_t *volrec)
{
    doenv(fd, "BLIB_BCK_ID"	, VT_INT64	, &volrec->bck_id);
    doenv(fd, "BLIB_VOLUME"	, VT_LABEL	, &volrec->label);
    doenv(fd, "BLIB_STATE"	, VT_STATE	, &volrec->state);
    doenv(fd, "BLIB_MEDIA"	, VT_STR	, &volrec->media);
    doenv(fd, "BLIB_USAGE"	, VT_INT	, &volrec->usage);
    doenv(fd, "BLIB_GROUP"	, VT_STR	, &volrec->groupname);
    doenv(fd, "BLIB_LOCATION"	, VT_STR	, &volrec->location);
    doenv(fd, "BLIB_LIBRARYDATE"  , VT_DATE	, &volrec->librarydate);
    doenv(fd, "BLIB_OFFSITEDATE"  , VT_DATE	, &volrec->offsitedate);
    
    // doenv(fd, "BLIB_RECORDDATE"   , VT_DATE	, &volrec->recorddate);
    // doenv(fd, "BLIB_FILENO"       , VT_INT	, &volrec->filecount);
    // doenv(fd, "BLIB_EXPIREDATE"   , VT_DATE	, &volrec->expiredate);
    // doenv(fd, "BLIB_BYTESONTAPE"  , VT_INT64	, &volrec->size);
    return(0);
}

int filtcmp(cmd_t *output_qual, void *val, int size)
{
    int rval;
    rval = 0;	 // display
    
    if (!output_qual) return(0);    // when it doubt put it out
    
    switch (output_qual->param->val_type) {
        case       VT_NONE:
            rval = 0;		// print all
            break;
        case    VT_STR:
            rval = strncasecmp((char *) output_qual->val,(char *) val, size);
            break;
        case VT_STATE:
            rval = ( *(char *) output_qual->val - *(char *)val);
            break;
        case    VT_INT:
            rval = ( *(int *)val - *(int *) output_qual->val);
            break;
        case VT_INT64:
            if (*(uint64_t *) output_qual->val == *(uint64_t *)val)	 rval=0;
            else if (*(uint64_t *) output_qual->val > *(uint64_t *)val) rval=-1;
            else						 rval=1;
            break;
        case    VT_DATE:
            rval = ( *(time_t *)val - *(time_t *)output_qual->val);
            break;
    }
    
    switch(output_qual->cmpflg) // rval =1 will skip record in filter_rec
    {
        case     CMP_ERR:
        case    CMP_NONE:
            return(0);	    // error so display everything
        case    CMP_LT:
            if (rval >= 0 ) rval=1; // <
            else	    rval=0;
            break;
        case    CMP_LE:
            if (rval > 0 ) rval=1; // <=
            else	   rval=0;
            break;
        case    CMP_EQ:
            if (rval != 0 ) rval=1; // =
            else	    rval=0;
            break;
        case    CMP_GE:
            if (rval < 0 ) rval=1;  // >=
            else	   rval=0;
            break;
        case    CMP_GT:
            if (rval <= 0 ) rval=1; // >
            else	    rval=0;
            break;
        case    CMP_NE:
            if (rval == 0 ) rval=1;  // !=
            else	    rval=0;
            break;
    }
    return(rval);
}

int filter_rec(filt_t *filtrec, vol_t *rec)
{ // return true to print record  0 false to skip it ie default 1 to skip
    
    if (filtcmp(filtrec->label      , &rec->label       , sizeof(rec->label)))	    return(0);	// mismatched
    if (filtcmp(filtrec->state      , &rec->state       , sizeof(rec->state)))	    return(0);	// mismatched
    if (filtcmp(filtrec->media      , &rec->media       , sizeof(rec->media)))	    return(0);	// mismatched
    if (filtcmp(filtrec->groupname  , &rec->groupname   , sizeof(rec->groupname)))  return(0);	// mismatched
    if (filtcmp(filtrec->location   , &rec->location    , sizeof(rec->location)))   return(0);	// mismatched
    
    if (filtcmp(filtrec->usage      ,(int *)  	&rec->usage      , sizeof(rec->usage)))	  	return(0);	// mismatched
    // if (filtcmp(filtrec->filecount  ,(int *)  	&rec->filecount  , sizeof(rec->filecount)))	return(0);	// mismatched
    // if (filtcmp(filtrec->recorddate ,(time_t *) &rec->recorddate , sizeof(rec->recorddate)))   	return(0);	// mismatched
    if (filtcmp(filtrec->offsitedate,(time_t *) &rec->offsitedate, sizeof(rec->offsitedate)))   return(0);	// mismatched
    // if (filtcmp(filtrec->expiredate ,(time_t *) &rec->expiredate , sizeof(rec->expiredate)))    return(0);	// mismatched
    // if (filtcmp(filtrec->size	    ,(bcount_t  *) &rec->size	 , sizeof(rec->size)))    	return(0);	// mismatched
    
    
    return(1); // default to display record
    
}

void do_cmd_env(fio_t *outfd)
{
    doenv(outfd, "BLIBDBS", VT_STR, get_default(QUAL_DATABASE));
    doenv(outfd, "MYBLIB_GROUP", VT_STR, get_default(QUAL_GROUP));
    doenv(outfd, "DAILYMEDIA", VT_STR, get_default(QUAL_MEDIA));
    doenv(outfd, "BLIB_LOG", VT_STR, get_default(QUAL_LOG));
    doenv(outfd, "BLIB_NODE", VT_STR, get_default(QUAL_NODE));
    
}

void do_cmd_modify_volume(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr, dbh_t *dbh)
{
    vol_t	volrec;
    vol_t	db_read;
    
    filt_t	filtrec;
    int		err;
    blabel_t	label;
    
    copy_label(&label ,  (char *) thecmd->val);
    
    // for these pre read the volume
    if (!db_find_volume_bylabel(dbh, &label, &db_read, FND_EQUAL)) {
        replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Volume: \"%s\" :NOT FOUND:\n", (char *) &label));
        if (dbh->status != BLIBDB_DONE ) {
            fprintf(outfd->fd, "#BLIB:  Unexpected error: %s\n", (char *) &dbh->errmsg);
            return;
        }
        dbh->status = ENOENT;
        return;
    } else {
        // load the values from the fetched record
        memcpy(&volrec,  &db_read, sizeof(volrec));
        
        switch(thecmd->param->cmdid) {
            case    CMD_DISPLAY:
                // /display=
                if (qual_ptr) {
                    fprintf(outfd->fd, "#BLIB:  Extra qualifiers to %s ignored\n", thecmd->param->cmdtxt);
                }
                display_volume(outfd, &volrec);
                break;
                
            case    CMD_MODIFY:
                // /modify=
                if ((err = modify_vol(thecmd, &filtrec, &volrec, qual_ptr)) !=0 ) { // update any values we have been given
                    fprintf(outfd->fd, "#BLIB:  Error modifying volume record %d\n", err);
                    return;
                }
                db_update_volume(dbh, &filtrec, &volrec);
                if (dbcheck(dbh,NULL)) {
                    if (!BLIB.quiet) fprintf(outfd->fd, "#BLIB:  \"%s\" modified\n",(char *) &volrec.label);				    
                } else {
                    fprintf(outfd->fd, "#BLIB:  Error updating volume %s %s\n", (char *) &volrec.label,(char *) &dbh->errmsg);
                    return;				
                }
                display_volume(outfd, &volrec);
                break;
                
            case    CMD_REMOVE:
                // /remove=
                if (qual_ptr) {
                    fprintf(outfd->fd, "#BLIB:  Extra qualifiers to %s ignored\n", thecmd->param->cmdtxt);
                }
                if (can_use(dbh, &volrec.label, 0) == 'F') {
                    db_delete_volume(dbh, &volrec); // delete from main.volumes where label=?
                    if (dbcheck(dbh,NULL)) {
                        replace_dynstr(&dbh->errmsg, newstr("#BLIB:  %s deleted\n", (char *) &volrec.label));
                    }
                } else {
                    replace_dynstr(&dbh->errmsg, newstr("#BLIB:  %s is inuse with backup id %llu and cannot be deleted\n", (char *) &volrec.label,
                                                        (llu_t) volrec.bck_id));
                    dbh->status = BLIBDB_CONSTRAINT;
                }
                break;
        }
    }
}


void	do_cmd_add_volume(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr, dbh_t *dbh)
{ //  /add=
    vol_t	db_key;
    vol_t	volrec;
    vol_t	db_volrec;
    int		err;
    filt_t	filtrec;
    blabel_t	label;
    
    bzero(&db_key, sizeof(vol_t));
    bzero(&volrec,sizeof(vol_t));
    bzero(&db_volrec,sizeof(vol_t));
    
    copy_label(&label ,  (char *) thecmd->val);
    
    copy_label(&db_key.label,  (char *) &label);
    default_volume(&volrec);
    copy_label(&volrec.label,  (char *) &label);
    
    if (!db_find_volume_bylabel(dbh, &label, &db_volrec, FND_EQUAL)) {  // not found so ok to add
    	if ( dbh->status != BLIBDB_ROW ) {
            if ((err = modify_vol(thecmd, &filtrec, &volrec, qual_ptr)) !=0 ) { // update any values we have been given
                replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Error modifying volume record %d\n", err));
                return;
            }
    	}
        
        db_insert_volumes(dbh,&volrec);
        switch(dbh->status) {
            case BLIBDB_DONE:
            case BLIBDB_OK:
                if (BLIB.debug) fprintf(outfd->fd, "#BLIB:  added volume label \"%s\"\n", (char *) &volrec.label);
                
                addqual(cmds, QUAL_STATE, volrec.state.str);
                
                addqual(cmds, QUAL_MEDIA, volrec.media.str);
                
                snprintf(outfd->buf, outfd->bufsiz, "%lu", (lu_t) volrec.usage);
                addqual(cmds, QUAL_USAGE, outfd->buf);
                
                addqual(cmds, QUAL_GROUP, volrec.groupname.str);
                
                addqual(cmds, QUAL_LOCATION, volrec.location.str);
                
                snprintf(outfd->buf, outfd->bufsiz, "%lu", (lu_t) volrec.librarydate);
                addqual(cmds, QUAL_LIBDATE, outfd->buf);
                
                snprintf(outfd->buf, outfd->bufsiz, "%lu", (lu_t) volrec.offsitedate);
                addqual(cmds, QUAL_OFFSITE, outfd->buf);
                
                break;
            case BLIBDB_CONSTRAINT:
                replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Duplicate key volume label \"%s\"\n", (char *) &volrec.label));
                break;
            default:
                replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Error storing tape label \"%s\" %d:%s\n", (char *) &volrec.label,dbh->status, (char *) &dbh->errmsg));
                break;
        }
        display_volume(outfd, &volrec);
    } else {
        replace_dynstr(&dbh->errmsg, newstr( "#BLIB:  Volume \"%s\" already in database you cannot add it again\n", (char *) &volrec.label));
        dbh->status = EEXIST;
        return;
    }
}


void do_cmd_report(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh)
{
    /*
     blib /report [/state ] [ /media ] [ /usage ] [ /group ] [ /location ] [ /libdate ] [ /offsite ] 
     where each of the optional filters have a filter state like = != ,= .= etc etc 
     these filters are never removed by modify_vol so they should come out in the log as they given
     */
    vol_t	db_key;
    vol_t	volrec;
    vol_t	db_read;
    backups_t	bck_rec;
    
    int		dbrstatus;
    int		err;
    int		filecount=0;
    filt_t	filtrec;
    datestr_t 	librarydate;
    datestr_t 	recorddate;
    datestr_t 	offsitedate;
    datestr_t 	expiredate;
    
    if (BLIB.debug) fprintf(stderr, "cmd: %s \n", thecmd->param->cmdtxt);
    
    bzero(&volrec, sizeof(vol_t)); // clear all defaults we only want those filter values
    err = modify_vol(thecmd, &filtrec, &volrec, qual_ptr);
    if (err !=0 ) { // update the volrec from cmd qualifiers use as filter
        return;
    }
    bzero(&db_key,sizeof(vol_t));
    bzero(&db_read,sizeof(vol_t));
    if (!BLIB.quiet) {
        fprintf(outfd->fd,"Volume    #Files  #use  Group     Location   Media     State Expires           Offsite           Record             Description\n"                                              
                "--------- ------ ------ --------- ---------- --------- ----- ----------------- ----------------- -----------------  -----------\n");   
        //	               Volume      #Files  #use  Group    Location     Media    State      Expires           Offsite           Record             Description
        //	               ------------- ----- ----- -----    --------     -----    -----      ----------------- ----------------- -----------------  ----------------
        //	               TST002              0       0 TESTGRP  testloc      TZ89     ALLOCATED  01-JAN-1970:10:00 01-JAN-1970:10:00 01-JAN-1970:10:00  New backup tes
    }
    
    dbrstatus = db_find_volumes_label(dbh, &db_key, &db_read, FND_FIRST);
    while (dbrstatus) {
        if (filter_rec(&filtrec,&db_read)) {
            copy_datestr(&librarydate, (datestr_t *) fmtctime(db_read.librarydate));
            copy_datestr(&offsitedate, (datestr_t *) fmtctime(db_read.offsitedate)); 
            
            bzero(&bck_rec, sizeof(bck_rec));
            filecount = 0;
            if (db_read.bck_id) {
                if (db_find_backups_by_bck_id(dbh, db_read.bck_id, &bck_rec)) {
                    filecount =  db_count_vol_obj_label(dbh, db_read.bck_id, &db_read.label);
                }
            }
            copy_datestr(&recorddate,  (datestr_t *) fmtctime(bck_rec.start));     // bck_rec was zero'd if we didn't find one to match
            copy_datestr(&expiredate,  (datestr_t *) fmtctime(bck_rec.expiredate));
            
            
            fprintf(outfd->fd,"%-9.9s %6d %6d %-9.9s %-10.10s %-9.9s %-5.5s %-17.17s %-17.17s %-17.17s  %s\n", 
                    (char *) &db_read.label,
                    filecount,
                    db_read.usage,
                    (char *) &db_read.groupname, 
                    (char *) &db_read.location, 
                    (char *) &db_read.media, 
                    lookup_state((char *) &db_read.state), 
                    expiredate.str, 
                    offsitedate.str, 
                    recorddate.str, 
                    (db_read.bck_id)?(char *) &bck_rec.desc:""
                    );
        }
        dbrstatus = db_find_volumes_label(dbh, NULL, &db_read, FND_NEXT);
    }
    if (!BLIB.quiet) fprintf(outfd->fd, "* End of Report *\n");
}

void do_cmd_report_free(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh)
{  // /reportfree
    // TODO: add suport for /xml /html /mail /stylesheet notyet though
    vol_t	volrec;
    vol_t	db_read;
    int		dbrstatus;
    int		err;
    filt_t	filtrec;
    int		toupdate;
    int		idx;
    
    ver();
    // select * from volumes where bck_id=0 and state='F' order by recorddate asc
    toupdate = 9999;
    idx=0;
    if (thecmd->valset == VAL_SET) {
	    if (thecmd->val) toupdate = * (int *) thecmd->val;
    }
    if (toupdate==0) toupdate = 9999;
    
    bzero(&volrec, sizeof(vol_t)); // clear all defaults we only want those filter values
    
    if ((err = make_filter_rec(qual_ptr, &filtrec )) !=0 ) { // update the volrec from cmd qualifiers use as filter
        fprintf(outfd->fd, "#BLIB:  Error setting up volume filter %d\n", err);
        return;
    }
    
    bzero(&db_read,sizeof(vol_t));
    if (!BLIB.quiet) {
        fprintf(outfd->fd,"#BLIB:  Looking for %d %s\n", toupdate, (char *) &volrec.media);                   
    }
    
    dbrstatus = db_find_volume_free(dbh, &db_read, FND_FIRST);
    while (dbrstatus) {
        if (filter_rec(&filtrec,&db_read)) {
            if (idx < toupdate) {
                idx++;
                fprintf(outfd->fd,"%-13.13s\n", (char *) &db_read.label);
            }
        }
        dbrstatus = db_find_volume_free(dbh, &db_read, FND_NEXT);
    }
    dbh->status = toupdate-idx;
    replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Could not find the request number of tapes free %d short\n", dbh->status ));
    return;
}


void do_cmd_report_exp(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh)
{ // reportexpired
    vol_t	volrec;
    vol_t	db_read;
    int		dbrstatus;
    int		err;
    filt_t	filtrec;
    int		toupdate;
    int		idx;
    
    // select * from volumes where bck_id=0 and state='A' order by bck_id asc
    toupdate = 9999;
    idx=0;
    if (thecmd->valset == VAL_SET) {
	    if (thecmd->val) toupdate = * (int *) thecmd->val;
    }
    if (toupdate == 0 ) toupdate=9999;
    
    bzero(&volrec, sizeof(vol_t)); // clear all defaults we only want those filter values
    if ((err = make_filter_rec(qual_ptr, &filtrec )) !=0 ) { // update the volrec from cmd qualifiers use as filter
        fprintf(outfd->fd, "#BLIB:  Error setting up volume filter %d\n", err);
        return;
    }
    
    bzero(&db_read,sizeof(vol_t));
    if (!BLIB.quiet) {
        fprintf(outfd->fd,"#BLIB:  Looking for %d %s\n", toupdate, (char *) &volrec.media);                   
    }
    
    dbrstatus = db_find_volume_expired(dbh, &db_read, FND_FIRST);
    while (dbrstatus) {
        if (filter_rec(&filtrec,&db_read)) {
            if (idx < toupdate) {
                idx++;
                fprintf(outfd->fd,"%-13.13s\n", (char *) &db_read.label);
            }
        }
        dbrstatus = db_find_volume_expired(dbh, &db_read, FND_NEXT);
    }
    dbh->status = toupdate-idx;
    replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Could not find the request number of tapes free %d short\n", dbh->status));
    return;
}

void do_cmd_runexpiration(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh)
{ // /runexpiration[=n] - expire enough backups for all or to a given number of free volumes 
    
    backups_t	bckrec;
    int		dbrstatus;
    int		toupdate;
    int		reqflg;
    cmd_t	*output_qual;
    time_t	nowgmt;
    bcount_t	volumesinbackup;
    bcount_t	volumesfreed;
    
    toupdate = -1;
    
    if (BLIB.debug == 99999 ) fprintf(outfd->fd, "Debug:\n");
    if (thecmd->valset == VAL_SET) {
	    if (thecmd->val) toupdate = * (int *) thecmd->val;
    }
    if (toupdate == 0 ) toupdate=9999;
    
    output_qual=qual_ptr;
    reqflg=0;
    
    while(output_qual) {
        fprintf(stderr, "#BLIB:  Error invalid qualifier %s given to %s\n", output_qual->param->cmdtxt, thecmd->param->cmdtxt);
        reqflg++;
        output_qual=output_qual->next;
    }
    if (reqflg) {
        exit(EINVAL);
    }
    nowgmt = nowgm();
    
    volumesfreed = 0;
    dbrstatus = db_find_backups_by_expire(dbh, &bckrec, FND_FIRST);
    while ((dbrstatus ) && (volumesfreed <= toupdate)) {
        if (BLIB.debug > 1 ) {
            fprintf(stderr, "Checking backup id %llu\n", (llu_t) bckrec.bck_id);
        }
        if (bckrec.expiredate < nowgmt) {
            volumesinbackup = db_count_volumes_in_backup_id(dbh, bckrec.bck_id);
            if (db_delete_backup_id(dbh, bckrec.bck_id)) {
                fprintf(outfd->fd, "Deleted backup id: %llu\n", (llu_t ) bckrec.bck_id);
                volumesfreed += volumesinbackup;
            }
        }
        dbrstatus = db_find_backups_by_expire(dbh, &bckrec, FND_NEXT);
    }
    if (dbcheck(dbh, NULL)) {
    	dbh->status = volumesfreed;
    	replace_dynstr(&dbh->errmsg, newstr("<< %d >> volumes free'd\n", volumesfreed));
    } else {
        dbh->status = -1;
    }
    return;
}


void do_cmd_replaylog(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh)
{ // /replaylog[=logfile] [/since=<n>]
    
    char    	*fnm;
    
    int		reqflg;
    cmd_t	*output_qual;
    qual_t	qualval;
    time_t	cmdtime;
    char	*cmdline;
    int		skip;
    size_t	line;
    fio_t	*loginfd;
    
    bzero(&qualval, sizeof(qual_t));
    reqflg=0;
    output_qual=qual_ptr;
    while(output_qual) {
        switch(output_qual->param->cmdid) {	
            case QUAL_SINCE:
                qualval.since = *(int *) output_qual->val;
                reqflg |= 0;	// optional
                break;
                
            case QUAL_UNTIL:
                qualval.until = *(int *) output_qual->val;
                reqflg |= 0;	// optional
                break;	
                
            default:
                fprintf(stderr, "#BLIB:  Error invalid qualifier %s given to %s\n", output_qual->param->cmdtxt, thecmd->param->cmdtxt);
                return;
                break;
        }
        output_qual=output_qual->next;
    }
    if (reqflg) {
        exit(EINVAL);
    }
    if (thecmd->valset == VAL_SET ) {
        fnm = (char *) thecmd->val;
    } else {
    	fnm = get_default(QUAL_LOG); // get the value set for log file name
    }
    if (fnm && *fnm) {
        loginfd = fio_alloc_open(fnm, NULL, "r", MAX_BUF);
        if (loginfd->status) {
            dbh->status=errno;
            replace_dynstr(&dbh->errmsg,newstr("#BLIB:  Error opening log file \"%s\", %s\n", fnm,strerror(dbh->status)));
            return;
        }
    } else {
        loginfd=fio_fd(stdin);
    }
    fio_close(BLIB.blib_log);
    BLIB.blib_log = fio_dup(outfd);
    set_default(QUAL_LOG, outfd->fnm); // set default so the log open will open this file
    line=0;
    fio_fgets(loginfd);
    while (!feof(loginfd->fd)) {
        line++;
        if ((cmdline=index(loginfd->buf, ':')) == (char *) NULL) {
            replace_dynstr(&dbh->errmsg, newstr("#BLIB:  corrupt log file line missing :\n"));
            return;
        }
        *cmdline++ ='\0'; // null the colon
        cmdtime = atoi(loginfd->buf);
        skip=0; // default not skip
        
        if (cmdtime==0) {
            skip=1;	// skip the line must have been an error as cmdtime==0
            if (BLIB.debug) fprintf(stderr, "#BLIB:  Skipping log line %lu as command time is 0\n", (lu_t) line);
        }
        
        if ((!skip) && (qualval.since)) {
            if (cmdtime < qualval.since) {
                skip=1;
            }
        }
        
        if ((!skip) && (qualval.until)) {
            if (cmdtime > qualval.until) {
                skip=1;
            }
        }
        
        if (skip) {
            if (BLIB.debug) fprintf(stderr, "#BLIB:  Skipping log line %lu as command time is less than value from /since\n", (lu_t) line);
        } else {
            fprintf(stderr, "#BLIB: replaying: %lu: blib %s\n", (lu_t) cmdtime,  cmdline);
            process_command_line(dbh, newstr(cmdline));
            if (dbh) {
                if (dbh->status) {
                    if (strcasecmp(dbh->errmsg, "not an error")) {
                        if (dbh->errmsg ) fputs(dbh->errmsg, stderr);
                    }
                }
            } else { // WTF
                fprintf(stderr, "#BLIB:  database handle null when it should never have been\n");
                exit(EINVAL);
            }	    
        }
        fio_fgets(loginfd);
    }
    fio_close_and_free(&loginfd);
}

void do_cmd_newbackup(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh)
{
    /* /newbackup   "Establish a new backup and return its bck_id
     /record /expire /desc /node /bck_id
     */
    backups_t	bck_rec;
    qual_t	qualval;
    cmd_t	*output_qual;
    time_t	whatnow;
    int		dbstatus;
    int		reqflg;
    
    bzero(&qualval, sizeof(qual_t));
    reqflg=0;
    output_qual=qual_ptr;
    while(output_qual) {
        switch(output_qual->param->cmdid) {	
            case QUAL_BCKID:
                qualval.bck_id = *(bckid_t *) output_qual->val;
                reqflg |= 0;	// optional
                break;	
            case QUAL_DESC:
                copy_desc(&qualval.desc,  (char *) output_qual->val);
                reqflg |= 1;
                break;
            case QUAL_RECORD:
                qualval.start = *(time_t *) output_qual->val;
                reqflg |= 2;
                break;
            case QUAL_EXPIRE:
                qualval.expiredate = *(time_t *) output_qual->val;
                reqflg |= 4;
                break;
            case QUAL_NODE:
                copy_node(&qualval.node,  (char *) output_qual->val);
                break;
            default:
                fprintf(stderr, "#BLIB:  Error invalid qualifier %s given to %s\n", output_qual->param->cmdtxt, thecmd->param->cmdtxt);
                return;
                break;
        }
        output_qual=output_qual->next;
    }
    
    if (reqflg != 7) {
        fprintf(stderr, "#BLIB:  Error command %s requires /desc= /record= /expire=\n", thecmd->param->cmdtxt);
        return;
    }
    
    if (qualval.node.str[0] == 0) {
        copy_node(&qualval.node, get_default(QUAL_NODE));		// set the default for /node if it wasnt provided
        addqual(cmds, QUAL_NODE, qualval.node.str);			// add the /node= into the command string for the log
    }
    
    whatnow = nowgm();
    bzero(&bck_rec, sizeof(backups_t));
    if (qualval.bck_id) {
        bck_rec.bck_id     = qualval.bck_id;
    } else {
        bck_rec.bck_id     = whatnow;
    }
    copy_node(&bck_rec.node, (char *) &qualval.node);
    bck_rec.start      = qualval.start;
    bck_rec.end        = 0;  // crystal ball is broken again ;(
    bck_rec.expiredate = qualval.expiredate;
    copy_desc(&bck_rec.desc, (char *) &qualval.desc);
    
    bck_rec.bck_id--; // so we can make the loop simple ;(
    do {
    	bck_rec.bck_id++;
        dbstatus = db_insert_backups(dbh, &bck_rec);
    } while ( (qualval.bck_id == 0 ) && (dbstatus == BLIBDB_CONSTRAINT) && (bck_rec.bck_id < whatnow+10000));
    
    if (dbcheck(dbh,NULL)) {
        snprintf(outfd->buf, outfd->bufsiz, "%llu",(llu_t) bck_rec.bck_id);
        addqual(cmds, QUAL_BCKID, outfd->buf);
        fprintf(outfd->fd,  "BLIB_BCKID=\047%s\047\n",outfd->buf);
    } else {
        fprintf(stderr, "#BLIB:  Error creating new backup\n");
    }
}

void do_cmd_startbackup(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh)
{
    /*	/startbackup    "start backup of object on a given /label="
     
     /record /objinstance /bck_id /label
     
     blib /startbackup=<objname> /bck_id=<bckid> /label=<label> /record=<startdate> /node=
     
     insert into bck_objects (bck_id, name, start, end, size) values ( <bckid> , <objname> , <startdate> , 0, 0 );
     
     fileno = select count(*) from vol_obj where bck_id=<bckid> and label=<label>;
     insert into vol_obj (bck_id, objname, label, fileno, start, end, size) 
     values (<bckid>, <objname>, <label>, <startdate>, 0, 0, 0, '');
     
     remove old references to the label
     for x_bck_id, x_objname in (select bck_id, objname from vol_obj where bck_id != <bckid> and label=<label>)
     do
     delete from bck_objects where bck_id=<x_bck_id> and objname=<x_objname>;
     delete from vol_obj where bck_id=<x_bck_id> and objname=<objname>;
     done
     
     */
    int		reqflg;
    cmd_t	*output_qual;
    int		fileno;
    bckobj_t 	bckobjrec;
    qual_t	qualval;
    
    vol_obj_t	volobjrec;
    objid_t     obj_instance;
    backups_t   backup;
    
    bzero(&qualval,sizeof(qual_t));
    
    copy_objname(&qualval.objname, (char *) thecmd->val);
    
    output_qual=qual_ptr;
    reqflg=0;
    while(output_qual) {
        switch(output_qual->param->cmdid) {
            case QUAL_BCKID:
                qualval.bck_id = *(bckid_t *) output_qual->val;
                reqflg |= 1;
                break;
            case QUAL_LABEL:
                copy_label(&qualval.label,  (char *) output_qual->val);
                reqflg |= 2;
                break;
            case QUAL_RECORD:
                qualval.start = *(time_t *) output_qual->val;
                reqflg |= 4;
                break;
            case QUAL_OBJINS:
                qualval.obj_instance = * (objid_t *) output_qual->val;
                break;
            default:
                replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Error invalid qualifier %s given to %s\n", output_qual->param->cmdtxt, thecmd->param->cmdtxt));
                dbh->status = EINVAL;
                return;
                break;
        }
        output_qual=output_qual->next;
    }
    
    
    if (reqflg != 7 ) {
        replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Error %s requires /bck_id= /label= /start= one of them was missing %s\n", thecmd->param->cmdtxt, thecmd->param->helptxt));
        dbh->status = EINVAL;
        return;
    }
    
    if (!db_find_backups_by_bck_id(dbh, qualval.bck_id, &backup)) {
        replace_dynstr(&dbh->errmsg, newstr("#BLIB: cannot add backup objects with invalid bck_id: %llu\n", qualval.bck_id));
        dbh->status = ENOENT;
        return;
    }
    
    if (usetape(dbh, &qualval.label, qualval.bck_id) < 0 ) {
        fprintf(outfd->fd, "#BLIB:  Cannot use volume %s not free\n", (char *)  &qualval.label);
        return;
    }
    
    //insert into bck_objects (bck_id, name, start, end, size) values ( <bckid> , <objname> , <startdate> , 0, 0 );
    bzero(&bckobjrec,sizeof(bckobj_t));
    bckobjrec.bck_id = qualval.bck_id;
    if (qualval.obj_instance) {
        bckobjrec.obj_instance = qualval.obj_instance;
    }
    copy_objname(&bckobjrec.objname, (char *) &qualval.objname);
    bckobjrec.start  = qualval.start;
    bckobjrec.end    = 0;
    bckobjrec.size   = 0;
    
    obj_instance = db_insert_bck_objects(dbh, &bckobjrec);
    if (dbcheck(dbh, NULL)) {
        fileno = db_count_vol_obj_label(dbh, qualval.bck_id, &qualval.label);  
        
        bzero(&volobjrec,sizeof(vol_obj_t));
        volobjrec.bck_id   = bckobjrec.bck_id ;
        copy_objname(&volobjrec.objname, (char *) &qualval.objname);
        volobjrec.obj_instance = obj_instance;
        copy_label(&volobjrec.label    , (char *) &qualval.label);
        volobjrec.fileno   = fileno;
        volobjrec.start    = qualval.start;
        volobjrec.end      = 0;
        volobjrec.size     = 0;
        
        if (db_insert_vol_obj(dbh, &volobjrec)) {
#ifdef db_clear_old_back_for_label_tmp
            db_clear_old_back_for_label(dbh, &volobjrec);
            if (!dbcheck(dbh, NULL)) {
                fprintf(outfd->fd, "#BLIB:  Error removing old usage against volume %s\n", (char *) &volobjrec.label);
                obj_instance = 0; // 0 to indicate failure
            }
#endif
        }
    } else {
        fprintf(outfd->fd, "#BLIB:  Error inserting backup object\n");
    }
    snprintf(outfd->buf, outfd->bufsiz, "%lu", (lu_t) obj_instance); 
    addqual(cmds, QUAL_OBJINS, outfd->buf);
    doenv(outfd, "BLIB_VOLID", VT_INT, &obj_instance);	// instance of the volume
}

void do_cmd_change_volume(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh)
{
    /*  /change_volume   "close current volume of object start new on /label="
     /objinstance /bck_id /label /size /end
     
     blib /change_volume=<objname> /bck_id=<bckid> /label=<newlabel> /end=<enddate>  /size=<size>
     * close old volume on size
     * open new volume <newlabel>
     cur_label = select label from vol_obj where bck_id=<bckid> and objname=<objname> order by fileno desc limit to 1 rows; // need to make sure that limit applies after sort
     update vol_obj set size=<size> where bck_id=<bckid> and objname=<objname> and label=<cur_label>;
     insert into vol_obj (bck_id, objname, label, size, fileno) values (<bckid>, <objname>, <newlabel>, 0, 0);
     */
    int		reqflg;
    cmd_t	*output_qual;
    vol_obj_t	volobjrec;
    qual_t	qualval;
    
    bzero(&qualval, sizeof(qual_t));
    copy_objname(&qualval.objname, (char *) thecmd->val);
    output_qual=qual_ptr;
    
    reqflg=0;
    qualval.obj_instance = 1;
    while(output_qual) {
        switch(output_qual->param->cmdid) {
            case QUAL_BCKID:
                qualval.bck_id = *(bckid_t *) output_qual->val;
                reqflg |= 1;
                break;
            case QUAL_LABEL:
                copy_label(&qualval.label,  (char *) output_qual->val);
                reqflg |= 2;
                break;
            case QUAL_ENDBCK:
                qualval.end = *(time_t *) output_qual->val;
                reqflg |= 4;
                break;
            case QUAL_SIZE:
                qualval.size = *( uint64_t *) output_qual->val;
                reqflg |= 8;
                break;
            case QUAL_OBJINS:
                qualval.obj_instance = *(unsigned int *) output_qual->val;
                break;
            default:
                fprintf(stderr, "#BLIB:  Error invalid qualifier %s given to %s\n", output_qual->param->cmdtxt, thecmd->param->cmdtxt);
                return;
                break;
        }
        output_qual=output_qual->next;
    }
    
    if (reqflg != 15) {
        fprintf(stderr, "#BLIB:  Error %s requires /bck_id=<bckid> /label=<newlabel> /end=<time>   /size=<size> one of them was missing %s\n", thecmd->param->cmdtxt, thecmd->param->helptxt);
        exit(EINVAL);
    }
    
    if ( usetape(dbh, &qualval.label, qualval.bck_id ) < 0 ) {
        fprintf(outfd->fd, "#BLIB:  Cannot use volume %s not free\n", (char *)  &qualval.label);
        return;
    }
    
    
    bzero(&volobjrec, sizeof(vol_obj_t));
    volobjrec.bck_id = qualval.bck_id;
    copy_objname(&volobjrec.objname, (char *) &qualval.objname);
    volobjrec.obj_instance = qualval.obj_instance;
    
    if (!db_find_current_volobj(dbh, &volobjrec)) {
        replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Request to change volume yet no current volume exists\n"));
        return;
    }
    // now close volobjrec we got back
    volobjrec.end = qualval.end;
    volobjrec.size = qualval.size;
    db_end_vol_obj(dbh, &volobjrec);
    
    // now create a new entry
    bzero(&volobjrec,sizeof(vol_obj_t));
    volobjrec.bck_id = qualval.bck_id ;
    copy_objname(&volobjrec.objname, (char *) &qualval.objname);
    copy_label(&volobjrec.label, (char *) &qualval.label);
    volobjrec.obj_instance = qualval.obj_instance;
    volobjrec.fileno   = 0; 			// we starting on a new volume
    volobjrec.start    = qualval.end+1;	// start from the end of the previous plus 1 second
    volobjrec.end      = 0;			// not ended yet
    volobjrec.size     = 0;			// size not yet known
    
    if (db_insert_vol_obj(dbh, &volobjrec)) {
#ifdef db_clear_old_back_for_label_tmp
        db_clear_old_back_for_label(dbh, &volobjrec);
        if (!dbcheck(dbh, NULL)) {
            fprintf(outfd->fd, "#BLIB:  Error removing old usage against volume %s\n", (char *) &volobjrec.label);
        }
#endif
    } else {
        fprintf(outfd->fd, "#BLIB:  Error inserting new volume usage record label=%s\n", (char *) &qualval.label);
        if (BLIB.debug > 5) dump_vol_obj(&volobjrec);
    }
}


void do_cmd_endbackup(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh)
{
    /*  /endbackup   "finish backup of object"
     
     /end /size /objinstance /bck_id /label 
     
     blib /endbackup=<objname> /bck_id=<bckid> /label=<currentlabel> /objinstance=<n> /end=<enddate>  /size=<size>
     * close old volume on size
     * open new volume <newlabel>
     cur_label = select label from vol_obj where bck_id=<bckid> and objname=<objname> order by fileno desc limit to 1 rows; // need to make sure that limit applies after sort
     verify that what we think is the cur_label matches what we told?
     
     update vol_obj set size=<size> where bck_id=<bckid> and objname=<objname> and label=<cur_label>;
     
     */
    vol_obj_t	volobjrec;
    bcount_t	totalsize;
    
    int		reqflg;
    cmd_t	*output_qual;
    qual_t	qualval;
    
    
    if (BLIB.debug == 9999 ) fprintf(outfd->fd, "debug:\n");
    bzero(&qualval, sizeof(qual_t));
    copy_objname(&qualval.objname, (char *) thecmd->val);
    output_qual=qual_ptr;
    reqflg=0;
    qualval.obj_instance = 1; // default to 1 if they dont say which 
    while(output_qual) {
        switch(output_qual->param->cmdid) {
            case QUAL_BCKID:
                qualval.bck_id = *(bckid_t *) output_qual->val;
                reqflg |= 1;
                break;
            case QUAL_LABEL:
                copy_label(&qualval.label,  (char *) output_qual->val);
                reqflg |= 2;
                break;
            case QUAL_ENDBCK:
                qualval.end = *(time_t *) output_qual->val;
                reqflg |=4;
                break;
            case QUAL_SIZE:
                qualval.size = *( uint64_t *) output_qual->val;
                reqflg |= 8;
                break;
                
            case QUAL_OBJINS:
                qualval.obj_instance = *(unsigned int *) output_qual->val;
                break;
            default:
                fprintf(stderr, "#BLIB:  Error invalid qualifier %s given to %s\n", output_qual->param->cmdtxt, thecmd->param->cmdtxt);
                return;
                break;
        }
        output_qual=output_qual->next;
    }
    
    if (reqflg != 15 ) {
        fprintf(stderr, "#BLIB:  Error %s requires /bck_id=<bckid> /label=<newlabel>   /size=<size> one of them was missing %s\n", thecmd->param->cmdtxt, thecmd->param->helptxt);
        exit(EINVAL);
    }
    
    bzero(&volobjrec, sizeof(vol_obj_t));
    volobjrec.bck_id = qualval.bck_id;
    copy_objname(&volobjrec.objname, (char *) &qualval.objname);
    copy_label(&volobjrec.label, (char *) &qualval.label);
    volobjrec.obj_instance = qualval.obj_instance;
    if (!db_find_current_volobj(dbh, &volobjrec)) {
        replace_dynstr(&dbh->errmsg,newstr("#BLIB:  Request to end volume yet no current open volume exists\n"));
        return;
    }
    if ( cmp_labels(&volobjrec.label, &qualval.label) == 0 ) {
        // now close volobjrec we got back
        volobjrec.end = qualval.end;
        volobjrec.size = qualval.size;
        db_end_vol_obj(dbh, &volobjrec);
    } else {
        replace_dynstr(&dbh->errmsg,newstr("#BLIB:  Error closing usage record current volume: %s  requested volume: %s\n", (char *) &volobjrec.label, (char *) &qualval.label));
        dbh->status = ENOTSUP;
        return;
    }
    /* now need to update the bck_objects record
     / update bck_objects set end=<end>, size=<totalsize> where bck_id=<bckid> and objname=<objname>
     / totalsize=select sum(size) total from vol_obj where bck_id=<bckid> and objname=<objname>
     */ 
    totalsize = db_vol_obj_sumsize(dbh,  &volobjrec);
    if (dbcheck(dbh, NULL)) {
        db_update_bck_object_size_end(dbh, volobjrec.bck_id, &volobjrec.objname , volobjrec.obj_instance,  volobjrec.end,  totalsize);
        
        dbcheck(dbh, NULL);
    }
    return;
}

int usetape(dbh_t *dbh, blabel_t *label, bckid_t bck_id )
{
    // volume must either be state='F' or have the same bck_id
    // return 0 if ok to use <0 on failure
    int status;
    
    status = can_use(dbh, label, bck_id);
    if (( status == 'F') || ( status == 'f')) {
	    if (db_setvolume_used(dbh, label, bck_id) < 0) {
            replace_dynstr(&dbh->errmsg, newstr( "#BLIB:  Error changing volume to allocated status %d:%s\n", dbh->status, dbh->errmsg));
            return(-3);
	    }
	    return(0);
    }
    return(-1);
}

int can_use(dbh_t *dbh, blabel_t *label, bckid_t bck_id)
{
    // volume must either be state='F' and have the same bck_id
    // return 0 if ok to use <0 on failure
    vol_t vol;
    int	state;
    
    if (!db_find_volume_bylabel(dbh, label, &vol, FND_EQUAL)) {
        dbh->status = ENOENT;
        replace_dynstr(&dbh->errmsg, newstr( "#BLIB:  Volume \"%s\" :NOT FOUND:\n", (char *) label));
        return(FALSE);
    }
    // we have the volume in vol lets see what we shale see
    state = getstate(&vol.state);
    if ( state == 'F' ) {
        // its free but, if the bck_id is not zero  and not the one they claiming then its not free
        if (vol.bck_id && (vol.bck_id != bck_id)) { // free if bck_id is 0 or equal to current then fail
            return('A');
        }
        return('F');
    } else { 
        // its not free but we may allow them to use it if they claim the correct nonzero bck_id
        if (vol.bck_id) {
            if (vol.bck_id == bck_id) {
                return('f'); // they free to use it, they better not delete it
            } 
        } else {
            return('f'); // bck_id =0 is also free
        }
    }
    return('A');
}


void do_cmd_errbackup(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh)
{
    /* /errbackup   "report an error during backup against /errbackup= /bck_id= /label="},
     blib /bckerror=<objname>  /objinstance= /bck_id=<bckid> /record=<errtime> /label=<label> /desc=<errmsg>
     report an error against this <label> and this <objname> begin backed up
     */
    
    int		reqflg;
    cmd_t	*output_qual;
    vol_obj_t	volobjrec;
    qual_t	qualval;
    bck_errors_t errrec;
    if (BLIB.debug == 9999 ) fprintf(outfd->fd, "debug:9999");
    
    bzero(&qualval, sizeof(qual_t));
    copy_objname(&qualval.objname, (char *) thecmd->val);
    output_qual=qual_ptr;
    reqflg=0;
    qualval.obj_instance = 1; // default to 1
    while(output_qual) {
        switch(output_qual->param->cmdid) {
            case QUAL_BCKID:
                qualval.bck_id = *(bckid_t *) output_qual->val;
                reqflg |= 1;
                break;
            case QUAL_LABEL:
                copy_label(&qualval.label,  (char *) output_qual->val);
                reqflg |= 2;
                break;
            case QUAL_RECORD:
                qualval.recorddate = *(time_t *) output_qual->val;
                reqflg |=4;
                break;
            case QUAL_DESC:
                copy_errmsg(&qualval.errmsg, (char *) output_qual->val);
                reqflg |= 8;
                break;
            case QUAL_OBJINS:
                qualval.obj_instance = *(objid_t *) output_qual->val;
                break;
            default:
                fprintf(stderr, "#BLIB:  Error invalid qualifier %s given to %s\n", output_qual->param->cmdtxt, thecmd->param->cmdtxt);
                return;
                break;
        }
        output_qual=output_qual->next;
    }
    
    if (reqflg != 15 ) {
        fprintf(stderr, "#BLIB:  Error %s requires /bck_id=<bckid> /record=<datetime> /label=<newlabel>   /desc=<errmsg> one of them was missing %s\n", thecmd->param->cmdtxt, thecmd->param->helptxt);
        exit(EINVAL);
    }
    
    bzero(&volobjrec, sizeof(vol_obj_t));
    volobjrec.bck_id = qualval.bck_id;
    copy_objname(&volobjrec.objname, (char *) &qualval.objname);
    copy_label(&volobjrec.label, (char *) &qualval.label);
    if (!db_find_current_volobj(dbh, &volobjrec)) {
        replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Request to report an error but no volume exists for this backup id\n"));
        return;
    }
    bzero(&errrec, sizeof(bck_errors_t));
    errrec.bck_id = qualval.bck_id;
    copy_label(&errrec.label    , (char *) &qualval.label);
    copy_objname(&errrec.objname, (char *) &qualval.objname);
    if (qualval.obj_instance) {
        errrec.obj_instance = qualval.obj_instance;
    } else {
        errrec.obj_instance = 1;
    }
	
    errrec.errtime = qualval.recorddate;
    copy_errmsg(&errrec.errmsg  , (char *) &qualval.errmsg);
    db_insert_bckerror(dbh, &errrec);
	
    dbcheck(dbh, NULL);
    
    return;
    
    
}

void	do_cmd_finishbackup(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh)
{
    // CMD_FINBCK,   	CMD,	"/finishbackup"	    ,NULL	     , VT_STR      , REQVAL_NONE , NULL      ,"finish a backup id (verify and summary updates)"},
    /*
     blib /finishbackup /bck_id=<bckid>  [ /end=<timestr> ] 
     need to update size on each volume  select sum(size) from vol_obj where label=? and bck_id=?
     
     update volumes  set size=(select sum(size) from vol_obj where vol_obj.label = volumes.label and vol_obj.bck_id=?);
     update volumes set duration = (select sum(end-start) from vol_obj where bck_id=volumes.bck_id and label=volumes.label and bck_id=?)
     update volumes set usage=usage+1 where bck_id=?
     update backups set end=<either from /end= or from last completed select max(end) from vol_obj where bck_id=?>
     
     */
    int		reqflg;
    cmd_t	*output_qual;
    qual_t	qualval;
    
    bzero(&qualval, sizeof(qual_t));
    copy_objname(&qualval.objname, (char *) thecmd->val);
    output_qual=qual_ptr;
    reqflg=0;
    while(output_qual) {
        switch(output_qual->param->cmdid) {
            case QUAL_BCKID:
                qualval.bck_id = *(bckid_t *) output_qual->val;
                reqflg |= 1;
                break;
            case QUAL_ENDBCK:
                qualval.end = *(time_t *) output_qual->val;
                reqflg |=0; // optional
                break;
                
            default:
                fprintf(stderr, "#BLIB:  Error invalid qualifier %s given to %s\n", output_qual->param->cmdtxt, thecmd->param->cmdtxt);
                return;
                break;
        }
        output_qual=output_qual->next;
    }
    
    if (reqflg != 1 ) {
        fprintf(stderr, "#BLIB:  Error %s requires /bck_id=<bckid> %s\n", thecmd->param->cmdtxt, thecmd->param->helptxt);
        exit(EINVAL);
    }
    if (qualval.end == 0 ) {
        qualval.end = db_lookup_endofbackup(dbh,qualval.bck_id);
    }
    db_inc_volume_usage(dbh, qualval.bck_id);
    if (!dbcheck(dbh, NULL)) {
        fprintf(outfd->fd, "#BLIB:  Error updating volume sizes in %s: %s\n", thecmd->param->cmdtxt, dbh->errmsg);
        exit(dbh->status);
    }
    
    db_update_backups_end(dbh, qualval.bck_id, qualval.end);
    if (!dbcheck(dbh, NULL)) {
        fprintf(outfd->fd, "#BLIB:  Error updating backup end times in %s: %s\n", thecmd->param->cmdtxt, dbh->errmsg);
        exit(dbh->status);
    }
    
}


void do_cmd_removebackup(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh)
{
    /*
     blib /removebackup=1286360908
     "Remove all data refering to backup id given"},
     blib /removebackup /bck_id=<bckid>
     Check that the backup has expired, it not expired require user input and terminal only
     delete from bck_objects where bck_id=<bckid>;
     delete from vol_obj where bck_id=<bckid>;
     update volumes set bck_id=0 where bck_id=<bckid>;
     
     */
    
    cmd_t	*output_qual;
    qual_t	qualval;
    char	ans[80];
    backups_t	bckrec;
    time_t	nowgmt;
    
    bzero(&qualval, sizeof(qual_t));
    qualval.bck_id = *(bckid_t *) thecmd->val;
    
    output_qual=qual_ptr;
    
    while(output_qual) {
        switch(output_qual->param->cmdid) {
            default:
                fprintf(outfd->fd, "#BLIB:  Error invalid qualifier %s given to %s\n", output_qual->param->cmdtxt, thecmd->param->cmdtxt);
                return;
                break;
        }
        output_qual=output_qual->next;
    }
    
    if (qualval.bck_id == 0 ) {
        fprintf(outfd->fd, "#BLIB:  You cannot delete backup id 0\n");
        exit(EINVAL);
    }
    
    nowgmt = nowgm();
    if (!db_find_backups_by_bck_id(dbh, qualval.bck_id, &bckrec)) {
        replace_dynstr(&dbh->errmsg, newstr("#BLIB:  error backup for id: %llu :NOT FOUND:\n", (llu_t) qualval.bck_id));
        fputs(dbh->errmsg, stderr);
        dbh->status = ENOENT;
        return;
    }
    
    if (bckrec.expiredate >= nowgmt) {
        if (isatty(fileno(stdin))) {	// only if we on a terminal ask
            fprintf(outfd->fd, "#BLIB:  Do you really want to remove bck_id %llu it HAS NOT EXPIRED YET (Y/N)? ", (llu_t) qualval.bck_id );
            fgets(ans,sizeof(ans),stdin);
            switch (*ans) {
                case 'Y':
                    fprintf(outfd->fd, "#BLIB:  I do hope you know what your doing or have a backup of the database\n");
                    break;
                case 'y':
                    fprintf(outfd->fd, "#BLIB:  only upper case Y is acceptable y is not sufficent, exiting\n");
                    exit(EINVAL);
                    break;
                    
                default:
                    fprintf(outfd->fd, "#BLIB:  Ok backup id %llu WILL NOT be removed, nothing done exiting...\n", (llu_t) qualval.bck_id);
                    exit(EINVAL);
                    break;
            }
        } else {
            fprintf(outfd->fd, "#BLIB:  Sorry the remove of an unexpired backup can only be done with a human at a terminal\n");
            exit(ENOTTY);   
        }
    }
    fprintf(outfd->fd, "#BLIB:  Removing backup id %llu\n", (llu_t) qualval.bck_id);
    db_delete_backup_id(dbh, qualval.bck_id);
    return;
}

void do_cmd_modifybackup(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh)
{
    /* 
     blib /modifybackup=<bckid>
     [/record=] [/end=] [/expire=] [/size=] [/desc=]  [/node=]
     */
    cmd_t	*output_qual;
    qual_t	qualval;
    backups_t	bckrec;
    time_t	nowgmt;
    
    bzero(&qualval, sizeof(qual_t));
    
    qualval.bck_id = *(bckid_t *) thecmd->val;
    
    if (qualval.bck_id == 0 ) {
        fprintf(outfd->fd, "#BLIB:  You cannot modiy backup id 0\n");
        exit(EINVAL);
    }
    if (!db_find_backups_by_bck_id(dbh, qualval.bck_id, &bckrec)) {
        replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Error backup for id: %llu :NOT FOUND:\n", (llu_t) qualval.bck_id));
        fputs(dbh->errmsg, stderr);
        dbh->status = ENOENT;
        return;
    }
    nowgmt = nowgm();
    if (bckrec.expiredate >= nowgmt) {
        fprintf(stderr, "#BLIB:  Warning you modifing a NONE EXPIRED backup\n");
    }
    
    output_qual=qual_ptr;
    
    
    while(output_qual) {
        switch(output_qual->param->cmdid) {				
            case QUAL_RECORD:
                bckrec.start = *(time_t *) output_qual->val;
                break;
            case QUAL_ENDBCK:
                bckrec.end = *(time_t *) output_qual->val;
                break;
            case QUAL_EXPIRE:
                bckrec.expiredate = *(time_t *) output_qual->val;
                break;		
            case QUAL_DESC:
                copy_desc(&bckrec.desc, (char *) output_qual->val);
                break;	
            case QUAL_NODE:
                copy_node(&bckrec.node, (char *) output_qual->val);
                break;
            default:
                fprintf(outfd->fd, "#BLIB:  Error invalid qualifier %s given to %s\n", output_qual->param->cmdtxt, thecmd->param->cmdtxt);
                return;
                break;
        }
        output_qual=output_qual->next;
    }
    
    db_update_backups(dbh, &bckrec);
    display_backup(outfd, bckrec.bck_id, CMP_EQ , dbh);
    
    return;
    
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void	do_cmd_listobjects(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh)
{
    // /listobjects[=objname] 
    
    cmd_t	*output_qual;
    qual_t	qualval;
    
    bzero(&qualval, sizeof(qual_t));
    
    
    if (BLIB.debug==99999) fprintf(stderr, "CMD: %s QUAL: %s\n", thecmd->param->cmdtxt, qual_ptr->param->cmdtxt);
    
    output_qual=qual_ptr;
    
    while(output_qual) {
        switch(output_qual->param->cmdid) {	
            default:
                fprintf(outfd->fd, "#BLIB:  Error invalid qualifier %s given to %s\n", output_qual->param->cmdtxt, thecmd->param->cmdtxt);
                return;
                break;
        }
        output_qual=output_qual->next;
    }
    
    copy_objname(&qualval.objname,  (char *) thecmd->val);
    
    display_objects(outfd, &qualval.objname, dbh);
}

void display_objects(fio_t *outfd, objname_t *objname, dbh_t *dbh)
{
    bckobj_t	bckobjrec;
    backups_t	bckrec;
    datestr_t 	start;
    datestr_t 	end;
    datestr_t 	expire;
    int		fndsts;
    char	objnameshrink[18];
    
    /*
     ====================================================================================
     bck_id         objname      ObjInstance    Size   Expire            Descritpion
     1287448004     /                 1            0   26-OCT-2010:21:26:00.00 NEWPROD backup Daily
     Volume:fileno Media     State     Offsite           Location
     ------------- --------  --------- ----------------- --------------------
     AAQ914:2        ULTRIUM5  ALLOC            NotSet NEWPRODTL1
     ====================================================================================
     
     */
    
    
    bzero(&bckrec, sizeof(backups_t));
    
    //    select * from bck_objects where objname > ? order by objname, bck_id desc, obj_instance; 
    if (!BLIB.quiet) fprintf(outfd->fd, "=======================================================================================\n");
    fndsts = db_find_bck_objects_by_name(dbh, objname, &bckobjrec, FND_FIRST);
    while( fndsts) {
        if (!BLIB.quiet) {
            
            fprintf(outfd->fd, "bck_id         objname       ObjInstance Recorded          Expires           Description\n");
        }
        
        if (!db_find_backups_by_bck_id(dbh, bckobjrec.bck_id, &bckrec)) {
            fprintf(stderr, "#BLIB:  Internal database error found bck_object but no matching backups record objname: %s bck_id: %llu\n", objname->str, (llu_t) bckobjrec.bck_id);
            exit(EBADF);
        }
        copy_datestr(&start , (datestr_t *) fmtctime(bckrec.start));
        copy_datestr(&end   , (datestr_t *) fmtctime(bckrec.end));
        copy_datestr(&expire, (datestr_t *) fmtctime(bckrec.expiredate));
        
        fprintf(outfd->fd, "%-13llu  %-21.21s %3u %-17.17s %-17.17s %s\n", bckobjrec.bck_id, 
                shrink_string_by_middle(objnameshrink, 21, bckobjrec.objname.str), bckobjrec.obj_instance, start.str, expire.str, bckrec.desc.str);
        
        display_backup_volumes_for_object(dbh,&bckobjrec, &bckrec, outfd);
        if (!BLIB.quiet) fprintf(outfd->fd, "=======================================================================================\n");
        
        fndsts = db_find_bck_objects_by_name(dbh, objname, &bckobjrec, FND_NEXT);
        if (fndsts && objname->str[0]) {
            if (cmp_objname(objname, &bckobjrec.objname) != 0 ) {
                fndsts = db_find_bck_objects_by_name(dbh, objname, &bckobjrec, FND_FINSIHED); // not end of record set make it so
            }
        }
    }
}

int display_backup_volumes_for_object(dbh_t *dbh, bckobj_t *bckobjrec,backups_t *bckrec , fio_t *outfd)
{
    vol_t	volrec;
    vol_obj_t	volobjrec;
    int		volobj_sts;
    int		vol_sts;
    datestr_t 	librarydate;
    datestr_t 	recorddate;
    datestr_t 	offsitedate;
    datestr_t 	expiredate;
    int		volcount;
    char	volfileno[16];
    
    
    bzero(&volrec,sizeof(volrec));
    if (!BLIB.quiet) {
        fprintf(outfd->fd,"Volume:fileno   Media     State  Offsite           Size             Location\n"                                              
                "-------------   --------  -----  ----------------- ---------------- --------------------\n");   
        /*
         1287386866     /                 2                0   26-OCT-2010:04:27 NEWPROD backup Daily
         Volume:fileno   Media     State Offsite           Location
         -------------   --------  ----- ----------------- --------------------
         AAQ912:48       ULTRIUM5  ALLOC NotSet            NEWPRODTL1
         
         */
    }
    
    copy_datestr(&recorddate,  (datestr_t *) fmtctime(bckrec->start));
    copy_datestr(&expiredate,  (datestr_t *) fmtctime(bckrec->expiredate));
    
    volcount = 0;
    volobj_sts = db_find_vol_obj_for_bck_object(dbh, bckobjrec, &volobjrec, FND_FIRST);
    while (volobj_sts) {
        vol_sts = db_find_volume_for_vol_obj(dbh, &volobjrec, &volrec, FND_EQUAL); // must never fail its an error if it doesnt return true
        if (!vol_sts) {
            if (volrec.bck_id != volobjrec.bck_id) {
                fprintf(stderr, "#BLIB:  Internal database consitency error we found a volume record for the vol_obj yet the bck_id's do not match :%llu  %llu\n", 
                        (llu_t) volrec.bck_id, (llu_t) volobjrec.bck_id);
            } else {
                fprintf(stderr, "#BLIB:  Internal database consitency error we did not find the volume %s\n", volobjrec.label.str);
            }
            exit(EBADF);
        }
        
        
        copy_datestr(&librarydate, (datestr_t *) fmtctime(volrec.librarydate));
        copy_datestr(&offsitedate, (datestr_t *) fmtctime(volrec.offsitedate));
        
        snprintf(volfileno, sizeof(volfileno), "%s:%d", volrec.label.str, volobjrec.fileno);
        
        fprintf(outfd->fd,"%-15s %-9.9s %-5.5s %16llu %-17.17s %-s\n", 
                volfileno,
                (char *) &volrec.media,
                lookup_state((char *) &volrec.state),
                (llu_t) volobjrec.size,
                offsitedate.str, 
                volrec.location.str
                );
        volcount++;
    	volobj_sts = db_find_vol_obj_for_bck_object(dbh, bckobjrec, &volobjrec, FND_NEXT);
    }
    return(volcount);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void	do_cmd_listbackups(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh)
{
    /* /listbackups[=<bckid>]  "list all backups or a given backup id
     */
    
    cmd_t	*output_qual;
    qual_t	qualval;
    
    bzero(&qualval, sizeof(qual_t));
    
    
    if (BLIB.debug==99999) fprintf(stderr, "CMD: %s QUAL: %s\n", thecmd->param->cmdtxt, qual_ptr->param->cmdtxt);
    
    output_qual=qual_ptr;
    
    
    while(output_qual) {
        switch(output_qual->param->cmdid) {	
            default:
                fprintf(outfd->fd, "#BLIB:  Error invalid qualifier %s given to %s\n", output_qual->param->cmdtxt, thecmd->param->cmdtxt);
                return;
                break;
        }
        output_qual=output_qual->next;
    }
    
    if (thecmd->valset == VAL_SET) {
	    qualval.bck_id = *(bckid_t *) thecmd->val;
    }

    display_backup(outfd, qualval.bck_id,thecmd->cmpflg,   dbh); 
} 

void display_backup(fio_t *outfd, bckid_t bckid, cmp_e flag, dbh_t *dbh)
{
    int		dbstatus;
    datestr_t 	start;
    datestr_t 	end;
    datestr_t 	expire;
    backups_t	bckrec;
    find_type_t fndmode;
    bcount_t    rows_read;
    
    if (!BLIB.quiet) {
        fprintf(outfd->fd, "====================================================================================\n");
        fprintf(outfd->fd, "bck_id           Start               End               Expire            Descritpion\n");
    }
    
    bzero(&bckrec, sizeof(backups_t));
    if (bckid) {
        fndmode=FND_EQUAL;
        if (flag != CMP_EQ) {
                fndmode=FND_FIRST;
        }
    } else {
        fndmode=FND_FIRST;
    }
    
    rows_read = 0;
    dbstatus = db_find_backups_orderbckid(dbh, &bckrec, bckid, fndmode);
    while( dbstatus) {
        rows_read++;
        copy_datestr(&start , (datestr_t *) fmtctime(bckrec.start));
        copy_datestr(&end   , (datestr_t *) fmtctime(bckrec.end));
        copy_datestr(&expire, (datestr_t *) fmtctime(bckrec.expiredate));
		
        fprintf(outfd->fd, "%-13llu  %-17.17s   %-17.17s  %-17.17s  %s\n", bckrec.bck_id,
                start.str, end.str, expire.str , bckrec.desc.str);
        if (bckid) { // did they ask for a specific one?
            display_backup_volumes(dbh,&bckrec, outfd);
        }
        
        if (fndmode==FND_FIRST) {
            dbstatus = db_find_backups_orderbckid(dbh, &bckrec, bckid, FND_NEXT);
        } else {
            dbstatus=0;
        }
    }
    if (rows_read == 0 ) {
        fprintf(outfd->fd, "#BLIB: Backup id: %llu :NOTFOUND:\n",bckid);
        dbh->status = ENOENT;
    }
}

int display_backup_volumes(dbh_t *dbh,backups_t *bck_rec, fio_t *outfd)
{
    vol_t	volrec;    
    int		dbrstatus;
    int		filecount=0;
    datestr_t 	librarydate;
    datestr_t 	recorddate;
    datestr_t 	offsitedate;
    datestr_t 	expiredate;
    int		volcount;
    
    
    bzero(&volrec,sizeof(vol_t));
    if (!BLIB.quiet) {
        fprintf(outfd->fd,"-------------------------------------------------------------------------------\n");
        fprintf(outfd->fd,"Volume    #Files  #use  Group     Location   Media     State            Offsite\n"                                              
                "--------- ------ ------ --------- ---------- --------- -----  -----------------\n");   
    }
    
    copy_datestr(&recorddate,  (datestr_t *) fmtctime(bck_rec->start));
    copy_datestr(&expiredate,  (datestr_t *) fmtctime(bck_rec->expiredate));
    
    volcount = 0;
    dbrstatus = db_find_volumes_by_bckid(dbh, bck_rec->bck_id, &volrec, FND_FIRST);
    while (dbrstatus) {
        copy_datestr(&librarydate, (datestr_t *) fmtctime(volrec.librarydate));
        copy_datestr(&offsitedate, (datestr_t *) fmtctime(volrec.offsitedate));
        
        filecount =  db_count_vol_obj_label(dbh, volrec.bck_id, &volrec.label);
        
        fprintf(outfd->fd,"%-9.9s %6d %6d %-9.9s %-10.10s %-9.9s %-5.5s %-17.17s\n", 
                (char *) &volrec.label,
                filecount,
                volrec.usage,
                (char *) &volrec.groupname, 
                (char *) &volrec.location, 
                (char *) &volrec.media, 
                lookup_state((char *) &volrec.state), 
                offsitedate.str
                );
        volcount++;
    	dbrstatus = db_find_volumes_by_bckid(dbh, bck_rec->bck_id, &volrec, FND_NEXT);
    }
    if (!BLIB.quiet) fprintf(outfd->fd, "* End of Report *\n");  
    return(volcount);
}

void dump_vol_obj(vol_obj_t *volobjrec)
{
    fprintf(stderr, "#BLIB:  vol_obj_rec:\n");
    fprintf(stderr, "#BLIB:  	bck_id    : %llu\n", (llu_t)	volobjrec->bck_id);
    fprintf(stderr, "#BLIB:  	objname   : %s\n"  , volobjrec->objname.str);
    fprintf(stderr, "#BLIB:   label     : %s\n"  , volobjrec->label.str);
    fprintf(stderr, "#BLIB:   filio     : %u\n"  , volobjrec->fileno);
    fprintf(stderr, "#BLIB:   start time: %s\n"  , fmtctime(volobjrec->start));
    fprintf(stderr, "#BLIB:   end time  : %s\n"  , fmtctime(volobjrec->end));
    fprintf(stderr, "#BLIB:   size      : %llu\n", (llu_t)  volobjrec->size);
    
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void	do_cmd_verifydb(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh)
{
        
    if (BLIB.debug==99999) fprintf(stderr, "CMD: %s QUAL: %s\n", thecmd->param->cmdtxt, qual_ptr->param->cmdtxt);
    
    
    db_verify(outfd, dbh);
 
}


