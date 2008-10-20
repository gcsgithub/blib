static char *rcsid="@(#) $Id: data_access.c,v 1.1 2008/10/19 22:18:58 root Exp root $";
/*
 *  data_access.c
 *  blib
 *
 *  Created by mark on 08/10/2008.
 *  Copyright 2008 Garetech Computer Solutions. All rights reserved.
 * $Log: data_access.c,v $
 * Revision 1.1  2008/10/19  22:18:58  root
 * Initial revision
 *
 *
 */

#include "blib.h"
#include "util.h"
#include "data_access.h"

extern blib_global_t BLIB;

static char *ver()
{
    return(rcsid);
}

int load_schema(DBH *dbh)
{ // create tables and indexes
    char *errmsg;
    const char *create_volumes_table =  "create table volumes ( "
    "v_label char(13),"
    "v_state char(1), "
    "v_media char(13),"
    "v_usage unsigned int,"
    "v_fileno unsigned int,"
    "v_group varchar(256),"
    "v_location varchar(64),"
    "v_librarydate timestamp,"
    "v_recorddate timestamp,"
    "v_offsitedate timestamp,"
    "v_expiredate timestamp,"
    "v_bytesontape unsigned int(8),"
    "v_desc varchar(256)"
    ")";
    
    const char *create_index_volumes = "create unique index v_pkey_1 on volumes  (v_label);";
    const char *create_events_table =  "create table events ( "
    "e_label char(13),"
    "e_fileno unsigned int,"
    "e_recorddate timestamp,"
    "e_fsetname varchar(256),"
    "e_desc varchar(256),"
    "e_byesinfset unsigned int(8)"
    ")";
    
    const char *create_index_events = "create unique index e_pkey_1 on events  (e_label, e_fileno);";
    
    if (!dbh) {
	fprintf(stderr, "Internal error %s called with null database control block\n", __PRETTY_FUNCTION__);
	return(-1);
    }
    
    dbh->status = sqlite3_exec(dbh->dbf, create_volumes_table, NULL, NULL, &errmsg);
    if (dbh->status != SQLITE_OK) {
	replace_dynstr(&dbh->errmsg ,  newstr(errmsg));
	sqlite3_free(errmsg);
	return(dbh->status);
    }
    
    dbh->status = sqlite3_exec(dbh->dbf, create_index_volumes, NULL, NULL, &errmsg);
    if (dbh->status != SQLITE_OK) {
	replace_dynstr(&dbh->errmsg, newstr(errmsg));
	sqlite3_free(errmsg);
	return(dbh->status);
    }
    
    dbh->status = sqlite3_exec(dbh->dbf,create_events_table, NULL, NULL, &errmsg);
    if (dbh->status != SQLITE_OK) {
	replace_dynstr(&dbh->errmsg, newstr(errmsg));
	sqlite3_free(errmsg);
	return(dbh->status);
    }
    dbh->status = sqlite3_exec(dbh->dbf,create_index_events, NULL, NULL, &errmsg);
    if (dbh->status != SQLITE_OK) {
	replace_dynstr(&dbh->errmsg, newstr(errmsg));
	sqlite3_free(errmsg);
	return(dbh->status);
    }
    
    return(dbh->status);
}

int db_newdb(DBH **dbh, char *fnm)
{   
    int status, err;
    char    ans[80];
    stat_t  db_stat;
    bool_t  doload_schema;
    char    bckfnm[MAXPATHLEN+1];
    
    
    doload_schema = NO;
    
    if (stat(fnm,&db_stat) != -1) {	// make sure we have a file before asking about removal or actually doing it
	if (isatty(fileno(stdin))) {	// only if we on a terminal ask
	    fprintf(stderr, "Do you really want to replace existing database file \"%s\" (Y/N)? ", fnm );
	    fgets(ans,sizeof(ans),stdin);
	    switch (*ans) {
		case 'Y':
		    unlink(bckfnm); // attempt to remove any old back, catch failure on rename
		    snprintf(bckfnm,sizeof(bckfnm), "%s.bck", fnm);
		    if ((status = rename(fnm, bckfnm)) == -1 ) {
			err=errno;
			fprintf(stderr, "Failed to rename \"%s\" to \"%s\" %d:%s\n", fnm, bckfnm,err, strerror(err));
			exit(err);
		    }
		    doload_schema = YES;
		    break;
		    
		default:
		    fprintf(stderr, "Database not removed, nothing done exiting...\n");
		    exit(EINVAL);
		    break;
	    }
	} else {
	    fprintf(stderr, "Sorry /new cannot be used unless your interative on a terminal\n");
	    exit(ENOTTY);   
	}
    }
    
    status = db_open(dbh, fnm);
    return(status);
}

int db_open(DBH **dbh, char *fnm)
{
    DBH	    *rval;
    stat_t  db_stat;
    bool_t  doload_schema;
    int	    err;
    char    *errmsg;
    
    if (dbh == (DBH **) NULL ) {
	fprintf(stderr, "Internal error null pointer passed into %s at line %d\n",__PRETTY_FUNCTION__,__LINE__);
	return(SQLITE_CANTOPEN);
    }
    if (*dbh != (DBH *) NULL ) { // must have an previous open
	rval = *dbh;
	if (rval->open == YES ) db_close(rval);
	nzfree(rval->errmsg);
	nzfree(rval->fnm);
    } else {
	if ((rval = malloc(sizeof(DBH))) == (DBH *) NULL ) {
	    fprintf(stderr, "Error allocating database control block\n");
	    return(SQLITE_CANTOPEN);
	} else {
	    bzero(rval,sizeof (DBH)); // clear out the new block
	}
    }
    
    rval->fnm	      = newstr(fnm);
    rval->errmsg      = newstr("NoError");
    
    doload_schema=NO;			// see if we need to load schema ie created new database file
    if (stat(fnm,&db_stat) == -1) {
	err=errno;
	if (errno == ENOENT) {
	    doload_schema = YES;
	} else {
	    fprintf(stderr, "Error accessing database file \"%s\" %s\n", fnm, strerror(err));
	    return(SQLITE_CANTOPEN);
	}
    }
    
    rval->saved_umask = umask(007);	// allow group rw permission to be set
    rval->status = sqlite3_open(fnm, &rval->dbf);
    if ( rval->status != SQLITE_OK ) {
	err = rval->status;					// save open fail status
	errmsg = newstr((char *) sqlite3_errmsg(rval->dbf));	// save errmsg from open fail
	sqlite3_close(rval->dbf);				// dont care about status, best effort
	// set the errmsg to the  saved open fail
	replace_dynstr(&rval->errmsg ,  errmsg);
	rval->status = err;					// restore saved status of open fail
    } else {
	rval->open = YES;
	rval->status = SQLITE_OK;
    }
    umask(rval->saved_umask); // restore the umask now the database is open
    
    if ((rval->status == SQLITE_OK ) && (doload_schema == YES )) {
	load_schema(rval);
    }
    *dbh = rval; // give it to them
    return(rval->status);
}

int db_close(DBH *dbh)
{
    if (!dbh) {
	fprintf(stderr, "Internal error %s called with null database pointer\n", __PRETTY_FUNCTION__);
	return(-1);
    }
    
    if ( dbh->open == YES ) {
	dbh->open = NO;
	dbh->status = sqlite3_close(dbh->dbf);
	// status 5 aka SQLITE_BUSY if we have prepared statements not released
	dbcheck(dbh,"Error closing database: %d\n", dbh->status);    		    
    } else {
	dbh->status = 0;
	replace_dynstr(&dbh->errmsg,  newstr("Database already closed"));
    }
    if (dbh->status != 0 ) {
	fprintf(stderr, "%s\n", dbh->errmsg);
    }
    return(dbh->status);
}

void	db_finish(DBH *dbh)
{
    if (!dbh) return;
    if (dbh->open == YES ) db_close(dbh);
    nzfree(dbh->fnm);
    nzfree(dbh->errmsg);
    free(dbh);
}



int db_insert_dbrec(DBH *dbh, dbrec_t *rec)
{
    char v_librarydate[64];
    char v_recorddate[64];
    char v_offsitedate[64];
    char v_expiredate[64];
    
    dbh->sqllen =  replace_dynstr(&dbh->sqltxt, newstr("insert into volumes ("
						       "v_label,v_state,v_media,v_usage,v_fileno,v_group,v_location,v_librarydate,"
						       "v_recorddate,v_offsitedate,v_expiredate,v_bytesontape,v_desc"
						       ") values ( ?,?,?,?,?,?,?,?,?,?,?,?,?)"));
    dbh->status = sqlite3_prepare_v2(dbh->dbf, dbh->sqltxt,-1, &dbh->stmt, 0);
    if (!dbcheck(dbh,NULL)) {
	return(dbh->status);
    }
    dbh->status = sqlite3_bind_text(dbh->stmt, 1, rec->v_label, strlen(rec->v_label),SQLITE_TRANSIENT);
    dbh->status = sqlite3_bind_text(dbh->stmt, 2, &rec->v_state, 1,SQLITE_TRANSIENT);
    dbh->status = sqlite3_bind_text(dbh->stmt, 3, rec->v_media, strlen(rec->v_media), SQLITE_TRANSIENT);
    
    
    dbh->status = sqlite3_bind_int( dbh->stmt, 4, rec->v_usage);
    dbh->status = sqlite3_bind_int( dbh->stmt, 5, rec->v_fileno);
    dbh->status = sqlite3_bind_text(dbh->stmt, 6, rec->v_group, strlen(rec->v_group),SQLITE_TRANSIENT);
    dbh->status = sqlite3_bind_text(dbh->stmt, 7, rec->v_location, strlen(rec->v_location),SQLITE_TRANSIENT);
    
    
    strncpy(v_librarydate,  fmtctime(rec->v_librarydate), sizeof(v_librarydate));
    dbh->status = sqlite3_bind_text(dbh->stmt, 8, v_librarydate, strlen(v_librarydate),SQLITE_TRANSIENT);
    
    strncpy(v_recorddate, fmtctime(rec->v_recorddate), sizeof(v_recorddate));
    dbh->status = sqlite3_bind_text(dbh->stmt, 9, v_recorddate, strlen(v_recorddate),SQLITE_TRANSIENT);
    
    strncpy(v_offsitedate, fmtctime(rec->v_offsitedate), sizeof(v_offsitedate));
    dbh->status = sqlite3_bind_text(dbh->stmt, 10, v_offsitedate, strlen(v_offsitedate),SQLITE_TRANSIENT);
    
    strncpy(v_expiredate, fmtctime(rec->v_expiredate), sizeof(v_expiredate));
    dbh->status = sqlite3_bind_text(dbh->stmt, 11, v_expiredate, strlen(v_expiredate),SQLITE_TRANSIENT);
    
    
    dbh->status = sqlite3_bind_int64(dbh->stmt, 12, rec->v_bytesontape); // sqlite3_int64
    
    dbh->status = sqlite3_bind_text(dbh->stmt, 13, rec->v_desc, strlen(rec->v_desc),SQLITE_TRANSIENT);
    
    dbh->status = sqlite3_step(dbh->stmt);
    dbcheck(dbh,NULL);
    dbh->status = sqlite3_finalize(dbh->stmt); // free old query
    dbcheck(dbh,NULL);
    return(dbh->status);
}

int db_find_dbrec(DBH *dbh,dbrec_t *key, dbrec_t *rec, find_type_t flag)
{
    // int cols;
    if (flag == FND_NEXT) {
	//
    } else {
	switch(flag) {
	    case FND_EQUAL:
		// select * from volumes where label = rec->label;
		dbh->sqllen =  replace_dynstr(&dbh->sqltxt, newstr("select * from main.volumes where v_label like ?"));
		break;
	    case FND_FIRST:
		// select * from volumes where label >= rec->label;
		dbh->sqllen =  replace_dynstr(&dbh->sqltxt, newstr("select * from main.volumes where v_label >= ? order by main.v_expiredate"));
		break;
	    default: // shouldnt be possible but
		fprintf(stderr, "Unknown flag value %d passed to %s\n", flag,__PRETTY_FUNCTION__);
		return(-1);
		break;
	}
	dbh->status = sqlite3_prepare_v2(dbh->dbf, dbh->sqltxt,-1, &dbh->stmt, 0);
	if (dbcheck(dbh,NULL) ) {
	    dbh->status = sqlite3_bind_text(dbh->stmt, 1, key->v_label, strlen(key->v_label),SQLITE_TRANSIENT);
	} else {
	    dbh->status = sqlite3_finalize(dbh->stmt); // free old query
	    return(dbh->status);
	}
	
    }
    
    if (dbcheck(dbh,NULL)) {
	dbh->status = sqlite3_step(dbh->stmt);	    // either inital fetch or subsequent next
	if (dbh->status == SQLITE_ROW) {
	    copy_volume_results(dbh,rec);
	}
    }
    
    replace_dynstr(&dbh->errmsg , newstr((char *) sqlite3_errmsg(dbh->dbf)));
    if ((dbh->status == SQLITE_DONE) || ( flag == FND_EQUAL )) {
	sqlite3_finalize(dbh->stmt); // free old query
    }
    
    return(dbh->status);
}

int do_upd(DBH *dbh, char *v_label, cmd_t *cmd )
{
    char datestr[64];
    
    if ((cmd) && (v_label)) {
	dbh->sqllen =  replace_dynstr(&dbh->sqltxt, newstr("update volumes set %s=? where v_label=?", cmd->param->sql_fldnam ));
	dbh->status = sqlite3_prepare_v2(dbh->dbf, dbh->sqltxt,-1, &dbh->stmt, 0);
	if (dbcheck(dbh,NULL))  {
	    switch(cmd->param->val_type) {
		case    VAL_NONE:  // Error
		    fprintf(stderr, "no update value allowed for %s\n", cmd->param->cmdtxt);
		    return(0);
		    break; // not reached
		case    VAL_STR:
		    dbh->status = sqlite3_bind_text(dbh->stmt, 1, cmd->val, -1,SQLITE_TRANSIENT);
		    break;
		case    VAL_STATE:
		    dbh->status = sqlite3_bind_text(dbh->stmt, 1, (char *)&cmd->val, 1,SQLITE_TRANSIENT);
		    break;
		case    VAL_INT:
		    dbh->status = sqlite3_bind_int( dbh->stmt, 1, *(int *) cmd->val);
		    break;
		case	VAL_INT64:
		    dbh->status = sqlite3_bind_int64(dbh->stmt, 1, *(int64_t *) cmd->val); // sqlite3_int64
		    break;
		case    VAL_DATE:
		    strncpy(datestr,  fmtctime(* (time_t *) cmd->val), sizeof(datestr));
		    dbh->status = sqlite3_bind_text(dbh->stmt, 1, datestr, -1,SQLITE_TRANSIENT);
		    break;
	    }
	    
	    dbh->status = sqlite3_bind_text(dbh->stmt, 2, v_label, -1 ,SQLITE_TRANSIENT);
	    if (dbcheck(dbh,NULL)) {
		dbh->status = sqlite3_step(dbh->stmt);	    // do it
		if (dbcheck(dbh,NULL)) {
		    fprintf(stdout, "\"%s\" updated!!\n", cmd->param->sql_fldnam);
		} else {
		    fprintf(stdout, "\"%s\" failed updated :%s!!\n", cmd->param->sql_fldnam, dbh->errmsg);
		}
	    } else {
		fprintf(stdout, "error binding field %s to update statement:%s!!\n",cmd->param->sql_fldnam , dbh->errmsg);
	    }
	} else {
	    fprintf(stderr, "Error preparing update statement %s:%s\n", dbh->sqltxt, dbh->errmsg);
	}
	sqlite3_finalize(dbh->stmt); // free old query
	if (!dbcheck(dbh,NULL)) {
	    fprintf(stderr, "Error updating datbase %s\n", dbh->errmsg);
	}
    }
    return(dbh->status);
}


int db_update_dbrec(DBH *dbh,filt_t *filtrec, dbrec_t *rec)
{
    
    do_upd(dbh,rec->v_label,filtrec->v_label);
    do_upd(dbh,rec->v_label,filtrec->v_state);
    do_upd(dbh,rec->v_label,filtrec->v_media);
    do_upd(dbh,rec->v_label,filtrec->v_group);
    do_upd(dbh,rec->v_label,filtrec->v_location);
    do_upd(dbh,rec->v_label,filtrec->v_desc);
    
    do_upd(dbh,rec->v_label,filtrec->v_usage);
    do_upd(dbh,rec->v_label,filtrec->v_fileno);
    do_upd(dbh,rec->v_label,filtrec->v_recorddate);
    do_upd(dbh,rec->v_label,filtrec->v_offsitedate);
    do_upd(dbh,rec->v_label,filtrec->v_expiredate);
    do_upd(dbh,rec->v_label,filtrec->v_bytesontape);
    
    return(dbh->status);
}

int db_delete_dbrec(DBH *dbh, dbrec_t *key)
{
    dbh->sqllen =  replace_dynstr(&dbh->sqltxt, newstr("delete from main.volumes where v_label=?"));
    dbh->status = sqlite3_prepare_v2(dbh->dbf, dbh->sqltxt,-1, &dbh->stmt, 0);
    if (dbcheck(dbh,NULL) ) {
	dbh->status = sqlite3_bind_text(dbh->stmt, 1, key->v_label, strlen(key->v_label),SQLITE_TRANSIENT);
    } else {
	dbh->status = sqlite3_finalize(dbh->stmt); // free old query
	return(dbh->status);
    }
    
    if (dbcheck(dbh,NULL)) {
	dbh->status = sqlite3_step(dbh->stmt);	    // do it
	if (dbh->status == SQLITE_ROW) {
	    fprintf(stdout, "\"%s\" deleted!!\n", key->v_label);
	}
    }
    
    replace_dynstr(&dbh->errmsg,  newstr((char *) sqlite3_errmsg(dbh->dbf)));
    if (dbh->status == SQLITE_DONE) {
	sqlite3_finalize(dbh->stmt); // free old query
    }
    
    return(dbh->status);
}

dbrec_t	*default_dbrec(dbrec_t *rec)
{
    bzero(rec,sizeof(dbrec_t));
    // set up default values
    rec->v_fileno		=  0;
    rec->v_state		= 'F';
    strncpy(rec->v_media, BLIB.default_media, sizeof(rec->v_media));
    rec->v_usage		= 0;
    strncpy(rec->v_group, BLIB.blib_group, sizeof(rec->v_group));
    strncpy(rec->v_location, "UNKNOWN", sizeof(rec->v_location));
    rec->v_librarydate	= now();
    rec->v_recorddate	= 0;
    rec->v_offsitedate	= 0;
    rec->v_expiredate	= 0;
    strncpy(rec->v_desc, "New volume", sizeof(rec->v_desc));
    
    return(rec);
}


int	db_insert_dbevt(DBH *dbh, dbevt_t *rec)
{
    return(-1);
}

int	db_find_dbevt(DBH *dbh,dbevt_t *key, dbevt_t *rec, find_type_t flag)
{
    return(-1);
}

int	db_update_dbevt(DBH *dbh, dbevt_t *rec)
{
    return(-1);
}

int	db_delete_dbevt(DBH *dbh, dbevt_t *key)
{
    return(-1);
}


int  copy_volume_results(DBH *dbh, dbrec_t *rec)
{
    const unsigned char    *sp;
    
    bzero(rec,sizeof(dbrec_t));
    
    sp = sqlite3_column_text(dbh->stmt, 0);
    if (dbcheck(dbh,"Error retrieving column label")) {
	if (sp) strncpy(rec->v_label, (char *) sp, sizeof(rec->v_label));
	
	sp = sqlite3_column_text(dbh->stmt, 1);
	if (dbcheck(dbh,"Error retrieving column state")) {
	    if (sp) rec->v_state = *sp;
	    
	    sp = sqlite3_column_text(dbh->stmt, 2);
	    if (dbcheck(dbh,"Error retrieving column media")) {
		if (sp) strncpy(rec->v_media, (char *) sp, sizeof(rec->v_media));
		
		rec->v_usage	= sqlite3_column_int(dbh->stmt, 3);
		if (dbcheck(dbh,"Error retrieving column usage")) {
		    rec->v_fileno = sqlite3_column_int(dbh->stmt, 4);
		    if (dbcheck(dbh,"Error retrieving column fileno")) {
			
			sp = sqlite3_column_text(dbh->stmt, 5);
			if (dbcheck(dbh,"Error retrieving column group")) {
			    if (sp) strncpy(rec->v_group, (char *) sp, sizeof(rec->v_group));
			    
			    sp = sqlite3_column_text(dbh->stmt, 6);
			    if (dbcheck(dbh,"Error retrieving column location")) {
				if (sp) strncpy(rec->v_location, (char *) sp, sizeof(rec->v_location));
				
				// len = sqlite3_column_bytes(dbh->stmt, 7);
				// fprintf(stderr, "date column is type %d length %d\n", sqlite3_column_type(dbh->stmt, 7),len);
				// dates are char(23)  dd-mmm-yyyy:HH:mm:ss.cc
				sp = sqlite3_column_text(dbh->stmt, 7);
				if (dbcheck(dbh,"Error retrieving column librarydate")) {
				    if (sp) rec->v_librarydate = scandate((char *) sp);
				    
				    sp = sqlite3_column_text(dbh->stmt, 8);
				    if (dbcheck(dbh,"Error retrieving column recorddate")) {
					if (sp) rec->v_recorddate = scandate((char *) sp); 
					
					sp = sqlite3_column_text(dbh->stmt, 9);
					if (dbcheck(dbh,"Error retrieving column offsitedate")) {
					    if (sp) rec->v_offsitedate = scandate((char *) sp); 
					    
					    sp = sqlite3_column_text(dbh->stmt, 10);
					    if (dbcheck(dbh,"Error retrieving column expiredate")) {
						if (sp) rec->v_expiredate = scandate((char *) sp); 
						
						rec->v_bytesontape = sqlite3_column_int64(dbh->stmt, 11); // sqlite3_int64
						if (dbcheck(dbh,"Error retrieving column bytesontape")) {
						    
						    sp = sqlite3_column_text(dbh->stmt, 12);
						    if (dbcheck(dbh,"Error retrieving column desc")) {
							if (sp) strncpy(rec->v_desc, (char *) sp, sizeof(rec->v_desc));
						    }
						}
					    }
					}
				    }
				}
			    }
			}
		    }
		}
	    }
	}
    }
    return(dbh->status);
}

/*
 const void *sqlite3_column_blob(sqlite3_stmt*, int iCol);
 int sqlite3_column_bytes(sqlite3_stmt*, int iCol);
 int sqlite3_column_bytes16(sqlite3_stmt*, int iCol);
 double sqlite3_column_double(sqlite3_stmt*, int iCol);
 int sqlite3_column_int(sqlite3_stmt*, int iCol);
 sqlite3_int64 sqlite3_column_int64(sqlite3_stmt*, int iCol);
 const unsigned char *sqlite3_column_text(sqlite3_stmt*, int iCol);
 const void *sqlite3_column_text16(sqlite3_stmt*, int iCol);
 int sqlite3_column_type(sqlite3_stmt*, int iCol);
 sqlite3_value *sqlite3_column_value(sqlite3_stmt*, int iCol);
 */


int	dbcheck(DBH *dbh, char *errmsg, ...)
{
    va_list args;
    char    *errstr;
    int	    len;
    
    if (errmsg) {
	va_start(args,errmsg);
	len = VASPRINTF(&errstr, errmsg, args);
	va_end(args);
	if (!errstr) {
	    fprintf(stderr, "Failure in dbcheck to allocate error message\n");
	    exit(ENOMEM);	    // whats the point if we that screwed that we cant allocate a few bytes
	}
    }  else {
	errstr=(char *) NULL;
    }  
    switch(dbh->status) {
	case SQLITE_OK:         //  0    Successful result 
        case SQLITE_ROW:        // 100   sqlite3_step() has another row ready
        case SQLITE_DONE:       // 101   sqlite3_step() has finished executing
	    return(TRUE);
	    break;
	    
        case SQLITE_ERROR:      //  1    SQL error or missing database 
        case SQLITE_INTERNAL:   //  2    Internal logic error in SQLite 
        case SQLITE_PERM:       //  3    Access permission denied 
        case SQLITE_ABORT:      //  4    Callback routine requested an abort 
        case SQLITE_BUSY:       //  5    The database file is locked 
        case SQLITE_LOCKED:     //  6    A table in the database is locked 
        case SQLITE_NOMEM:      //  7    A malloc() failed 
        case SQLITE_READONLY:   //  8    Attempt to write a readonly database 
        case SQLITE_INTERRUPT:  //  9    Operation terminated by sqlite3_interrupt()
        case SQLITE_IOERR:      // 10    Some kind of disk I/O error occurred 
        case SQLITE_CORRUPT:    // 11    The database disk image is malformed 
        case SQLITE_NOTFOUND:   // 12    NOT USED. Table or record not found 
        case SQLITE_FULL:       // 13    Insertion failed because database is full 
        case SQLITE_CANTOPEN:   // 14    Unable to open the database file 
        case SQLITE_PROTOCOL:   // 15    NOT USED. Database lock protocol error 
        case SQLITE_EMPTY:      // 16    Database is empty 
        case SQLITE_SCHEMA:     // 17    The database schema changed 
        case SQLITE_TOOBIG:     // 18    String or BLOB exceeds size limit 
        case SQLITE_CONSTRAINT: // 19    Abort due to constraint violation 
        case SQLITE_MISMATCH:   // 20    Data type mismatch 
        case SQLITE_MISUSE:     // 21    Library used incorrectly 
        case SQLITE_NOLFS:      // 22    Uses OS features not supported on host 
        case SQLITE_AUTH:       // 23    Authorization denied 
        case SQLITE_FORMAT:     // 24    Auxiliary database format error 
        case SQLITE_RANGE:      // 25    2nd parameter to sqlite3_bind out of range 
        case SQLITE_NOTADB:     // 26    File opened that is not a database file 
	    if (errstr) {
		replace_dynstr(&dbh->errmsg ,  newstr("%s: %s", errstr, (char *) sqlite3_errmsg(dbh->dbf)));
	    } else {
		replace_dynstr(&dbh->errmsg ,  newstr((char *) sqlite3_errmsg(dbh->dbf)));
	    }
	    if (errmsg != NULL ) {
		fprintf(stderr, "Error: %s\n", dbh->errmsg);
	    }
	    return(FALSE);
	    break;
	default:
	    replace_dynstr(&dbh->errmsg, newstr("Unknown error return code %d", dbh->status));
	    fprintf(stderr, "%s\n", dbh->errmsg);
	    return(FALSE);
    }
}
