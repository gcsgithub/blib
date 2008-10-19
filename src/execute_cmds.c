static char *rcsid="@(#) $Id:$";
/*
 * $Log:$
 *
 *  execute_cmds.c
 *  blib
 *
 *  Created by mark on 28/09/2008.
 *  Copyright 2008 Garetech Computer Solutions. All rights reserved.
 * 
 */

#include "execute_cmds.h"

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

int execute_cmds(cmd_t *cmds)
{
    cmd_t   *cmdp, *thecmd;
    int	    idx, err;
    dbrec_t db_key, db_read;
    dbevt_t db_key_evt, db_read_evt;
    dbrec_t volrec;
    filt_t  filtrec;
    
    char    *v_state;
    int	    dbstatus, dbrstatus;
    DBH	    *dbh;
    FILE    *iofd;
    char    *fnm;
    char    buf[16384];
    list_t  *lst;
    entry_t *ent;
    char    *sp;
    char v_librarydate[64];
    char v_recorddate[64];
    char v_offsitedate[64];
    char v_expiredate[64];
    
    
    dbh= (DBH *) 0;
    
    if (have_qual(cmds, QUAL_NEW) == YES ) {
	dbstatus = db_newdb(&dbh, BLIB.blibdb_name);
    } else {
	dbstatus = db_open(&dbh, BLIB.blibdb_name);
    }
    if (dbstatus != SQLITE_OK ) {
	if (dbh && dbh->errmsg ) fprintf(stderr, "Error opening database: %s\n",dbh->errmsg); 
	return(dbstatus);   // if we didnt get the database to open the get out now
    }
    default_dbrec(&volrec);
    cmdp = cmds;
    thecmd=cmdp;
    cmdp=thecmd->next; // point to the first qualifier
    switch(thecmd->param->cmdid) {
	case    CMD_DISPLAY:
	case    CMD_MODIFY:
	case    CMD_REMOVE:	// for these pre read the volume
	    strncpy(db_key.v_label,  thecmd->val, sizeof(db_key.v_label));
	    
	    dbstatus = db_find_dbrec(dbh, &db_key, &db_read, FND_EQUAL);
	    if ( dbstatus != SQLITE_ROW ) {
		fprintf(stderr, "Volume \"%s\" not found\n", db_key.v_label);
		if (dbstatus != SQLITE_DONE ) {
		    fprintf(stderr, "Unexpected error: %s\n", dbh->errmsg);
		}
		db_close(dbh);
		return(dbstatus);
	    } else {
		// load the values from the fetched record
		memcpy(&volrec,  &db_read, sizeof(volrec));
		
		switch(thecmd->param->cmdid) {
		    case    CMD_DISPLAY:
			if (cmdp) {
			    fprintf(stderr, "Extra qualifiers to %s ignored\n", thecmd->param->cmdtxt);
			}
			display_volume( BLIB.quiet, &volrec);
			break;
			
		    case    CMD_MODIFY:
			if ((err = modify_rec(&filtrec, &volrec, cmdp)) !=0 ) { // update any values we have been given
			    db_close(dbh);
			    return(err);
			}
			strncpy(volrec.v_label,  thecmd->val, sizeof(db_key.v_label));
			dbstatus = db_update_dbrec(dbh, &filtrec, &volrec);
			if (dbcheck(dbh,NULL)) {
				fprintf(stderr, "\"%s\" modified\n", volrec.v_label);				    
			} else {
				fprintf(stderr, "Error updating volume %s %s\n", volrec.v_label, dbh->errmsg);
				db_close(dbh);
				
				return(err);				
			}
			break;
			
		    case    CMD_REMOVE:
			if (cmdp) {
			    fprintf(stderr, "Extra qualifiers to %s ignored\n", thecmd->param->cmdtxt);
			}
			dbstatus = db_delete_dbrec(dbh, &db_key);
			switch(dbstatus) {
			    case SQLITE_OK:
			    case SQLITE_DONE:
				//TODO: need to delete all event records at the same time
				fprintf(stderr, "%s deleted\n", db_key.v_label);
				memcpy(db_key_evt.e_label, db_key.v_label, sizeof(db_key_evt.e_label));
				dbrstatus = db_find_dbevt(dbh, &db_key_evt, &db_read_evt, FND_FIRST);
				while (dbrstatus == SQLITE_ROW ) {
				    dbstatus = db_delete_dbevt(dbh,&db_read_evt);
				    if (dbstatus == -1 ) {
					err = errno;
					fprintf(stderr, "Error deleting volume backuprecord %s %s\n", volrec.v_label, dbh->errmsg);				    
				    }
				    dbrstatus = db_find_dbevt(dbh, &db_key_evt, &db_read_evt, FND_NEXT);
				}
				break;
			    default:
				fprintf(stderr, "Error removing tape label \"%s\" %s\n", volrec.v_label, dbh->errmsg);
				break;
			}
			break;
		}
	    }
	    break;
	    
	case    CMD_ADD:
	    strncpy(db_key.v_label,  thecmd->val, sizeof(db_key.v_label));
	    strncpy(volrec.v_label,  thecmd->val, sizeof(db_key.v_label));
	    
	    dbstatus = db_find_dbrec(dbh, &db_key, &db_read, FND_EQUAL);
	    if ( dbstatus != SQLITE_ROW ) { // not found so ok to add
		if ((err = modify_rec(&filtrec, &volrec, cmdp)) !=0 ) { // update any values we have been given
		    db_close(dbh);    		    
		    return(err);
		}		
		dbstatus = db_insert_dbrec(dbh,&volrec);
		switch(dbstatus) {
		    case SQLITE_OK:
			if (BLIB.debug) fprintf(stderr, "added tape label \"%s\"\n", volrec.v_label);
			break;
		    case SQLITE_CONSTRAINT:
			fprintf(stderr, "Duplicate key volume label \"%s\"\n", volrec.v_label);
			break;
		    default:
			fprintf(stderr, "Error storing tape label \"%s\" %s\n", volrec.v_label, dbh->errmsg);
			break;
		}
	    } else {
		fprintf(stderr, "Volume \"%s\" already in database you cannot add it again\n", volrec.v_label);
		db_close(dbh);	    		
		return(EEXIST);
	    }
	    break;
	    
	case    CMD_REPORT:
	    bzero(&volrec, sizeof(volrec)); // clear all defaults we only want those filter values
	    if ((err = modify_rec(&filtrec, &volrec, cmdp)) !=0 ) { // update the volrec from cmd qualifiers use as filter
		return(err);
	    }
	    bzero(&db_key,sizeof(db_key));
	    bzero(&db_read,sizeof(db_read));
	    if (!BLIB.quiet) {
		fprintf(stdout,"Volume         Fileno  Group    Location     Media    State      Expires           Offsite           Record             Description\n");                                              
		fprintf(stdout,"-------------  ------  -----    --------     -----    -----      ----------------- ----------------- -----------------  ----------------\n");                   
	    }
	    dbrstatus = db_find_dbrec(dbh, &db_key, &db_read, FND_FIRST);
	    while (dbrstatus == SQLITE_ROW ) {
		if (filter_rec(&filtrec,&db_read)) {
		    switch(db_read.v_state) {
			case 'A':
			    v_state="ALLOCATED";
			    break;
			case 'F':
			    v_state="FREE";
			    break;
		    }
                    strncpy(v_librarydate,  fmtctime(db_read.v_librarydate), sizeof(v_librarydate));
                    strncpy(v_recorddate, fmtctime(db_read.v_recorddate), sizeof(v_recorddate));
                    strncpy(v_offsitedate, fmtctime(db_read.v_offsitedate), sizeof(v_offsitedate));
                    strncpy(v_expiredate, fmtctime(db_read.v_expiredate), sizeof(v_expiredate));
		    
		    fprintf(stdout,"%-13.13s %7d  %-8.8s %-12.12s %-8.8s %-10.10s %-17.17s %-17.17s %-17.17s  %s\n", db_read.v_label,
			    db_read.v_fileno, db_read.v_group, db_read.v_location, db_read.v_media, v_state, 
			    v_expiredate, v_offsitedate, v_recorddate, db_read.v_desc);
		}
		dbrstatus = db_find_dbrec(dbh, &db_read, &db_read, FND_NEXT);
	    }
	    break;
	    
	case CMD_EXPORT:
	    fnm = thecmd->val;
	    if (fnm && *fnm) {
		if ((iofd = fopen(fnm,"w")) == (FILE *) NULL  ) {
		    err=errno;
		    fprintf(stderr, "Error opening export file \"%s\", %s\n", fnm,dbh->errmsg);
		    db_close(dbh);
		    return(err);
		}
	    } else {
		iofd = stdout;
	    }
	    bzero(&volrec, sizeof(volrec)); // clear all defaults we only want those filter values
	    if ((err = modify_rec(&filtrec, &volrec, cmdp)) !=0 ) { // update the volrec from cmd qualifiers use as filter
		return(err);
	    }
	    
	    dbrstatus = db_find_dbrec(dbh, &db_key, &db_read, FND_FIRST);
	    while (dbrstatus == SQLITE_ROW ) {
		if (filter_rec(&filtrec,&db_read)) {
		    switch(db_read.v_state) {
			case 'A':
			    v_state="ALLOCATED";
			    break;
			case 'F':
			    v_state="FREE";
			    break;
		    }
		    // ${BLIB_VOLUME}|${BLIB_FILENO}|${BLIB_STATE}|${BLIB_MEDIA}|${BLIB_USAGE}|${BLIB_GROUP}|${BLIB_LOCATION}|
		    //    %s         |   %d         | %s          | %s          |   %s        | %s          | %s
		    // ${BLIB_LIBRARYDATE}|${BLIB_RECORDDATE}|${BLIB_OFFSITEDATE}|${BLIB_EXPIREDATE}|${BLIB_DESC}
		    //    %s              |  %s              | %s                | %s               | %s
		    
		    strncpy(v_librarydate,  fmtctime(db_read.v_librarydate), sizeof(v_librarydate));
		    strncpy(v_recorddate, fmtctime(db_read.v_recorddate), sizeof(v_recorddate));
		    strncpy(v_offsitedate, fmtctime(db_read.v_offsitedate), sizeof(v_offsitedate));
		    strncpy(v_expiredate, fmtctime(db_read.v_expiredate), sizeof(v_expiredate));
		    fprintf(iofd,"%s|%d|%s|%s|%d|%s|%s|%s|%s|%s|%s|%s\n", 
			    db_read.v_label,
			    db_read.v_fileno,
			    v_state,
			    db_read.v_media,
			    db_read.v_usage, 
			    db_read.v_group, 
			    db_read.v_location,
			    v_librarydate,
			    v_recorddate,
			    v_offsitedate,
			    v_expiredate,
			    db_read.v_desc);
		}
		dbrstatus = db_find_dbrec(dbh, &db_key, &db_read, FND_NEXT);
	    }
	    if (iofd != stdout) fclose(iofd);
	    break;
	    
	case CMD_IMPORT:
	    fnm = thecmd->val;
	    if (fnm && *fnm) {
		if ((iofd = fopen(fnm,"r")) == (FILE *) NULL  ) {
		    err=errno;
		    fprintf(stderr, "Error opening export file \"%s\", %s\n", fnm,dbh->errmsg);
		    db_close(dbh);
		    return(err);
		}
	    } else {
		iofd=stdin;
	    }
	    
	    fgets(buf, sizeof(buf),iofd);
	    while (!feof(iofd)) {
		lst = 0;
		split(buf,"|", &lst);
		if (lst) {
		    if (BLIB.debug) dump_list(lst);
		    ent=lst->head;
		    ent = snprintf_ent(volrec.v_label, sizeof(volrec.v_label), "%s",  ent, "Missing Volume label\n");
		    ent = atoi_ent(&volrec.v_fileno, ent, "Missing fileno\n");
		    
		    if (ent && ent->e ) {
			sp = (char *) ent->e;
			if      (strcmp(sp, "ALLOCATED" ) == 0)  volrec.v_state = 'A';
			else if (strcmp(sp, "FREE"      ) == 0)  volrec.v_state = 'F';
			else				     volrec.v_state = 'X';
			ent = ent->np;
		    } else {
			fprintf(stderr, "Missing state\n");
		    }    
		    
		    ent = snprintf_ent(volrec.v_media, sizeof(volrec.v_media),"%s", ent, "Missing media\n");
		    ent = atoi_ent(&volrec.v_usage , ent, "Missing usage\n");
		    ent = snprintf_ent(volrec.v_group, sizeof(volrec.v_group),"%s", ent, "Missing group\n");
		    ent = snprintf_ent(volrec.v_location, sizeof(volrec.v_location),"%s", ent, "Missing location\n");
		    
		    ent = time_ent(&volrec.v_librarydate, ent, "Missing Library date\n");
		    ent = time_ent(&volrec.v_recorddate, ent, "Missing record date\n");
		    ent = time_ent(&volrec.v_offsitedate, ent, "Missing offsite date\n");
		    ent = time_ent(&volrec.v_expiredate, ent, "Missing expire date\n");
		    
		    ent = snprintf_ent(volrec.v_desc, sizeof(volrec.v_desc),"%s", ent, "Missing description\n");
		    
		    dbstatus = db_insert_dbrec(dbh, &volrec);
		    switch(dbstatus) {
			case SQLITE_OK:
			    if (BLIB.debug) fprintf(stderr, "Loaded tape label \"%s\"\n", volrec.v_label);
			    break;
			case SQLITE_CONSTRAINT:
			    fprintf(stderr, "Duplicate key volume label \"%s\"\n", volrec.v_label);
			    break;
			default:
			    fprintf(stderr, "Error storing tape label \"%s\" %s\n", volrec.v_label, dbh->errmsg);
			    break;
		    }
		    free_list(&lst, freeent);
		} else {
		    fprintf(stderr, "Error spliting command\n");
		}
		fgets(buf, sizeof(buf),iofd);
	    }
	    if (iofd != stdin) fclose(iofd);
	    
	    break;
	default:
	    fprintf(stderr, "Error invalid initial command %s, initial command must be one of :-\n", thecmd->param->cmdtxt);
	    idx=CMD_ERR+1;
	    while(CMDQUALS[idx].cmdid != CMD_END) {
		if (CMDQUALS[idx].cmdtype == CMD ) fprintf(stderr, "\t\t%s\n", CMDQUALS[idx].cmdtxt);
		idx++;
	    }
	    break;
    }
    return(db_close(dbh));
}    

int   modify_rec(filt_t  *filtrec, dbrec_t *volrec, cmd_t *cmdp)
{
    int rval;
    
    rval = 0; // no error
    bzero(filtrec, sizeof(filt_t));
    
    while(cmdp) {
	switch(cmdp->param->cmdid) {
	    case QUAL_MEDIA:
		filtrec->v_media = cmdp;
		strncpy(volrec->v_media	, cmdp->val, sizeof(volrec->v_media));
		break;
	    case QUAL_GROUP:
		filtrec->v_group = cmdp;
		strncpy(volrec->v_group	, cmdp->val, sizeof(volrec->v_group));
		break;
	    case QUAL_LOCATION:
		filtrec->v_location = cmdp;
		strncpy(volrec->v_location, cmdp->val, sizeof(volrec->v_location));
		break;
	    case QUAL_INCFILENO:
		filtrec->v_fileno = cmdp;
		volrec->v_fileno++;
		replace_dynstr((char **) &cmdp->val,(char *) newint(volrec->v_fileno));
		break;
	    case QUAL_STATE:
		volrec->v_state = *(char *) &cmdp->val;
		filtrec->v_state = cmdp;
		break;
	    case QUAL_RECORD:
		filtrec->v_recorddate = cmdp;
		volrec->v_recorddate  = *(time_t *) cmdp->val;
		break;
	    case QUAL_EXPIRE:
		filtrec->v_expiredate = cmdp;
		volrec->v_expiredate  = *(time_t *) cmdp->val;
		break;
	    case QUAL_OFFSITE:
		filtrec->v_offsitedate = cmdp;
		volrec->v_offsitedate = *(time_t *) cmdp->val;
		break;
	    case QUAL_USAGE:
		filtrec->v_usage = cmdp;
		volrec->v_usage++;
		replace_dynstr((char **) &cmdp->val,(char *) newint(volrec->v_usage));
		break;
	    case QUAL_FILENO:
		filtrec->v_fileno = cmdp;
		volrec->v_fileno = *(int *) cmdp->val;
		break;
	    case QUAL_DESC:
		filtrec->v_desc = cmdp;
		strncpy(volrec->v_desc, cmdp->val, sizeof(volrec->v_desc));
		break;
	    case QUAL_ADDFSET:
		fprintf(stderr, "Not yet implmented\n");
		rval = EINVAL;
		break;
	}
	cmdp = cmdp->next;
    }
    return(rval);
}

char  *doenv(int flag, char *symbol, valtype_t val_type,void *value)
{
    static char    buf[2048];
    char    *sp;
    char    val[2048];
    int	    rlen, len, idx;
    
    switch(val_type) {
	case VAL_NONE:
	    snprintf(val,sizeof(val), "");
	    break;
	case    VAL_STR:
	    snprintf(val,sizeof(val), "\047%s\047", value);
	    break;
	case    VAL_STATE:
	    switch(*(char *)value) {
		case    'F':
		    snprintf(val,sizeof(val), "%s", "FREE");
		    break;
		case    'A':
		    snprintf(val,sizeof(val), "%s", "ALLOCATED");
		    break;
		default:
		    snprintf(val,sizeof(val), "%s", "UNKNOWN");
		    break;
	    }    
	    break;	    
	case    VAL_INT:
	    snprintf(val,sizeof(val), "%d", *( int *) value);
	    break;
	case    VAL_INT64:
	    snprintf(val,sizeof(val), "%llu", *( uint64_t *) value);
	    break;
	case VAL_DATE:
	    snprintf(val,sizeof(val) ,"%s", fmtctime(*(time_t *) value));
	    break;
	    
    }
    
    rlen=sizeof(buf);
    
    if (!flag) {
	snprintf(buf,rlen, "%s=", symbol);
	len = strlen(buf);
	sp = buf+len;
	rlen -= len;
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
	    fprintf(stdout, "export %s;\n", buf);
	}
    }
    return(buf);
}

int display_volume(int flag , dbrec_t *volrec)
{
    char    *v_state;
    
    doenv(flag, "BLIB_VOLUME", VAL_STR, volrec->v_label);
    
    switch(volrec->v_state) {
	case 'A':
	    v_state="ALLOCATED";
	    break;
	case 'F':
	    v_state="FREE";
	    break;
	default:
	    v_state="UNKNOWN";
	    break;
    }
    doenv(flag, "BLIB_STATE",	VAL_STR, v_state);
    doenv(flag, "BLIB_MEDIA",	VAL_STR, volrec->v_media);
    doenv(flag, "BLIB_USAGE",	VAL_INT, &volrec->v_usage);
    doenv(flag, "BLIB_FILENO",	VAL_INT, &volrec->v_fileno);
    doenv(flag, "BLIB_GROUP",  VAL_STR,	volrec->v_group);
    doenv(flag, "BLIB_LOCATION", VAL_STR, volrec->v_location);
    doenv(flag, "BLIB_LIBRARYDATE",VAL_STR, fmtctime(volrec->v_librarydate));
    doenv(flag, "BLIB_RECORDDATE", VAL_STR,  fmtctime(volrec->v_recorddate));
    doenv(flag, "BLIB_OFFSITEDATE",VAL_STR,  fmtctime(volrec->v_offsitedate));
    doenv(flag, "BLIB_EXPIREDATE",VAL_STR, fmtctime(volrec->v_expiredate));
    doenv(flag, "BLIB_BYTESONTAPE", VAL_INT64, &volrec->v_bytesontape);
    doenv(flag, "BLIB_DESC", VAL_STR, volrec->v_desc);
    fprintf(stdout, "\n");
    
    return(0);
}

int filtcmp(cmd_t *qual, void *val, int size)
{
    int rval;
    rval = 0;	 // display
    
    if (!qual) return(0);    // when it doubt put it out
    
    switch (qual->param->val_type) {
	case       VAL_NONE:
	    rval = 0;		// print all
	    break;
	case    VAL_STR:
	    rval = strncasecmp((char *) qual->val,(char *) val, size);
	    break;
	case VAL_STATE:
	    rval = (*(char *) &qual->val - *(char *)val);
	    break;
	case    VAL_INT:
	    rval = ( *(int *)val - *(int *) qual->val);
	    break;
	case VAL_INT64:
	    if (*(uint64_t *) qual->val == *(uint64_t *)val)	 rval=0;
	    else if (*(uint64_t *) qual->val > *(uint64_t *)val) rval=-1;
	    else						 rval=1;
	    break;
	case    VAL_DATE:
	    rval = ( *(time_t *)val - *(time_t *)qual->val);
	    break;
    }
    
    switch(qual->cmpflg) // rval =1 will skip record in filter_rec
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

int filter_rec(filt_t *filtrec, dbrec_t *rec)
{ // return true to print record  0 false to skip it ie default 1 to skip
    
    if (filtcmp(filtrec->v_label      , rec->v_label       , sizeof(rec->v_label)))	    return(0);	// mismatched
    if (filtcmp(filtrec->v_state      , &rec->v_state      , sizeof(rec->v_state)))	    return(0);	// mismatched
    if (filtcmp(filtrec->v_media      , rec->v_media       , sizeof(rec->v_media)))	    return(0);	// mismatched
    if (filtcmp(filtrec->v_group      , rec->v_group       , sizeof(rec->v_group)))	    return(0);	// mismatched
    if (filtcmp(filtrec->v_location   , rec->v_location    , sizeof(rec->v_location)))	    return(0);	// mismatched
    if (filtcmp(filtrec->v_desc       , rec->v_desc        , sizeof(rec->v_desc)))	    return(0);	// mismatched
    
    if (filtcmp(filtrec->v_usage      ,(int *)  &rec->v_usage      , sizeof(rec->v_usage)))	  return(0);	// mismatched
    if (filtcmp(filtrec->v_fileno     ,(int *)  &rec->v_fileno     , sizeof(rec->v_fileno)))	  return(0);	// mismatched
    if (filtcmp(filtrec->v_recorddate ,(time_t *)    &rec->v_recorddate , sizeof(rec->v_recorddate)))     return(0);	// mismatched
    if (filtcmp(filtrec->v_offsitedate,(time_t *)    &rec->v_offsitedate, sizeof(rec->v_offsitedate)))    return(0);	// mismatched
    if (filtcmp(filtrec->v_expiredate ,(time_t *)    &rec->v_expiredate , sizeof(rec->v_expiredate)))     return(0);	// mismatched
    if (filtcmp(filtrec->v_bytesontape,(uint64_t  *) &rec->v_bytesontape, sizeof(rec->v_bytesontape)))    return(0);	// mismatched
    
    
    return(1); // default to display record
    
}
