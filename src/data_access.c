static const char *rcsid="@(#) $Id: data_access.c,v 1.10 2011/04/15 03:22:57 mark Exp mark $";
/*
 *  data_access.c
 *  blib
 *
 *  Created by mark on 08/10/2008.
 *  Copyright 2008 Garetech Computer Solutions. All rights reserved.
 * $Log: data_access.c,v $
 * Revision 1.10  2011/04/15 03:22:57  mark
 * add db_count_bck_errors_bck_id so finishbackup can report errors
 *
 * Revision 1.9  2011/04/14 02:28:06  mark
 * fix for missing obj_instance on bck_errors
 *
 * Revision 1.8  2011/04/13 03:59:18  mark
 * fix bug in bck_errors tables, wasnt using obj_instance to look it up and wasnt inserting the obj_instance
 *
 * Revision 1.7  2011/04/12 00:35:26  mark
 * tidyup verify code
 *
 * Revision 1.6  2011/04/11 03:51:47  mark
 * generally fix OSrval's, fix records being added with invalid bck_id, add /verify
 *
 * Revision 1.5  2010/11/26 00:56:48  mark
 * fix incorrect fld order on update backups node, start where the wrong way around ;(
 *
 * Revision 1.4  2010/11/24 00:58:47  mark
 * BUGFIX: reportexpire reportfree where very wrong based on older database structures
 *
 * Revision 1.3  2010/11/16 04:10:15  mark
 * rc1
 *
 * Revision 1.2  2008/10/20  13:05:22  root
 * checkpoint
 *
 * Revision 1.1  2008/10/19  22:18:58  root
 * Initial revision
 *
 *
 */

#include "blib.h"
#include "util.h"
#include "data_access.h"
#include "copycheck.h"
#include "timefunc.h"


extern blib_global_t BLIB;

static const char *version()
{
    return(rcsid);
}

int load_schema(dbh_t *dbh)
{ // create tables and indexes
    
    char *create_table_backups =
    "create table backups ( "
    "bck_id		unsigned long long,"
    "node		varchar("NODE_SIZ_STR"),"
    "start		unsigned int,"
    "end		unsigned int,"
    "expiredate		unsigned int,"
    "desc		varchar("DESC_SIZ_STR")"
    ")";
    char *create_index_backups_1 = "create unique index b_pkey_1 on backups  (bck_id);";
    
    char *create_table_volumes =  
    "create table volumes ( "
    "bck_id		unsigned long long,"
    "label		char(" LABEL_SIZ_STR "),"
    "state		char(" STATE_SIZ_STR "), "
    "media		char(" MEDIA_NAM_SIZ_STR "),"
    "usage		unsigned int,"
    "groupname		varchar(" GROUPNAM_SIZ_STR "),"
    "location		varchar(" LOCNAM_SIZ_STR "),"
    "librarydate	date,"
    "recorddate		date,"
    "offsitedate	date"
    ")";
    char *create_index_volumes_1 = "create unique index labelpkey_1 on volumes  (label);";
    /****************************************************************************************/
    
    char *create_table_bck_objects =  
    "create table bck_objects ( "
    "bck_id		unsigned long long,"
    "objname	  	varchar("OBJ_NAME_SIZ_STR "),"
    "obj_instance	unsigned int,"		// muliple objects copies of an object backed
    "start		date,"
    "end		date,"
    "size		unsigned long long"
    ")";
    
    char *create_index_bck_objects_1 = "create unique index bo_pkey_1 on bck_objects  (objname, obj_instance, bck_id);";
    /****************************************************************************************/
    
    char *create_table_vol_obj =  
    "create table vol_obj ( "
    "bck_id		unsigned long long,"
    "objname		varchar(" OBJ_NAME_SIZ_STR "),"
    "obj_instance	unsigned int, " // multiple objects backed up with the same name
    "label		varchar(" LABEL_SIZ_STR "),"
    "fileno 		unsigned int,"
    "start		date,"
    "end		date,"
    "size		unsigned long long"
    ")";
    
    char *create_index_vol_obj_1 = "create index vo_pkey_1 on vol_obj  (bck_id,objname, fileno);";
    char *create_index_vol_obj_2 = "create index vo_pkey_2 on vol_obj  (bck_id,label, fileno);";
    char *create_index_vol_obj_3 = "create unique index vo_pkey_3 on vol_obj  (bck_id,objname,obj_instance,label);";
    
    
    char *create_table_bck_errors = 
    "create table bck_errors ("
    "bck_id		unsigned long long,"
    "label		varchar(" LABEL_SIZ_STR "),"
    "objname		varchar(" OBJ_NAME_SIZ_STR ")," 
    "obj_instance	unsigned int, "
    "errtime		date,"
    "errmsg		varchar(" ERRMSG_SIZ_STR ")"
    ")";
    
    char *create_index_bck_errors_1 = "create index be_pkey_1 on bck_errors  (bck_id,label,objname);";
    
    if (!dbh) {
        fprintf(stderr, "#BLIB:  Internal error %s called with null database control block\n", __PRETTY_FUNCTION__);
        return(-1);
    }
    
    if (db_exec_sql(dbh, create_table_backups) 	  	!= BLIBDB_OK)  return(dbh->status);
    if (db_exec_sql(dbh, create_index_backups_1)	!= BLIBDB_OK)  return(dbh->status);
    if (db_exec_sql(dbh, create_table_volumes)		!= BLIBDB_OK)  return(dbh->status);
    if (db_exec_sql(dbh, create_index_volumes_1)	!= BLIBDB_OK)  return(dbh->status);
    if (db_exec_sql(dbh, create_table_bck_objects)	!= BLIBDB_OK)  return(dbh->status);
    if (db_exec_sql(dbh, create_index_bck_objects_1)!= BLIBDB_OK)  return(dbh->status);
    if (db_exec_sql(dbh, create_table_vol_obj)		!= BLIBDB_OK)  return(dbh->status);
    if (db_exec_sql(dbh, create_index_vol_obj_1)	!= BLIBDB_OK)  return(dbh->status);
    if (db_exec_sql(dbh, create_index_vol_obj_2)	!= BLIBDB_OK)  return(dbh->status);
    if (db_exec_sql(dbh, create_index_vol_obj_3)	!= BLIBDB_OK)  return(dbh->status);
    if (db_exec_sql(dbh, create_table_bck_errors)	!= BLIBDB_OK)  return(dbh->status);
    if (db_exec_sql(dbh, create_index_bck_errors_1)	!= BLIBDB_OK)  return(dbh->status);
    
    return(dbh->status);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int db_exec_sql_bckid(dbh_t *dbh, char *sqltext, bckid_t bck_id)
{
    int rval;
    list_t	*flds = (list_t *) NULL;
    
    db_fldsmklist(&flds, "bckid"  , FLD_INT64, (void *) &bck_id);
    
    rval = db_exec_sql_flds_pushpop(dbh, sqltext, flds);
    
    db_fldsfreelist(&flds);
    return(rval);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
int db_exec_sql_flds_push(dbh_t *dbh, char *sqltext, list_t *flds)
{
    int rval;
    sqlstack_push(dbh);
    rval = db_exec_sql_flds(dbh, sqltext, flds);
    return(rval);
}

int db_exec_sql_flds_pop(dbh_t *dbh, char *sqltext, list_t *flds)
{
    int rval;
    
    rval = db_exec_sql_flds(dbh, sqltext, flds);
    
    sqlstack_pop(dbh);
    return(rval);
}

int db_exec_sql_flds_pushpop(dbh_t *dbh, char *sqltext, list_t *flds)
{
    int rval;
    sqlstack_push(dbh);
    rval = db_exec_sql_flds_pop(dbh, sqltext, flds);
    return(rval);
}

int db_exec_sql_flds(dbh_t *dbh, char *sqltext, list_t *flds)
{
    int rval;
    
    if (dbh && dbh->sqlcmd ) {
        dbh->sqlcmd->putflds = flds;
        replace_dynstr(&dbh->sqlcmd->sqltxt, newstr(sqltext));
        
        if ((rval = db_prepare_sql(dbh, dbh->sqlcmd, sqltext)))  {
            dbh->status = sqlite3_step(dbh->sqlcmd->stmt);	    // do it
            rval = dbcheck(dbh,NULL);
            if (rval) {
                replace_dynstr(&dbh->errmsg, newstr("not an error"));
            } else {
                replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Error stepping sql statement %s\n", sqltext, (char *) dbh->errmsg));
            }
        } else {
            replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Error preparing sql statement: %s: %s\n", sqltext, (char *) dbh->errmsg));
        }
    } else  {
        fprintf(stderr, "#BLIB:  Internal error in %s either database handle or sqlcmd was null\n", __PRETTY_FUNCTION__);
        exit(EINVAL);
    }
    return(rval);
}

//////////////////////////////////////////////////////////////////////    
int db_finalize_stmt(dbh_t *dbh)
{
    int rval=0;
    if (dbh->sqlcmd && dbh->sqlcmd->stmt) {
        if (dbh->sqlcmd->needfinalized) {
            rval=1;
            sqlite3_finalize(dbh->sqlcmd->stmt); // free old query
            dbh->sqlcmd->needfinalized=0;
        }
    }
    return(rval);
}
/////////////////////////////////////////////////
int db_exec_sql(dbh_t *dbh,  char *sqltext)
{
    char *errmsg;
    dbh->status = sqlite3_exec(dbh->dbf,sqltext, NULL, NULL, &errmsg);
    if (dbh->status != BLIBDB_OK) {
        replace_dynstr(&dbh->errmsg, newstr(errmsg));
        sqlite3_free(errmsg);
    }
    return(dbh->status);
}

////////////////////////////////////////////////////////////////////////
int db_prepare_sql(dbh_t *dbh, sqlcmd_t *sqlcmd, char *sqltextfmt, ...)
{
    va_list args;
    int sts;
    char    *sqltext;
    size_t  len;
    
    sts=0;
    if (sqltextfmt) {
        va_start(args,sqltextfmt);
        len = VASPRINTF(&sqltext, sqltextfmt, args);
        va_end(args);
        if (!sqltextfmt) {
            fprintf(stderr, "Failure in " __PRETTY_FUNCTION__ "to allocate error message %llu bytes\n", (llu_t) len);
            exit(ENOMEM);	    // whats the point if we that screwed that we cant allocate a few bytes
        }    
        sqlcmd->sqllen =  replace_dynstr(&sqlcmd->sqltxt, newstr(sqltext));
        dbh->status = sqlite3_prepare_v2(dbh->dbf, sqlcmd->sqltxt,-1, &sqlcmd->stmt, 0);
        sts = dbcheck(dbh,NULL);
        if (sts) {
            db_flds_bind(dbh, sqlcmd);
            sts = dbcheck(dbh,NULL);
        }
        safe_inc_int(&dbh->sqlcmd->needfinalized);
    }
    return(sts);
}
///////////////////////////////////
sqlcmd_t *sqlstack_push(dbh_t *dbh)
{
    sqlcmd_t *rval;
    if (++dbh->sqlidx == SQL_MAX) {
        fprintf(stderr, "#BLIB:  Internal error SQL stack overflow\n");
        exit(EOVERFLOW);
    }
    rval = &dbh->sqlstack[dbh->sqlidx];
    dbh->sqlcmd=rval;
    
    return(rval);
}
///////////////////////////////////
sqlcmd_t *sqlstack_pop(dbh_t *dbh)
{
    sqlcmd_t *rval;
    
    if (dbh->sqlidx < 0) {
        fprintf(stderr, "#BLIB:  Internal error SQL stack underflow\n");
        exit(EOVERFLOW);
    }
    
    db_finalize_stmt(dbh); // internal uses flag to do if needed
    if (dbh && dbh->sqlcmd) dbh->sqlcmd->getflds = 0;  // not our job to free the value
    
    dbcheck(dbh,NULL);
    
    if (--dbh->sqlidx >= 0 ) {
        rval = &dbh->sqlstack[dbh->sqlidx];
    } else {
        rval = &dbh->sqlstack[0]; // last entry was pop'd make it valid though it should not be used with out a proper push
    }
    dbh->sqlcmd=rval;
    
    return((sqlcmd_t *) NULL);
    
}

objid_t db_get_obj_instance(dbh_t *dbh, bckid_t bck_id, objname_t *objname )
{
    list_t      *flds = (list_t *) NULL;
    objid_t     obj_instance=0;
    
    db_fldsmklist(&flds, "objname", FLD_TEXT , objname);
    db_fldsmklist(&flds, "bck_id" , FLD_INT64, &bck_id);
    
    if (db_exec_sql_flds_push(dbh, "select max(obj_instance) from bck_objects where objname=? and bck_id=?", flds)) {
        obj_instance = sqlite3_column_int(dbh->sqlcmd->stmt, 0);
    }
    
    db_fldsfreelist(&flds);
    sqlstack_pop(dbh);
    return(obj_instance);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

int db_newdb(dbh_t **dbh, char *fnm)
{   
    int status, err;
    char    ans[80];
    stat_t  db_stat;
    char    bckfnm[MAXPATHLEN+1];
    
    // we just need to rename/delete the current file the db_open will create and load schema when it finds it missing
    if (stat(fnm,&db_stat) != -1) {	// make sure we have a file before asking about removal or actually doing it
        if (isatty(fileno(stdin))) {	// only if we on a terminal ask
            fprintf(stderr, "#BLIB:  Do you really want to replace existing database file \"%s\" (Y/N)? ", fnm );
            fgets(ans,sizeof(ans),stdin);
            switch (*ans) {
                case 'Y':
                    unlink(bckfnm); // attempt to remove any old back, catch failure on rename
                    snprintf(bckfnm,sizeof(bckfnm), "%s.bck", fnm);
                    if (rename(fnm, bckfnm) < 0 ) {
                        err=errno;
                        fprintf(stderr, "#BLIB:  Failed to rename \"%s\" to \"%s\" %d:%s\n", fnm, bckfnm,err, strerror(err));
                        exit(err);
                    }
                    break;
                    
                case 'y':
                    fprintf(stderr, "#BLIB:  only upper case Y is acceptable y is not sufficent, exiting\n");
                    exit(EINVAL);
                    break;
                    
                    
                default:
                    fprintf(stderr, "#BLIB:  Database not removed, nothing done exiting...\n");
                    exit(EINVAL);
                    break;
            }
        } else {
            fprintf(stderr, "#BLIB:  Sorry /new cannot be used unless your interative on a terminal\n");
            exit(ENOTTY);   
        }
    }
    
    status = db_open(dbh, fnm);
    return(status);
}

int db_open(dbh_t **dbh, char *fnm)
{
    dbh_t   *rval;
    stat_t  db_stat;
    bool_t  doload_schema;
    int	    err;
    char    *errmsg;
    
    if (dbh == (dbh_t **) NULL ) {
        fprintf(stderr, "#BLIB:  Internal error null pointer passed into %s at line %d\n",__PRETTY_FUNCTION__,__LINE__);
        return(SQLITE_CANTOPEN);
    }
    if (*dbh != (dbh_t *) NULL ) { // must have an previous open
        rval = *dbh;
        if (rval->open == YES ) db_close(rval);
        nzfree(&rval->errmsg);
        nzfree(&rval->fnm);
    } else {
        if ((rval = malloc(sizeof(dbh_t))) == (dbh_t *) NULL ) {
            fprintf(stderr, "#BLIB:  Error allocating database control block\n");
            return(SQLITE_CANTOPEN);
        } else {
            bzero(rval,sizeof(dbh_t)); // clear out the new block
        }
    }
    
    rval->fnm	      = newstr(fnm);
    rval->errmsg      = newstr("not an error");
    
    doload_schema=NO;			// see if we need to load schema ie created new database file
    if (stat(fnm,&db_stat) == -1) {
        err=errno;
        if (errno == ENOENT) {
            doload_schema = YES;
        } else {
            fprintf(stderr, "#BLIB:  Error accessing database file \"%s\" %s\n", fnm, strerror(err));
            return(SQLITE_CANTOPEN);
        }
    }
    
    rval->saved_umask = umask(007);	// allow group rw permission to be set
    rval->status = sqlite3_open(fnm, &rval->dbf);
    if ( rval->status != BLIBDB_OK ) {
        err = rval->status;					// save open fail status
        errmsg = newstr((char *) sqlite3_errmsg(rval->dbf));	// save errmsg from open fail
        sqlite3_close(rval->dbf);				// dont care about status, best effort
        // set the errmsg to the  saved open fail
        replace_dynstr(&rval->errmsg ,  errmsg);
        rval->status = err;					// restore saved status of open fail
    } else {
        rval->open = YES;
        rval->status = BLIBDB_OK;
    }
    umask(rval->saved_umask); // restore the umask now the database is open
    
    if ((rval->status == BLIBDB_OK ) && (doload_schema == YES )) {
        load_schema(rval);
    }
    bzero(rval->sqlstack,sizeof(rval->sqlstack));
    rval->sqlidx=-1; // nothing on stack yet
    *dbh = rval; // give it to them
    if (BLIB.verbose) fprintf(stderr, "#BLIB:  Accessing database file \"%s\"\n", rval->fnm);
    rval->status = sqlite3_busy_handler(rval->dbf, db_busy_handler , (void*) "Busy" );
    return(rval->status);
}

int db_busy_handler(void *tag,int lockoccurence)
{
    int div;
    long rnd;
    long spin;
    const int onesecond=351470318; // 1second spin on 3GHz core2 duo x86_64 specifically my mac laptop, its going to be wrong on different process though its not going to be significant
    
    rnd=random()%1000;
    
    if (BLIB.debug>4) fprintf(stderr, "%s: %s called with occurence: %d\n", (char *) tag, __PRETTY_FUNCTION__, lockoccurence);
    
    
    if (lockoccurence) { // 1st time we going to try again imediatly 
        div = lockoccurence % 10000;
    	if (div  ==  0) {
            fprintf(stderr, "%s:%s:Database is still locked after %d tries\n", (char *) tag, __PRETTY_FUNCTION__, lockoccurence);
    	}
        spin = (onesecond/1000) * rnd * (lockoccurence); // an attempt to provide random wait
        while(spin--);
        if (lockoccurence > 2000000 ) {
            fprintf(stderr, "%s:Database has been locked for too long, giving up\n", (char *) tag);
            return(0);
        }
    }
    return(1); // tell them to try again, bumpers...
}

int db_close(dbh_t *dbh)
{
    if (!dbh) {
        fprintf(stderr, "#BLIB:  Internal error %s called with null database pointer\n", __PRETTY_FUNCTION__);
        return(-1);
    }
    
    if ( dbh->open == YES ) {
        dbh->open = NO;
        dbh->status = sqlite3_close(dbh->dbf);
        // status 5 aka SQLITE_BUSY if we have prepared statements not released
        if (dbh->status == 5 ) {
            fprintf(stderr, "wtf its broken\n");
        }
        dbcheck(dbh,"#BLIB:  Error closing database: %d\n", dbh->status);    		    
    } else {
        dbh->status = 0;
        replace_dynstr(&dbh->errmsg,  newstr("#BLIB:  Database already closed"));
    }
    if (dbh->status != 0 ) {
        fprintf(stderr, "%s\n", dbh->errmsg);
    }
    return(dbh->status);
}

void	db_finish(dbh_t *dbh)
{
    if (!dbh) return;
    if (dbh->open == YES ) db_close(dbh);
    nzfree(&dbh->fnm);
    nzfree(&dbh->errmsg);
    free(dbh);
}

int	dbcheck(dbh_t *dbh, char *errmsg, ...)
{
    va_list args;
    char    *errstr;
    size_t  len;
    
    if (errmsg) {
        va_start(args,errmsg);
        len = VASPRINTF(&errstr, errmsg, args);
        va_end(args);
        if (!errstr) {
            fprintf(stderr, "Failure in " __PRETTY_FUNCTION__ "to allocate error message %llu bytes\n", (llu_t) len);
            exit(ENOMEM);	    // whats the point if we that screwed that we cant allocate a few bytes
        }
    }  else {
        errstr=(char *) NULL;
    }  
    switch(dbh->status) {
        case BLIBDB_OK:         //  0    Successful result 
        case BLIBDB_ROW:        // 100   sqlite3_step() has another row ready
        case BLIBDB_DONE:       // 101   sqlite3_step() has finished executing
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
        case BLIBDB_CONSTRAINT: // 19    Abort due to constraint violation 
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
                fprintf(stderr, "#BLIB:  Error: %s\n", dbh->errmsg);
            }
            return(FALSE);
            break;
        default:
            replace_dynstr(&dbh->errmsg, newstr("Unknown error return code %d", dbh->status));
            fprintf(stderr, "%s\n", dbh->errmsg);
            return(FALSE);
    }
}

////////////////////////////////////////////////////////////////////////////////////

int  copy_results_volume(dbh_t *dbh, void *recp)
{
    vol_t 	*rec;
    list_t	*flds = (list_t *) NULL;
    
    rec = (vol_t *) recp;
    
    bzero(rec,sizeof(vol_t));
    
    db_fldsmklist(&flds, "bck_id", FLD_INT64, &rec->bck_id);
    db_fldsmklist(&flds, "label" , FLD_TEXT, &rec->label);
    db_fldsmklist(&flds, "state", FLD_TEXT, &rec->state);
    db_fldsmklist(&flds, "media", FLD_TEXT, &rec->media);
    db_fldsmklist(&flds, "usage", FLD_INT, &rec->usage);
    db_fldsmklist(&flds, "groupname", FLD_TEXT, &rec->groupname);
    db_fldsmklist(&flds, "location", FLD_TEXT, &rec->location);
    
    db_fldsmklist(&flds, "librarydate", FLD_INT, &rec->librarydate);
    db_fldsmklist(&flds, "offsitedate", FLD_INT, &rec->offsitedate);
    
    dbh->sqlcmd->getflds = flds;
    db_columns(dbh);
    db_fldsfreelist(&dbh->sqlcmd->getflds);
    return(dbh->status);
}

int  copy_results_backup(dbh_t *dbh, void *recp)
{
    list_t	*flds = (list_t *) NULL;
    
    backups_t *rec;    
    rec = (backups_t *) recp;
    bzero(rec,sizeof(backups_t));
    
    db_fldsmklist(&flds, "bck_id"    , FLD_INT64, &rec->bck_id);
    db_fldsmklist(&flds, "node"      , FLD_TEXT , &rec->node);
    db_fldsmklist(&flds, "start"     , FLD_INT  , &rec->start);
    db_fldsmklist(&flds, "end"       , FLD_INT  , &rec->end);
    db_fldsmklist(&flds, "expiredate", FLD_INT  , &rec->expiredate);
    db_fldsmklist(&flds, "desc"      , FLD_TEXT , &rec->desc);
    
    dbh->sqlcmd->getflds = flds;
    db_columns(dbh);
    db_fldsfreelist(&dbh->sqlcmd->getflds);
    return(dbh->status);
}

int  copy_results_bck_objects(dbh_t *dbh, void *recp)
{
    list_t	*flds = (list_t *) NULL;
    bckobj_t *rec;
    
    rec = (bckobj_t *) recp;
    bzero(rec,sizeof(bckobj_t));    
    
    db_fldsmklist(&flds, "bck_id"      , FLD_INT64, &rec->bck_id);
    db_fldsmklist(&flds, "objname"     , FLD_TEXT , &rec->objname);
    db_fldsmklist(&flds, "obj_instance", FLD_INT  , &rec->obj_instance);
    db_fldsmklist(&flds, "start"       , FLD_INT  , &rec->start);
    db_fldsmklist(&flds, "end"         , FLD_INT  , &rec->end);
    db_fldsmklist(&flds, "size"        , FLD_INT64, &rec->size);
    
    dbh->sqlcmd->getflds = flds;
    db_columns(dbh);
    db_fldsfreelist(&dbh->sqlcmd->getflds);
    return(dbh->status);
}

int copy_results_vol_obj(dbh_t *dbh, void *recp)
{
    list_t	*flds = (list_t *) NULL;
    vol_obj_t *rec;
    
    rec = (vol_obj_t *) recp;
    
    bzero(rec,sizeof(vol_obj_t));    
    
    db_fldsmklist(&flds, "bck_id"      , FLD_INT64, &rec->bck_id);
    db_fldsmklist(&flds, "objname"     , FLD_TEXT , &rec->objname);
    db_fldsmklist(&flds, "obj_instance", FLD_INT  , &rec->obj_instance);
    db_fldsmklist(&flds, "label"       , FLD_TEXT , &rec->label);
    db_fldsmklist(&flds, "fileno"      , FLD_INT  , &rec->fileno);
    db_fldsmklist(&flds, "start"       , FLD_INT  , &rec->start);
    db_fldsmklist(&flds, "end"         , FLD_INT  , &rec->end);
    db_fldsmklist(&flds, "size"        , FLD_INT64, &rec->size);
    
    dbh->sqlcmd->getflds = flds;
    db_columns(dbh);
    db_fldsfreelist(&dbh->sqlcmd->getflds);
    return(dbh->status); 
}

int	copy_results_bck_errors(dbh_t  *dbh, void *recp)
{
    list_t	 *flds = (list_t *) NULL;
    bck_errors_t *rec;
    
    rec = (bck_errors_t *) recp;
    
    bzero(rec,sizeof(bck_errors_t));    
    
    db_fldsmklist(&flds, "bck_id"      , FLD_INT64, &rec->bck_id);
    db_fldsmklist(&flds, "label"       , FLD_TEXT , &rec->label);
    db_fldsmklist(&flds, "objname"     , FLD_TEXT , &rec->objname);
    db_fldsmklist(&flds, "obj_instance", FLD_INT, &rec->obj_instance);
    db_fldsmklist(&flds, "errtime"     , FLD_INT  , &rec->errtime);
    db_fldsmklist(&flds, "errmsg"      , FLD_TEXT , &rec->errmsg);
    
    dbh->sqlcmd->getflds = flds;
    db_columns(dbh);
    db_fldsfreelist(&dbh->sqlcmd->getflds);
    return(dbh->status); 
    
}


////////////////////////////////////////////////////////////////////////////////////

int db_flds_bind(dbh_t *dbh, sqlcmd_t *sqlcmd)
{
    int	db_status=-1;
    dbfld_t  *fld;
    entry_t   *fld_ent;
    
    if (sqlcmd->putflds) {
        fld_ent = sqlcmd->putflds->head;
        while (fld_ent) {
            fld = (dbfld_t *) fld_ent->e;
            switch(fld->fldtype) {
                case     FLD_TEXT:
                    db_status = sqlite3_bind_text(sqlcmd->stmt, fld->fldidx, fld->fldval, fld->fldlen, SQLITE_TRANSIENT);
                    break;
                case FLD_INT:
                    db_status = sqlite3_bind_int(sqlcmd->stmt, fld->fldidx, *( int *) fld->fldval);
                    break;
                case FLD_INT64:
                    db_status = sqlite3_bind_int64(sqlcmd->stmt, fld->fldidx, *(sqlite3_int64 *)fld->fldval);
                    break;
                default:
                    fprintf(stderr, "#BLIB:  Unsupported key type  %d passed to %s\n", fld->fldtype,__PRETTY_FUNCTION__);
                    exit(EINVAL);
                    break;
            }
            if (db_status < BLIBDB_OK) {
                replace_dynstr(&dbh->errmsg , newstr("#BLIB:  Error binding field %s : %s\n", fld->fldname, (char *) sqlite3_errmsg(dbh->dbf)));
                return(db_status);
            }
            fld_ent = fld_ent->np;
    	}
    }
    return(db_status);
}

int 	db_columns(dbh_t *dbh)
{
    dbfld_t  *fld;
    entry_t   *fld_ent;
    const unsigned char	*sp;
    sqlcmd_t	*sqlcmd;
    
    sqlcmd = dbh->sqlcmd;
    if (sqlcmd->getflds) {
        fld_ent = sqlcmd->getflds->head;
        while (fld_ent) {
            fld = (dbfld_t *) fld_ent->e;
            switch(fld->fldtype) {
                case     FLD_TEXT:
                    sp =  sqlite3_column_text(sqlcmd->stmt, fld->fldidx-1);
                    strcpy((char *) fld->fldval, (char *) sp);
                    fld->fldlen = strlen((char *) sp);
                    break;
                case FLD_INT:
                    *(int *) fld->fldval  = sqlite3_column_int(sqlcmd->stmt, fld->fldidx-1);
                    break;
                case FLD_INT64:
                    *(int64_t *) fld->fldval = sqlite3_column_int64(sqlcmd->stmt, fld->fldidx-1);
                    break;
                default:
                    fprintf(stderr, "#BLIB:  Unsupported key type  %d passed to %s\n", fld->fldtype,__PRETTY_FUNCTION__);
                    exit(EINVAL);
                    break;
            }
            if (!dbcheck(dbh, "#BLIB:  Error fetching column %s : %s\n", fld->fldname, (char *) sqlite3_errmsg(dbh->dbf))) {
                return(dbh->status);
            }
            fld_ent = fld_ent->np;
    	}
    }
    return(dbh->status);
}

dbfld_t *db_fldsmklist(list_t **fldhead,const char *fldname,  fld_type_t fldtype, void *fldptr)
{
    // create a chain of flds to be used by db_flds_bind()
    dbfld_t	*fld;
    entry_t	*ent;
    dbfld_t	*last_fld;
    int		fldidx;
    list_t	*head;
    
    fldidx=1; // 0 for column 1 for bind
    if (fldhead ) {
        head = *fldhead;
        if ( head && head->tail && head->tail->e) {
            last_fld = (dbfld_t *) head->tail->e;
            fldidx = last_fld->fldidx+1;
        }
    }
    
    fld          = db_flds_new();
    fld->fldname = fldname;
    fld->fldtype = fldtype;
    fld->fldidx  = fldidx;
    
    switch(fldtype) {
        case FLD_INT:
            fld->fldlen = sizeof(int);
            fld->fldval = (int *) fldptr;
            break;
        case FLD_INT64:
            fld->fldlen = sizeof(int64_t);
            fld->fldval = (int64_t *) fldptr;
            break;
        case FLD_TEXT:
            fld->fldlen = strlen((char *) fldptr);
            fld->fldval = (char *) fldptr;
            break;
        default:
            fprintf(stderr, "Internal error unknown field type %d in %s\n", fldtype, __PRETTY_FUNCTION__);
            exit(EINVAL);
            break;
    }    
    
    ent = new_entry(fld);
    list_insert_tail(fldhead, ent);
    return(fld); // give back the fld so they can update as they need
}

int db_fldsdump(list_t *fldhead)
{
    // create a chain of flds to be used by db_flds_bind()
    dbfld_t	*fld;
    entry_t	*ent;
    
    if (!fldhead ) return(-1);
    
    ent = fldhead->head;
    while (ent) {
        fld = ent->e;
        db_flds_display(fld);
        ent=ent->np;
    }
    return(0);
}

void db_flds_display(dbfld_t *fld)
{
    FILE *outfd=stderr;
    
    fprintf(outfd, "#BLIB:  Entry: %s type: %s value:", fld->fldname, fldtype_name(fld->fldtype));
    switch(fld->fldtype) {
        case FLD_INT:
            fprintf(outfd , "%d",   *(int *) fld->fldval);
            break;
        case FLD_INT64:
            fprintf(outfd, "%llu", (llu_t) *(int64_t *)  fld->fldval);
            break;
        case FLD_TEXT:
            fprintf(outfd, "%s",    (char *) fld->fldval);
            break;
        default:
            fprintf(outfd, "Internal error unknown field type %d in %s\n", fld->fldtype, __PRETTY_FUNCTION__);
            exit(EINVAL);
            break;
    }
    fputc('\n', outfd);
}



const char *fldtype_name(fld_type_t fldtype)
{
    const char *rval;
    static char buf[64];
    
    switch(fldtype) {
        case FLD_TEXT:
            rval="FLD_TEXT";
            break;
        case FLD_INT:
            rval="FLD_INT";
            break;
        case FLD_INT64:
            rval="FLD_INT64";
            break;
        default:
            snprintf(buf,sizeof(buf), "Unknown field type: %d\n", fldtype);
            rval = buf;
            break;
    }
    return(rval);
}

dbfld_t *db_flds_byname(list_t *flds, const char *fldname)
{
    dbfld_t	*fld;
    entry_t	*ent;
    dbfld_t 	*rval;
    rval = NULL;
    
    if (flds) {
        ent = flds->head;
        while(ent) {
            fld = (dbfld_t *) ent->e;
            if (strcmp(fld->fldname, fldname) == 0 ) {
                rval = fld;
                break;
            }
            ent = ent->np;
        }
    }
    return(rval);
    
}



uint64_t db_flds_int64byname(list_t *flds, const char *fldname)
{
    uint64_t rval;
    dbfld_t *fld;
    
    rval = 0;
    fld = db_flds_byname(flds, fldname);
    if (fld) {
        if (fld->fldtype != FLD_INT64) {
            fprintf(stderr, "#BLIB:  FATAL internal error, %s called to get a field %s  but the field is type %s\n", __PRETTY_FUNCTION__, fldtype_name(FLD_INT64), fldtype_name(fld->fldtype));
            exit(EINVAL);
        }
        rval = *(uint64_t *) fld->fldval;
    }
    return(rval);
}

char *db_flds_textbyname(list_t *flds, const char *fldname)
{
    char *rval;
    dbfld_t *fld;
    
    rval = 0;
    fld = db_flds_byname(flds, fldname);
    if (fld) {
        if (fld->fldtype != FLD_TEXT) {
            fprintf(stderr, "#BLIB:  FATAL internal error, %s called to get a field %s  but the field is type %s\n", __PRETTY_FUNCTION__, fldtype_name(FLD_TEXT), fldtype_name(fld->fldtype));
            exit(EINVAL);
        }
        rval = (char *) fld->fldval;
    }
    return(rval);
}


dbfld_t *db_flds_new()
{
    dbfld_t *db_flds_new;
    if ((db_flds_new = malloc(sizeof(dbfld_t))) == (dbfld_t *) NULL ) {
        fprintf(stderr, "#BLIB:  Error allocating memory for new fld\n");
        exit(ENOMEM);
    }
    bzero(db_flds_new,sizeof(dbfld_t));
    return(db_flds_new);
}

void db_fldsfreelist(list_t **listp)
{
    if (listp) free_list(listp, NULL);
}


int db_find(dbh_t *dbh, char *sqltext,list_t *key, void *results, int (*copyresult)(dbh_t *dbh, void *rsp), find_type_t flag)
{
    int rval=0;
    // return true while no error and a row available
    rval = 0;
    switch(flag) {
        case FND_EQUAL:  // only diff from FND_FIRST is we finish the statement on one row
        case FND_FIRST:
            sqlstack_push(dbh);
            dbh->sqlcmd->putflds = key;
            rval = db_prepare_sql(dbh, dbh->sqlcmd, sqltext);
            break;
            
        case FND_NEXT: // just come in a step we already setup
            break;
            
        case FND_FINSIHED:
            if (dbh->sqlcmd) {
                db_fldsfreelist(&dbh->sqlcmd->putflds);
                sqlstack_pop(dbh);
            }
            return(0);
            break;
            
        default:
            fprintf(stderr, "#BLIB:  Internal Unknown flag value %d passed to %s\n", flag,__PRETTY_FUNCTION__);
            exit(EINVAL);
            break;
    }
    if (dbh->sqlcmd) {
        if (dbcheck(dbh,NULL)) {
            dbh->status = sqlite3_step(dbh->sqlcmd->stmt);	    // either inital fetch or subsequent next
            if (dbh->status == BLIBDB_ROW) {
                (copyresult)(dbh, results);
                rval = 1;
            } else {
                rval = 0;
            }
        } else {
            if (dbh->sqlcmd) {
                db_fldsfreelist(&dbh->sqlcmd->putflds);
                sqlstack_pop(dbh);
            }
            rval = 0;  // error return false
        }
        
        if ((dbh->status == BLIBDB_DONE) || ( flag == FND_EQUAL ) || (flag == FND_FINSIHED)) {
            if (dbh->sqlcmd) {
                db_fldsfreelist(&dbh->sqlcmd->putflds);
                sqlstack_pop(dbh);
            }
        }
    }
    if (dbh->sqlcmd) db_fldsfreelist(&dbh->sqlcmd->putflds); // always drop the flds our callers insist on reminding us with fresh values
    // apparently rebinding does work
    return(rval);
}

////////////////////////////////////////////////////////////////////////////////////

int db_find_volumes_label(dbh_t *dbh,vol_t *volkey, vol_t *rec, find_type_t flag)
{
    int rval;
    char *sqltext;
    list_t *key = (list_t *) NULL;
    
    if (flag == FND_EQUAL) {
        sqltext="select * from main.volumes where label = ? order by label";
    } else {
        sqltext="select * from main.volumes where label >= ? order by label";
    }
    
    if (volkey) db_fldsmklist(&key,"label",  FLD_TEXT , (void *) &volkey->label.str);
    rval =  db_find(dbh, sqltext, key,(void *)rec, copy_results_volume , flag);
    
    return(rval);
}

int db_find_volume_bylabel(dbh_t *dbh,blabel_t *label, vol_t *resultrec, find_type_t flag)
{
    int rval;
    list_t *key = (list_t *) NULL;
    char *sqltext;
    
    if (flag == FND_EQUAL) {
        sqltext="select * from main.volumes where label = ?";
    } else {
        sqltext="select * from main.volumes where label like ?";
    }
    
    bzero(resultrec, sizeof(vol_t));
    db_fldsmklist(&key,"label",  FLD_TEXT , (void *) label);
    rval =  db_find(dbh, sqltext, key, (void *)resultrec, copy_results_volume , flag);
    
    return(rval);
}

int	db_find_backups_by_expire(dbh_t *dbh, backups_t *bckrec, find_type_t flag)
{
    int		rval;
    char	*sqltext;
    list_t	*flds = (list_t *) NULL;
    
    sqltext="select * from main.backups  order by expiredate desc";
    
    bzero(bckrec, sizeof(backups_t));
    rval =  db_find(dbh, sqltext, flds, (void *)bckrec, copy_results_backup , flag);
    
    return(rval);
}

int db_find_volume_for_vol_obj(dbh_t *dbh, vol_obj_t *volobjrec, vol_t *volrec, find_type_t flag)
{
    int		rval;
    char	*sqltext;
    list_t	*flds = (list_t *) NULL;
    
    sqltext="select * from main.volumes where label=?"; // there can only be one conner, need to check that it does have our bck_id caller can do that
    
    db_fldsmklist(&flds, "label"     , FLD_TEXT , &volobjrec->label);    
    
    bzero(volrec, sizeof(vol_t));
    rval =  db_find(dbh, sqltext, flds, (void *)volrec, copy_results_volume , flag);
    
    return(rval);
}


int	db_find_vol_obj_for_bck_object(dbh_t *dbh, bckobj_t *bckobjrec, vol_obj_t *volobjrec, find_type_t flag)
{
    int		rval;
    char	*sqltext;
    list_t	*flds = (list_t *) NULL;
    
    sqltext="select * from vol_obj where bck_id=? and objname=? and obj_instance=? order by start, label, fileno";
    
    // select * from vol_obj where bck_id= bckobjrec->bck_id and objname= bckobjrec->objname and obj_instance= bckobjrec->obj_instance
    db_fldsmklist(&flds, "bck_id"      , FLD_INT64, &bckobjrec->bck_id);
    db_fldsmklist(&flds, "objname"     , FLD_TEXT , &bckobjrec->objname);
    db_fldsmklist(&flds, "obj_instance", FLD_INT  , &bckobjrec->obj_instance);
    
    
    bzero(volobjrec, sizeof(vol_obj_t));
    rval =  db_find(dbh, sqltext, flds, (void *)volobjrec, copy_results_vol_obj , flag);
    
    return(rval);
}


int	db_find_bck_objects_by_name(dbh_t *dbh, objname_t *objname, bckobj_t *bckobjrec, find_type_t flag)
{
    int		rval;
    char	*sqltext;
    list_t	*flds = (list_t *) NULL;
    
    // sqltext="select * from main.bck_objects where objname >=? order by objname,obj_instance desc";
    if (flag == FND_EQUAL) {
        sqltext="select * from main.bck_objects where objname =? order by objname, bck_id desc obj_instance"; 
    } else {
        sqltext="select * from main.bck_objects where objname >=? order by objname, bck_id desc obj_instance";
    }
    
    
    db_fldsmklist(&flds, "objname", FLD_TEXT, objname);
    
    
    bzero(bckobjrec, sizeof(bckobj_t));
    rval =  db_find(dbh, sqltext, flds, (void *)bckobjrec, copy_results_bck_objects , flag);
    
    return(rval);
}

int	db_find_backups_orderbckid(dbh_t *dbh, backups_t *bckrec, bckid_t bckid, find_type_t flag)
{
    int		rval;
    char	*sqltext;
    list_t	*flds = (list_t *) NULL;
    
    if (flag == FND_EQUAL ) {
        sqltext="select * from main.backups where bck_id =? order by bck_id";
    } else {
        sqltext="select * from main.backups where bck_id >=? order by bck_id";
    }
    
    db_fldsmklist(&flds, "bck_id", FLD_INT64, &bckid);
    
    
    bzero(bckrec, sizeof(backups_t));
    rval =  db_find(dbh, sqltext, flds, (void *)bckrec, copy_results_backup , flag);
    
    return(rval);
}


int db_find_volume_free(dbh_t *dbh, vol_t *volrec, find_type_t flag)
{
    int rval;
    
    rval =  db_find(dbh, "select * from volumes where state='F' order by recorddate asc", NULL, (void *)volrec, copy_results_volume , flag);
    
    return(rval);
}

int db_find_volume_expired(dbh_t *dbh, vol_t *rec, find_type_t flag)
{
    int rval;
    
    char *sqltext="select volumes.* from volumes,  backups where volumes.bck_id = backups.bck_id and expiredate <= strftime('%%s','now') and state='A' order by recorddate asc";
    
    rval =  db_find(dbh, sqltext, NULL, (void *)rec, copy_results_volume , flag);
    
    return(rval);
}


int db_find_vol_obj_id_notbckid_label(dbh_t *dbh, list_t *key, vol_obj_t *volobj2del,find_type_t flag)
{
    
    int rval;
    
    char *sqltext="select * from vol_obj where bck_id != ? and label=?";
    
    rval =  db_find(dbh, sqltext, key, (void *)volobj2del, copy_results_vol_obj , flag);
    
    return(rval);
}

int db_find_current_volobj(dbh_t *dbh, vol_obj_t *volobjrec)
{
    
    int		rval;
    list_t	*key_bckid= (list_t *) NULL;
    // TODO: check this is now correct for multi entries of objname in one backup
    char *sqltext="select * from vol_obj where bck_id=? and objname=? and end=0 and obj_instance=?";
    //----------------------------------------------
    db_fldsmklist(&key_bckid,"bck_id"      ,  FLD_INT64, (void *) &volobjrec->bck_id);
    db_fldsmklist(&key_bckid,"objname"     ,  FLD_TEXT , (void *) &volobjrec->objname);
    db_fldsmklist(&key_bckid,"obj_instance",  FLD_INT  , (void *) &volobjrec->obj_instance);
    
    rval =  db_find(dbh, sqltext, key_bckid, (void *)volobjrec, copy_results_vol_obj , FND_EQUAL);
    
    return(rval);
}    

///////////////////////////////////////////////////////////////////
int	db_setvolume_free(dbh_t *dbh, char *label)
{
    list_t	*flds= (list_t *) NULL;
    
    db_fldsmklist(&flds, "label", FLD_TEXT, (void *) label);
    
    if (db_exec_sql_flds_pushpop(dbh, "update volumes set state='F' where label=?", flds))  {
	    replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Volume: %s now free\n",label ));
    } else {
	    replace_dynstr(&dbh->errmsg, newstr("#BLIB:  failed freeing \"%s\" : %s!!\n", label, dbh->errmsg));
	    if (BLIB.debug > 2) db_fldsdump(flds);
    }
    
    db_fldsfreelist(&flds);
    return(dbh->status);    
}

int db_setvolume_used(dbh_t *dbh, blabel_t *label, bckid_t bckid)
{
    list_t	*flds= (list_t *) NULL;
    int	rval;
    
    db_fldsmklist(&flds, "bck_id"    , FLD_INT64, (void *) &bckid);
    db_fldsmklist(&flds, "label"     , FLD_TEXT , (void *) label);
    
    if (db_exec_sql_flds_pushpop(dbh, "update volumes set bck_id=?, state='A'  where label=?", flds))  {
        replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Volume: %s now Allocated\n",(char *) label));
        rval=1;
    } else {
        replace_dynstr(&dbh->errmsg, newstr("#BLIB:  failed Allocating \"%s\" : %s!!\n", (char *) label, dbh->errmsg));
        rval=0;
        if (BLIB.debug > 2) {
            fputs(dbh->errmsg, stderr);
            db_fldsdump(flds);
        }
    }
    
    db_fldsfreelist(&flds);
    return(rval);   
}

int	db_end_vol_obj(dbh_t *dbh, vol_obj_t *volobjrec)
{
    list_t	*flds = (list_t *) NULL;
    int	rval;
    
    db_fldsmklist(&flds, "end"         , FLD_INT  , (void *) &volobjrec->end);
    db_fldsmklist(&flds, "size"        , FLD_INT64, (void *) &volobjrec->size);
    db_fldsmklist(&flds, "bck_id"      , FLD_INT64, (void *) &volobjrec->bck_id);
    db_fldsmklist(&flds, "objname"     , FLD_TEXT , (void *) &volobjrec->objname);
    db_fldsmklist(&flds, "obj_instance", FLD_INT  , (void *) &volobjrec->obj_instance);
    db_fldsmklist(&flds, "label"       , FLD_TEXT , (void *) &volobjrec->label);
    
    // TODO: check this is now correct for multi instance of objname in one backup
    if (db_exec_sql_flds_pushpop(dbh, "update vol_obj set end=?, size=? where bck_id=? and objname=? and obj_instance=? and label=? and end=0", flds))  {
        if (BLIB.debug) fprintf(stderr, "#BLIB:  usage closed for %llu on %s of label: %s\n",(llu_t) volobjrec->bck_id, (char *) &volobjrec->objname, (char *) &volobjrec->label );
        rval=1;
    } else {
        rval=0;
        replace_dynstr(&dbh->errmsg, newstr("#BLIB:  error closing usage on label \"%s\" for %s : %s!!\n", (char *) &volobjrec->label
                                            ,(char *) &volobjrec->objname, dbh->errmsg));
        if (BLIB.debug > 2) {
            fputs(dbh->errmsg, stderr);
            db_fldsdump(flds);
        }
    }
    
    db_fldsfreelist(&flds);
    return(rval);    
}

fld_type_t db_fldtype_from_valtype(valtype_e valtype)
{
    fld_type_t rval;
    
    switch(valtype) {
        case VT_NONE:
            rval = FLD_UNKNOWN;
            break;
        case VT_STR:
        case VT_FILENAM:
        case VT_LABEL:
        case VT_STATE:
            rval=FLD_TEXT;
            break;
        case VT_DATE:
        case VT_INT:
            rval=FLD_INT;
            break;
        case VT_INT64:
            rval=FLD_INT64;
            break;
        default:
            rval = FLD_UNKNOWN;
            break;
    }
    return(rval);
}

int do_upd_vol(dbh_t *dbh, char *label, cmd_t *cmd )
{
    sqlcmd_t 	*sqlcmd;
    list_t	*flds = (list_t *) NULL;
    fld_type_t	fldtype;
    char	*volfldname;
    
    if ((cmd) && (label)) {
        sqlcmd = sqlstack_push(dbh);
        
        fldtype = db_fldtype_from_valtype(cmd->param->val_type);
        volfldname = cmd->param->sql_fldnam;
        
        db_fldsmklist(&flds, cmd->param->sql_fldnam, fldtype , (void *) cmd->val);
        db_fldsmklist(&flds, "label"               , FLD_TEXT, (void *) label);
        sqlcmd->putflds = flds;
        
        if (db_prepare_sql(dbh, sqlcmd, "update volumes set %s=? where label=?", volfldname))  {
            dbh->status = sqlite3_step(sqlcmd->stmt);	    // do it
            if (dbcheck(dbh,NULL)) {
                if (!BLIB.quiet ) fprintf(stderr, "#BLIB:  \"%s\" updated!!\n", volfldname);
            } else {
                fprintf(stderr, "#BLIB:  \"%s\" failed updated :%s!!\n", volfldname, dbh->errmsg);
            }
        } else {
            fprintf(stderr, "#BLIB:  Error updating database %s\n", dbh->errmsg);
            if (BLIB.debug > 2) db_fldsdump(flds);
        }
        
        db_fldsfreelist(&sqlcmd->putflds);
        sqlstack_pop(dbh);
    }
    
    return(dbh->status);
}


void db_update_volume(dbh_t *dbh,filt_t *filtrec, vol_t *rec)
{
    do_upd_vol(dbh,rec->label.str,filtrec->label);
    do_upd_vol(dbh,rec->label.str,filtrec->state);
    do_upd_vol(dbh,rec->label.str,filtrec->media);
    do_upd_vol(dbh,rec->label.str,filtrec->groupname);
    do_upd_vol(dbh,rec->label.str,filtrec->location);    
    do_upd_vol(dbh,rec->label.str,filtrec->usage);
    do_upd_vol(dbh,rec->label.str,filtrec->offsitedate);
}

int db_delete_volume(dbh_t *dbh, vol_t *vol2del)
{
    backups_t	bckrec;
    list_t      *flds = (list_t *) NULL;
    
    
    if (vol2del->bck_id) {
        if (!db_find_backups_by_bck_id(dbh, vol2del->bck_id, &bckrec)) {  // see if there is an attached backups record??
            replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Volume %s can not be deleted while still attached to a backup delete bck_id %llu first\n",vol2del->label.str, (llu_t) vol2del->bck_id));
            dbh->status = SQLITE_CONSTRAINT;
            return(dbh->status);
        }
    }
    
    db_fldsmklist(&flds, "label", FLD_TEXT, (void *) &vol2del->label );
    
    if (db_exec_sql_flds_pushpop(dbh, "delete from main.volumes where label=?", flds)) {
        replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Volume %s deleted!\n",  (char *) &vol2del->label));
    } else {
        replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Error deleting volume with label %s, %s\n", vol2del->label.str, dbh->errmsg));
    }
    
    db_fldsfreelist(&flds);
    return(dbh->status);
}

///////////////////////////////////////////////////////////////
int db_find_backups_by_bck_id(dbh_t *dbh, bckid_t key_bck_id_val, backups_t *bck_rec)
{
    
    list_t *flds = (list_t *) NULL;
    int	rval=0;
    
    db_fldsmklist(&flds, "bck_id", FLD_INT64, (void *) &key_bck_id_val);
    
    if (db_exec_sql_flds_push(dbh, "select * from main.backups where bck_id = ?", flds)) {
        if (dbh->status == BLIBDB_ROW) {
            copy_results_backup(dbh ,bck_rec);
            rval=1;
        }
    }
    
    db_fldsfreelist(&flds);
    sqlstack_pop(dbh);
    return(rval);
}

/////////////////////////////////////////////////////////////////////////////////

int db_insert_volumes(dbh_t *dbh, vol_t *vol)
{
    list_t	*flds = (list_t *) NULL;
    
    char *sqltext="insert into volumes ("
    "bck_id   , label      , state      , media      , usage, "
    "groupname, location   , librarydate, offsitedate"
    ") values ( ?,?,?,?,?,?,?,?,?)";
    
    db_fldsmklist(&flds , "bck_id"     , FLD_INT64, (void *) &vol->bck_id);
    db_fldsmklist(&flds , "label"      , FLD_TEXT , (void *) &vol->label);
    db_fldsmklist(&flds , "state"      , FLD_TEXT , (void *) &vol->state);
    db_fldsmklist(&flds , "media"      , FLD_TEXT , (void *) &vol->media);
    db_fldsmklist(&flds , "usage"      , FLD_INT  , (void *) &vol->usage);
    
    db_fldsmklist(&flds , "groupname"  , FLD_TEXT , (void *) &vol->groupname);
    db_fldsmklist(&flds , "location"   , FLD_TEXT , (void *) &vol->location);
    db_fldsmklist(&flds , "librarydate", FLD_INT  , (void *) &vol->librarydate);
    db_fldsmklist(&flds , "offsitedate", FLD_INT  , (void *) &vol->offsitedate);
    
    if (!db_exec_sql_flds_pushpop(dbh, sqltext, flds)) {
	    replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Error inserting into volumes: %s\n", dbh->errmsg));
	    if (BLIB.debug > 2 ) db_fldsdump(flds);
    }
    
    db_fldsfreelist(&flds);
    
    return(dbh->status);
}

int db_insert_backups(dbh_t *dbh, backups_t *bck_rec)
{
    list_t	*flds = (list_t *) NULL;
    
    char *sqltext="insert into backups ("
    "bck_id,"
    "node,"
    "start,"
    "end,"
    "expiredate,"
    "desc"
    ") values ( ?,?,?,?,?,?)";
    
    db_fldsmklist(&flds , "bck_id"     , FLD_INT64, (void *) &bck_rec->bck_id);
    db_fldsmklist(&flds , "node"       , FLD_TEXT , (void *) &bck_rec->node);
    db_fldsmklist(&flds , "start"      , FLD_INT  , (void *) &bck_rec->start);
    db_fldsmklist(&flds , "end"        , FLD_INT  , (void *) &bck_rec->end);
    db_fldsmklist(&flds , "expiredate" , FLD_INT  , (void *) &bck_rec->expiredate);
    db_fldsmklist(&flds , "desc"       , FLD_TEXT , (void *) &bck_rec->desc);
    
    if (!db_exec_sql_flds_pushpop(dbh, sqltext, flds)) {
	    replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Error inserting into backups: %s\n", dbh->errmsg));
	    if (BLIB.debug > 2 ) db_fldsdump(flds);
    }
    
    db_fldsfreelist(&flds);
    return(dbh->status);
    
}

objid_t db_insert_bck_objects(dbh_t *dbh, bckobj_t *bckobjrec)
{
    list_t	*flds = (list_t *) NULL;
    objid_t	obj_instance;
    
    char *sqltext="insert into bck_objects ("
    "bck_id,"
    "objname,"
    "obj_instance,"
    "start,"
    "end,"
    "size"
    ") values ( ?,?,?,?,?,?)";
    
    db_exec_sql(dbh, "BEGIN IMMEDIATE  transaction");
    
    obj_instance = bckobjrec->obj_instance;
    if (obj_instance == 0 ) {
    	obj_instance = db_get_obj_instance(dbh, bckobjrec->bck_id, &bckobjrec->objname);
    	obj_instance++;
    }
    
    db_fldsmklist(&flds , "bck_id"      , FLD_INT64, (void *) &bckobjrec->bck_id);
    db_fldsmklist(&flds , "objname"     , FLD_TEXT , (void *) &bckobjrec->objname);
    db_fldsmklist(&flds , "obj_instance", FLD_INT  , (void *) &obj_instance);
    db_fldsmklist(&flds , "start"       , FLD_INT  , (void *) &bckobjrec->start);
    db_fldsmklist(&flds , "end"         , FLD_INT  , (void *) &bckobjrec->end);
    db_fldsmklist(&flds , "size"        , FLD_INT64, (void *) &bckobjrec->size);
    
    if (db_exec_sql_flds_pushpop(dbh, sqltext, flds)) {
        db_exec_sql(dbh, "commit");
        dbcheck(dbh, "commit error");
    } else {
        replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Error inserting new backup object: %s\n", dbh->errmsg));
        db_exec_sql(dbh, "rollback transaction");  
    }
    
    db_fldsfreelist(&flds);
    return(obj_instance);
}

int db_insert_vol_obj(dbh_t *dbh, vol_obj_t *volobjrec)
{
    list_t	*flds = (list_t *) NULL;
    int rval;
    
    char *sqltext="insert into vol_obj ("
    "bck_id,"
    "objname,"
    "obj_instance,"
    "label,"
    "fileno,"
    "start,"
    "end,"
    "size"
    ") values ( ?,?,?,?,?,?,?,?)";
    
    db_fldsmklist(&flds , "bck_id"      , FLD_INT64, (void *) &volobjrec->bck_id);
    db_fldsmklist(&flds , "objname"     , FLD_TEXT , (void *) &volobjrec->objname);
    db_fldsmklist(&flds , "obj_instance", FLD_INT  , (void *) &volobjrec->obj_instance);
    db_fldsmklist(&flds , "label"       , FLD_TEXT , (void *) &volobjrec->label);
    db_fldsmklist(&flds , "fileno"      , FLD_INT  , (void *) &volobjrec->fileno);
    db_fldsmklist(&flds , "start"       , FLD_INT  , (void *) &volobjrec->start);
    db_fldsmklist(&flds , "end"         , FLD_INT  , (void *) &volobjrec->end);
    db_fldsmklist(&flds , "size"        , FLD_INT64, (void *) &volobjrec->size);    
    
    if (!(rval = db_exec_sql_flds_pushpop(dbh, sqltext, flds))) {
        replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Error inserting new vol_obj: %s\n", dbh->errmsg));
        if (BLIB.debug > 2 ) db_fldsdump(flds);
    }
    
    db_fldsfreelist(&flds);
    
    return(rval);
}

#ifdef db_clear_old_back_for_label_tmp
// This should now be removed since we cant use a tape with out doing a delete backup against the bck_id whic would do this code equiv 
/////////////////////////////////////////////////////////////////////////////////
int db_clear_old_back_for_label(dbh_t *dbh, vol_obj_t *volobjrec)
{
    /* we writing over this label so clear off any rememberance of it from previous backups
     remove old references to the label
     for x_bck_id, x_objname in (select bck_id, objname from vol_obj where bck_id != <bckid> and label=<label>)
     do
     delete from bck_objects where bck_id=<x_bck_id> and objname=<x_objname>;
     delete from vol_obj     where bck_id=<x_bck_id> and objname=<x_objname> and label=<label>;
     done
     */
    int 	dbstatus;
    vol_obj_t	volobj2del;
    
    list_t	*key_bck_id= (list_t *) NULL;
    
    list_t	*key_bck_objects= (list_t *) NULL;
    
    list_t	*key_vol_obj= (list_t *) NULL;
    dbfld_t	*vol_obj_objname;
    dbfld_t	*vol_obj_label;
    
    bzero(&volobj2del,       	sizeof(vol_obj_t));
    
    /* 
     delete from bck_objects where bck_id=bo.bck_id and objname=bo.objname and label=bo.label
     */
    
    //--------------Key find on vol_obj -- select bck_id, objname from vol_obj where bck_id != <bckid> and label=<label> into volobj2del
    db_fldsmklist(&key_bck_id,"bck_id", FLD_INT64, (void *) &volobjrec->bck_id);
    db_fldsmklist(&key_bck_id,"label" , FLD_TEXT , (void *) &volobjrec->label);
    
    //-------------- Key delete from bck_objects -- delete from bck_objects where bck_id=<volobj2del.bck_id> and objname=<volobj2del.objname>
    db_fldsmklist(&key_bck_objects,"bck_id"  , FLD_INT64, (void *) &volobj2del.bck_id);
    db_fldsmklist(&key_bck_objects,"objname" , FLD_TEXT , (void *) &volobj2del.objname);
    
    //-------------- Key delete from vol_obj ----- delete from vol_obj where bck_id=<volobj2del.bck_id> and objname=<volobj2del.objname> and label=<volobj2del.label>
    db_fldsmklist(&key_vol_obj,"bck_id" , FLD_INT64, (void *) &volobj2del.bck_id);
    vol_obj_objname = db_fldsmklist(&key_vol_obj,"objname", FLD_TEXT , (void *) &volobj2del.objname);
    vol_obj_label   = db_fldsmklist(&key_vol_obj,"label"  , FLD_TEXT , (void *) &volobj2del.label);
    
    
    dbstatus = db_find_vol_obj_id_notbckid_label(dbh, key_bck_id, &volobj2del, FND_FIRST);
    while (dbstatus) {
        vol_obj_objname->fldlen	  = strlen((char *) &volobj2del.objname);
        // vol_obj_objname->fldval   = (already set)
        
        db_exec_sql_flds_pushpop(dbh, "delete from bck_objects where bck_id=? and objname=?", key_bck_objects);
        
        vol_obj_objname->fldlen	  = strlen((char *) &volobj2del.objname);
        vol_obj_label->fldlen	  = strlen((char *) &volobj2del.label);
        
        db_exec_sql_flds_pushpop(dbh, "delete from vol_obj where bck_id=? and objname=? and label=?", key_vol_obj);
        
        dbstatus = db_find_vol_obj_id_notbckid_label(dbh,  NULL, &volobj2del, FND_NEXT);
    }
    db_fldsfreelist(&key_bck_objects);
    db_fldsfreelist(&key_bck_objects); 
    db_fldsfreelist(&key_vol_obj);
    return(dbh->status);
}
#endif /* db_clear_old_back_for_label */

bcount_t db_count_volumes_in_backup_id(dbh_t *dbh, bckid_t bck_id)
{
    bcount_t 	rcount;
    list_t	*flds = (list_t *) NULL;
    
    rcount = -1;
    
    db_fldsmklist(&flds, "bck_id"      , FLD_INT64, (void *) &bck_id);
    
    if (db_exec_sql_flds_push(dbh, "select count(*) from volumes where bck_id=?", flds)) {
        rcount = sqlite3_column_int64(dbh->sqlcmd->stmt, 0);
    } else {
        replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Error counting volumes in backup id %llu: %s\n",(llu_t) bck_id, dbh->errmsg));
        if (BLIB.debug > 2 ) db_fldsdump(flds);
    }   
    
    db_fldsfreelist(&flds);
    sqlstack_pop(dbh);
    return(rcount);
}

int db_count_vol_obj_label(dbh_t *dbh, bckid_t bck_id, blabel_t *label)
{
    //   
    int 	rcount;
    list_t	*flds = (list_t *) NULL;
    
    rcount = -1;
    
    db_fldsmklist(&flds, "bck_id"      , FLD_INT64, (void *) &bck_id);
    db_fldsmklist(&flds, "label"       , FLD_TEXT , label);
    
    if (db_exec_sql_flds_push(dbh, "select count(*) from vol_obj where bck_id=? and label=?", flds)) {
        rcount = sqlite3_column_int(dbh->sqlcmd->stmt, 0);
    } else {
        replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Error inserting new vol_obj: %s\n", dbh->errmsg));
        if (BLIB.debug > 2 ) db_fldsdump(flds);
    }   
    
    db_fldsfreelist(&flds);
    sqlstack_pop(dbh);
    return(rcount);
}


bcount_t db_vol_obj_sumsize(dbh_t *dbh, vol_obj_t *volobjrec)
{
    list_t   *flds = (list_t *) NULL;
    bcount_t	totalsize = 0;
    
    db_fldsmklist(&flds, "bck_id"   , FLD_INT64,  (void *) &volobjrec->bck_id);
    db_fldsmklist(&flds, "objname"  , FLD_TEXT ,  (void *) &volobjrec->objname);
    
    if (db_exec_sql_flds_push(dbh, "select sum(size) totalsize from vol_obj where bck_id=? and objname=?", flds))  {
        totalsize = sqlite3_column_int64(dbh->sqlcmd->stmt, 0);
        if (!dbcheck(dbh,NULL)) {
            fprintf(stderr, "#BLIB:  error summing totalsize for  object \"%s\"  %s!!\n", (char *) &volobjrec->objname, (char *) dbh->errmsg);
        }
    } 
    
    db_fldsfreelist(&flds);
    sqlstack_pop(dbh);
    return(totalsize);  
}

//////////////////////////////////////////////////////////////////////////////////

int db_update_bck_object_size_end(dbh_t *dbh, bckid_t bckid, objname_t *objname, objid_t obj_instance,  time_t end, bcount_t totalsize)
{
    list_t	*flds = (list_t *) NULL;
    
    db_fldsmklist(&flds, "end"         , FLD_INT  , (void *) &end);
    db_fldsmklist(&flds, "size"        , FLD_INT  , (void *) &totalsize);
    db_fldsmklist(&flds, "bckid"       , FLD_INT64, (void *) &bckid);
    db_fldsmklist(&flds, "objname"     , FLD_TEXT , (void *) objname);
    db_fldsmklist(&flds, "obj_instance", FLD_INT  , (void *) &obj_instance);
    
    // update the most recent
    if (db_exec_sql_flds_pushpop(dbh, "update bck_objects set end=?, size=? where bck_id=? and objname=? and end=0 "
                                 " and   obj_instance=?", flds))  {
        if (dbcheck(dbh,NULL)) {
            if (BLIB.debug) fprintf(stderr, "#BLIB:  closed bck_object for %llu on object: %s\n", bckid, (char *) objname );
        } else {
            replace_dynstr(&dbh->errmsg, newstr("#BLIB:  error closing bck_object on object \"%s\"  %s!!\n", (char *) objname, (char *) dbh->errmsg));
        }
        
        if (BLIB.debug) {
            fprintf(stderr, "#BLIB:  end updated on %llu %s\n", (llu_t) bckid, (char *)  objname);
        }
    } else {
        replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Error updating database %s\n", dbh->errmsg));
    }
    
    db_fldsfreelist(&flds);
    return(dbh->status);  
}


int	db_insert_bckerror(dbh_t *dbh, bck_errors_t *bckerror)
{
    list_t	*flds = (list_t *) NULL;
    
    // [bck_id|label|objname|obj_instance|errtime|errmsg]
    char *sqltext="insert into bck_errors ("
    "bck_id,"
    "label,"
    "objname,"
    "obj_instance,"
    "errtime,"
    "errmsg"
    ") values ( ?,?,?,?,?,?)";
    
    db_fldsmklist(&flds , "bck_id"      , FLD_INT64, (void *) &bckerror->bck_id);
    db_fldsmklist(&flds , "label"       , FLD_TEXT , (void *) &bckerror->label);
    db_fldsmklist(&flds , "objname"     , FLD_TEXT , (void *) &bckerror->objname);
    db_fldsmklist(&flds , "obj_instance", FLD_INT  , (void *) &bckerror->obj_instance);
    db_fldsmklist(&flds , "errtime"     , FLD_INT  , (void *) &bckerror->errtime);
    db_fldsmklist(&flds , "errmsg"      , FLD_TEXT , (void *) &bckerror->errmsg);
    
    if (!db_exec_sql_flds_pushpop(dbh, sqltext, flds)) {
	    replace_dynstr(&dbh->errmsg,newstr("#BLIB:  Error inserting into bck_errors: %s\n", dbh->errmsg));
	    if (BLIB.debug > 2 ) db_fldsdump(flds);
    }
    
    db_fldsfreelist(&flds);
    return(dbh->status);
}



time_t db_lookup_endofbackup(dbh_t *dbh,bckid_t bck_id)
{
    list_t   *flds = (list_t *) NULL;
    time_t	end = 0;
    
    db_fldsmklist(&flds, "bck_id"   , FLD_INT64,  (void *) &bck_id);
    
    if (db_exec_sql_flds_push(dbh, "select max(end) from vol_obj where bck_id=?", flds))  {
        end = sqlite3_column_int(dbh->sqlcmd->stmt, 0);
    } 
    
    db_fldsfreelist(&flds);
    sqlstack_pop(dbh);
    return(end);  
}

int db_inc_volume_usage(dbh_t *dbh, bckid_t bck_id)
{    
    sqlstack_push(dbh);
    db_exec_sql_bckid(dbh,  "update volumes set usage=usage+1  where volumes.bck_id=?", bck_id);
    
    sqlstack_pop(dbh);
    return(dbh->status);  
    
}

int db_update_backups_end(dbh_t *dbh, bckid_t bck_id, time_t end)
{
    list_t	*flds = (list_t *) NULL;
    sqlcmd_t *sqlcmd = (sqlcmd_t *) NULL;
    
    sqlcmd = sqlstack_push(dbh);
    
    
    db_fldsmklist(&flds, "end"    , FLD_INT  , (void *) &end);
    db_fldsmklist(&flds, "bckid"  , FLD_INT64, (void *) &bck_id);
    sqlcmd->putflds = flds;
    
    if (db_prepare_sql(dbh, sqlcmd, "update backups set end=? where bck_id=?"))  {
        dbh->status = sqlite3_step(sqlcmd->stmt);	    // do it
        if (!dbcheck(dbh,NULL)) {
            if (BLIB.debug) fprintf(stderr, "#BLIB:  Error running sql to closed backups for %llu %s\n", (llu_t) bck_id, (char *) dbh->errmsg);
        }
        
    } else {
        fprintf(stderr, "#BLIB:  Error preparing to run sql to close backups for %llu %s\n", (llu_t) bck_id, (char *) dbh->errmsg);
    }
    db_fldsfreelist(&sqlcmd->putflds);
    sqlstack_pop(dbh);
    return(dbh->status);  
}


int db_delete_backup_id(dbh_t *dbh, bckid_t bck_id)
{
    
    /*
     delete from bck_objects where bck_id=<bckid>;
     delete from vol_obj where bck_id=<bckid>;
     update volumes set bck_id=0 where bck_id=<bckid>;
     */  
    if (BLIB.debug)  {
        fprintf(stderr, "Deleting backup %llu\n", (llu_t ) bck_id);
    }
    
    sqlstack_push(dbh);
    db_exec_sql(dbh,     "begin transaction");    
    db_exec_sql_bckid(dbh, "delete from bck_objects               where bck_id=?", bck_id);
    db_exec_sql_bckid(dbh, "delete from vol_obj                   where bck_id=?", bck_id);
    db_exec_sql_bckid(dbh, "delete from backups                   where bck_id=?", bck_id);
    db_exec_sql_bckid(dbh, "update bck_errors set bck_id=0        where bck_id=?", bck_id);
    db_exec_sql_bckid(dbh, "update volumes set bck_id=0,state='F' where bck_id=?", bck_id);
    db_exec_sql(dbh,     "commit"); 
    
    sqlstack_pop(dbh);
    return(dbh->status);      
}

int	db_update_backups(dbh_t *dbh,backups_t *bckrec)
{
    list_t	*flds = (list_t *) NULL;
    
    db_fldsmklist(&flds , "node"       , FLD_TEXT , (void *) &bckrec->node);
    db_fldsmklist(&flds , "start"      , FLD_INT  , (void *) &bckrec->start);
    db_fldsmklist(&flds , "end"        , FLD_INT  , (void *) &bckrec->end);
    db_fldsmklist(&flds , "expiredate" , FLD_INT  , (void *) &bckrec->expiredate);
    db_fldsmklist(&flds , "desc"       , FLD_TEXT , (void *) &bckrec->desc);
    db_fldsmklist(&flds , "bck_id"     , FLD_INT64, (void *) &bckrec->bck_id);
    
    db_exec_sql_flds_pushpop(dbh, "update backups set node=?, start=?, end=?, expiredate=?, desc=? where bck_id=?", flds);
    
    db_fldsfreelist(&flds);
    return(dbh->status);
}

int	db_find_bck_objects_by_bckid(dbh_t *dbh, bckid_t bck_id,bckobj_t *bckobjrec, find_type_t flag)
{
    int 	rval;
    list_t	*key_bckid= (list_t *) NULL;
    char *sqltext="select * from main.bck_objects where bck_id=? order by start, objname, obj_instance";
    // TODO: this may well be still wrong to get a correct multple object backup looking right
    
    
    bzero(bckobjrec, sizeof(bckobj_t));
    db_fldsmklist(&key_bckid,"bck_id" ,  FLD_INT64 , (void *) &bck_id);
    
    rval =  db_find(dbh, sqltext, key_bckid, (void *)bckobjrec, copy_results_bck_objects , flag);
    
    return(rval);
}

int	db_find_vol_obj_from_objects(dbh_t *dbh, bckobj_t *key, vol_obj_t *volobjrec, find_type_t flag)
{
    int 	rval;
    list_t	*keylist= (list_t *) NULL;
    char *sqltext="select * from main.vol_obj where bck_id=? and objname=? and obj_instance=? order by bck_id, objname ,start, label";
    
    // TODO: here we have a problem determining what vol_obj belong to a bck_objects
    
    bzero(volobjrec, sizeof(vol_obj_t));
    db_fldsmklist(&keylist,"bck_id"       ,  FLD_INT64, (void *) &key->bck_id);
    db_fldsmklist(&keylist,"objname"      ,  FLD_TEXT , (void *) &key->objname);
    db_fldsmklist(&keylist,"obj_instance" ,  FLD_INT  , (void *) &key->obj_instance);
    
    rval =  db_find(dbh, sqltext, keylist, (void *)volobjrec, copy_results_vol_obj , flag);
    
    return(rval);
    
}

bcount_t db_count_bck_errors(dbh_t *dbh, vol_obj_t *key)
{
    bcount_t	rcount;
    list_t	*flds = (list_t *) NULL;
    
    rcount = -1;
    
    db_fldsmklist(&flds,"bck_id"        ,  FLD_INT64, (void *) &key->bck_id);
    db_fldsmklist(&flds,"objname"       ,  FLD_TEXT , (void *) &key->objname);
    db_fldsmklist(&flds,"label"         ,  FLD_TEXT , (void *) &key->label);
    db_fldsmklist(&flds,"obj_instance"  ,  FLD_INT , (void *) &key->obj_instance);
    
    if (db_exec_sql_flds_push(dbh, "select count(*) from main.bck_errors where bck_id=? and objname=? and label=? and obj_instance=?", flds)) {	
        rcount = sqlite3_column_int64(dbh->sqlcmd->stmt, 0);
    }
    
    db_fldsfreelist(&flds);
    sqlstack_pop(dbh);
    return(rcount);
}

bcount_t db_count_bck_errors_bck_id(dbh_t *dbh, bckid_t bck_id)
{
    bcount_t	rcount;
    list_t	*flds = (list_t *) NULL;
    
    rcount = -1;
    
    db_fldsmklist(&flds,"bck_id"        ,  FLD_INT64, (void *) &bck_id);
    
    if (db_exec_sql_flds_push(dbh, "select count(*) from main.bck_errors where bck_id=?", flds)) {	
        rcount = sqlite3_column_int64(dbh->sqlcmd->stmt, 0);
    }
    
    db_fldsfreelist(&flds);
    sqlstack_pop(dbh);
    return(rcount);
}

bcount_t db_count_notset(dbh_t *dbh, bckid_t bck_id)
{
    bcount_t	rcount;
    list_t	*flds = (list_t *) NULL;
    
    rcount = -1;
    
    db_fldsmklist(&flds,"bck_id"        ,  FLD_INT64, (void *) &bck_id);
    
    if (db_exec_sql_flds_push(dbh, "select count(*) from main.backups where bck_id=? and end=0" , flds)) {	
        rcount = sqlite3_column_int64(dbh->sqlcmd->stmt, 0);
    }
    sqlstack_pop(dbh);
    
    if (db_exec_sql_flds_push(dbh, "select count(*) from bck_objects where bck_id=? and end=0" , flds)) {	
        rcount += sqlite3_column_int64(dbh->sqlcmd->stmt, 0);
    }
    
    db_fldsfreelist(&flds);
    sqlstack_pop(dbh);
    return(rcount);
}


int	db_find_bck_errors(dbh_t *dbh, vol_obj_t *volobjkey, bck_errors_t *bckerrrec, find_type_t flag)
{
    int 	rval;
    list_t	*keylist= (list_t *) NULL;
    char 	*sqltext="select * from main.bck_errors where bck_id=? and objname=? and label=? and obj_instance=?";
    
    bzero(bckerrrec, sizeof(bck_errors_t));
    db_fldsmklist(&keylist,"bck_id"       ,  FLD_INT64, (void *) &volobjkey->bck_id);
    db_fldsmklist(&keylist,"objname"      ,  FLD_TEXT , (void *) &volobjkey->objname);
    db_fldsmklist(&keylist,"label"        ,  FLD_TEXT , (void *) &volobjkey->label);
    db_fldsmklist(&keylist,"obj_instance" ,  FLD_INT , (void *) &volobjkey->obj_instance);
    
    rval =  db_find(dbh, sqltext, keylist, (void *)bckerrrec, copy_results_bck_errors , flag);
    
    return(rval);
}

int	db_find_volumes_by_bckid(dbh_t *dbh, bckid_t bck_id, vol_t *volrec, find_type_t flag)
{
    int 	rval;
    list_t	*keylist= (list_t *) NULL;
    char	*sqltext="select * from main.volumes where bck_id=?";
    
    bzero(volrec, sizeof(vol_t));
    db_fldsmklist(&keylist,"bck_id" ,  FLD_INT64 , (void *) &bck_id);
    
    rval =  db_find(dbh, sqltext, keylist, (void *)volrec, copy_results_volume, flag);
    
    return(rval);
}



bcount_t db_get_size(dbh_t *dbh, vol_t *volrec)
{
    // get the size of data written to the given label
    list_t      *flds = (list_t *) NULL;
    bcount_t     rval=0;
    
    db_fldsmklist(&flds, "bck_id" , FLD_INT64 , &volrec->bck_id);
    db_fldsmklist(&flds, "label", FLD_TEXT,  &volrec->label);
    
    if (db_exec_sql_flds_push(dbh, "select sum(size) from vol_obj where bck_id=? and label=?", flds)) {
        rval = sqlite3_column_int64(dbh->sqlcmd->stmt, 0);
    }
    
    db_fldsfreelist(&flds);
    sqlstack_pop(dbh);
    return(rval);
}


int  db_get_duration(dbh_t *dbh, vol_t *volrec)
{
    // get the size of data written to the given label
    list_t      *flds = (list_t *) NULL;
    int     rval=0;
    
    db_fldsmklist(&flds, "bck_id" , FLD_INT64 , &volrec->bck_id);
    db_fldsmklist(&flds, "label", FLD_TEXT,  &volrec->label);
    
    if (db_exec_sql_flds_push(dbh, "select sum(end-start) from vol_obj where bck_id=? and label=?", flds)) {
        rval = sqlite3_column_int(dbh->sqlcmd->stmt, 0);
    }
    
    db_fldsfreelist(&flds);
    sqlstack_pop(dbh);
    return(rval);
}

bcount_t  db_count_sqltext(dbh_t *dbh, char *sqltext)
{
    bcount_t rval;
    
    rval = 0;
    if (db_exec_sql_flds_push(dbh, sqltext, NULL)) {
        rval = sqlite3_column_int64(dbh->sqlcmd->stmt, 0);
    } else {
        replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Error counting %s\n",sqltext, dbh->errmsg));
    } 
    
    sqlstack_pop(dbh);
    
    return(rval);
}



///////////////////////////////////////////////////////////////////////////////////////
///////////////////////// VERIFY //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

int db_read_bck_objects_fault1(dbh_t *dbh, bckobj_t *bck_obj, find_type_t flag)
{
    int 	rval;
    char 	*sqltext;
    
    // bck_objects (bck_id) -> backups
    
    sqltext = "select * from bck_objects where bck_id not in (select bck_id from backups)";

    bzero(bck_obj, sizeof(bckobj_t));
    
    rval =  db_find(dbh, sqltext, NULL, (void *)bck_obj, copy_results_bck_objects , flag);
    
    return(rval);
}



int db_read_bck_objects_fault2(dbh_t *dbh, bckobj_t *bck_obj, find_type_t flag)
{
    int 	rval;
    char 	*sqltext;
    
    // bck_objects (bck_id, objname, obj_instance) -> vol_obj
    
    sqltext = "select  * from (select bo.*, vo.bck_id as vobck_id from bck_objects bo left outer  join vol_obj vo on bo.bck_id = vo.bck_id and bo.obj_instance  = vo.obj_instance and bo.objname   = vo.objname ) where vobck_id is null";
    
    bzero(bck_obj, sizeof(bckobj_t));
    
    rval =  db_find(dbh, sqltext, NULL, (void *)bck_obj, copy_results_bck_objects , flag);
    
    return(rval);
}



int	db_read_bck_errors_fault1(dbh_t *dbh, bck_errors_t *bckerrrec, find_type_t flag)
{
    int 	rval;
    
    // checking: (bck_id > 0 )   -> backups
    char 	*sqltext="select * from bck_errors where bck_id>0 and bck_id not in ( select bck_id from backups)";
    
    bzero(bckerrrec, sizeof(bck_errors_t));
    
    rval =  db_find(dbh, sqltext, NULL, (void *)bckerrrec, copy_results_bck_errors , flag);
    
    return(rval);
}

int	db_read_bck_errors_fault2(dbh_t *dbh, bck_errors_t *bckerrrec, find_type_t flag)
{
    int 	rval;
    char    *sqltext;
    
    // checking: (bck_id, objname, obj_instance) -> bck_objects
   /*
    
    TODO: work out why this doesnt work
    sqltext="select * from bck_errors e left outer  join bck_objects o on"
            " e.bck_id        = o.bck_id and"
            " e.obj_instance  = o.obj_instance"
            " and e.objname   = o.objname"
            " and o.bck_id is null";
    // select e.* o.bckid as obckid from bck_errors e left outer  join bck_objects o on e.bck_id = o.bck_id and e.obj_instance  = o.obj_instance and e.objname   = o.objname and obck_id is null
    */
    
    sqltext = "select  * from (select e.*, o.bck_id as obck_id from bck_errors e left outer  join bck_objects o on e.bck_id = o.bck_id and e.obj_instance  = o.obj_instance and e.objname   = o.objname ) where obck_id is null";
    

    bzero(bckerrrec, sizeof(bck_errors_t));
    
    rval =  db_find(dbh, sqltext, NULL, (void *)bckerrrec, copy_results_bck_errors , flag);
    
    return(rval);
}

int db_read_vol_obj_fault1(dbh_t *dbh, vol_obj_t *vol_obj, find_type_t flag)
{
    int 	rval;
    char    *sqltext;
    
    // (bck_id, objname, obj_instance) -> bck_objects
    sqltext = "select  * from (select vo.*, bo.bck_id as bobck_id from vol_obj vo left outer  join bck_objects bo on vo.bck_id = bo.bck_id and vo.obj_instance  = bo.obj_instance and vo.objname   = bo.objname ) where bobck_id is null";    
    bzero(vol_obj, sizeof(vol_obj_t));
    
    rval =  db_find(dbh, sqltext, NULL, (void *)vol_obj, copy_results_vol_obj , flag);
    
    return(rval);
}

int db_read_vol_obj_fault2(dbh_t *dbh, vol_obj_t *vol_obj, find_type_t flag)
{
    int 	rval;
    char    *sqltext;
    
    // (bck_id, label) -> volumes
    sqltext = "select  * from (select vo.*, v.bck_id as bobck_id from vol_obj vo left outer  join volumes v on vo.bck_id = v.bck_id and vo.label  = v.label ) where bobck_id is null";    
    bzero(vol_obj, sizeof(vol_obj_t));
    
    rval =  db_find(dbh, sqltext, NULL, (void *)vol_obj, copy_results_vol_obj , flag);
    
    return(rval);
}


int db_read_vol_volumes1(dbh_t *dbh, vol_t *volume, find_type_t flag)
{
    int 	rval;
    char    *sqltext;
    
    // (bck_id>0, label) -> vol_obj
    sqltext = "select  * from (select vo.*, vo.bck_id as vobck_id from volumes v left outer  join vol_obj vo on v.bck_id = v.bck_id and v.label  = vo.label where v.bck_id > 0 ) where vobck_id is null";
    
    bzero(volume, sizeof(vol_t));
    
    rval =  db_find(dbh, sqltext, NULL, (void *)volume, copy_results_volume , flag);
    
    return(rval);
}

int db_read_vol_volumes2(dbh_t *dbh, vol_t *volume, find_type_t flag)
{
    int 	rval;
    char    *sqltext;
    
    // (bck_id > 0 )   -> backups
    sqltext = "select  * from volumes where bck_id > 0 and bck_id not in (select bck_id from backups) ";
    
    bzero(volume, sizeof(vol_t));
    
    rval =  db_find(dbh, sqltext, NULL, (void *)volume, copy_results_volume , flag);
    
    return(rval);
}



/////////////////////////////////////////////////////////////////
/////////////////// DISPLAY RECORDS /////////////////////////////
/////////////////////////////////////////////////////////////////
#define BCK_ERR_HDR     "[bck_id       |label|             objname| obj_instance|errtime|errmsg]"
#define BCK_ERR_FMT_LINE    "%-13llu  %-17.17s  %-17s %3d %-17.17s  %s\n"

void db_display_bck_errors(FILE *fd, bck_errors_t *bckerrrec)
{
    datestr_t errtime;
    /*
    1              test               test                0 NotSet             0

    12345678901234567890123456789012345678901234567890123456789012345678901234567890
     */
                    
    copy_datestr(&errtime, (datestr_t *) fmtctime(bckerrrec->errtime));
    fprintf(fd, BCK_ERR_FMT_LINE, 
                              (llu_t ) bckerrrec->bck_id
                            , bckerrrec->label.str
                            , bckerrrec->objname.str
                            , bckerrrec->obj_instance
                            , errtime.str
                            , bckerrrec->errmsg.str);
    
}


#define BCKOBJ_FMT_HDR  "[bck_id       |objname        |obj_instance  |start          |end|   size]"
#define BCKOBJ_FMT_LINE "%-13llu    %-17.17s %3d %s %s %-13llu\n"

void db_display_bck_object(FILE *fd, bckobj_t *bo)
{
    datestr_t start, end;

    copy_datestr(&start, (datestr_t *) fmtctime(bo->start));
    copy_datestr(&end,   (datestr_t *) fmtctime(bo->end));
    fprintf(fd, BCKOBJ_FMT_LINE, 
            (llu_t ) bo->bck_id, 
            pstr(bo->objname.str,""), 
            bo->obj_instance, 
            start.str, 
            end.str, 
            (llu_t) bo->size);
    
}


#define VOLOBJ_FMT_HDR  "[bck_id       |objname|obj_instance|label|fileno|start|end|size]"
#define VOLOBJ_FMT_LINE     "%-13llu  %-17s %3d %-17.17s %3d %s %s %llu\n"

void db_display_vol_obj(FILE *fd, vol_obj_t *vo)
{
    datestr_t start, end;
    
    copy_datestr(&start, (datestr_t *) fmtctime(vo->start));
    copy_datestr(&end,   (datestr_t *) fmtctime(vo->end));
    fprintf(fd, VOLOBJ_FMT_LINE, 
            (llu_t ) vo->bck_id, 
            pstr(vo->objname.str,""), 
            vo->obj_instance, 
            pstr(vo->label.str,""),
            vo->fileno,
            start.str, 
            end.str, 
            (llu_t) vo->size);
    
}

#define VOL_FMT_HDR  "[bck_id|label|state|media|usage|groupname|location|librarydate|offsitedate]"
#define VOL_FMT_LINE "%-13llu  %-17s %s %s %3d %-17.17s %s %s %s\n"


void db_display_volume(FILE *fd, vol_t *vol)
{
    datestr_t librarydate, offsitedate;
    copy_datestr(&librarydate, (datestr_t *) fmtctime(vol->librarydate));
    copy_datestr(&offsitedate, (datestr_t *) fmtctime(vol->offsitedate));
    fprintf(fd, VOL_FMT_LINE, 
            (llu_t ) vol->bck_id, 
            pstr(vol->label.str,""),
            fmt_state((char *) &vol->state),
            pstr(vol->media.str,""), 
            vol->usage,
            pstr(vol->groupname.str, ""),
            pstr(vol->location.str,""),
            librarydate.str,
            offsitedate.str
            );

    

    
}


///////////////////////////////////////////////
int     db_verify(fio_t *outfd, dbh_t *dbh)
{
    
    // bcount_t total_rows;
    
    int fnd_sts;
    /*
     sqlite> .tables
     backups      bck_errors   bck_objects  vol_obj      volumes
     */
    // backups_t    backups;
    bck_errors_t bck_errors;
    bckobj_t     bck_objects;
    vol_obj_t    vol_objs;
    vol_t        volumes;
    
    // uint64_t    backups_errcount=0;
    uint64_t    bck_errors_errcount=0;
    uint64_t    bck_objects_errcount=0;
    uint64_t    vol_objs_errcount=0;
    uint64_t    volumes_errcount=0;
    uint64_t    errcount=0;
    uint64_t    total_errcount=0;
    
    
    /*
    bck_id maybe zero as we keep a history gainst label for a backup volume to track its errors
    */
/////////////////////////////
// check table: BCK_ERRORS //
/////////////////////////////
        // (bck_id > 0 )   -> backups
    fprintf(outfd->fd, "Verifying table: bck_errors\n");
    // total_rows = db_count_sqltext(dbh, "select count(*) from bck_errors");
    
    fprintf(outfd->fd, "  bck_errors(bck_id>0) -> backups: ");
    fnd_sts = db_read_bck_errors_fault1(dbh, &bck_errors, FND_FIRST);
    if (fnd_sts) {
        
        fprintf(outfd->fd, "ERROR\n");
        if (BLIB.verbose) fprintf(outfd->fd, "%s\n", BCK_ERR_HDR);
        while (fnd_sts) {
            errcount++;
            if (BLIB.verbose) db_display_bck_errors(outfd->fd, &bck_errors);
            fnd_sts = db_read_bck_errors_fault1(dbh, &bck_errors, FND_NEXT);
        }
        fprintf(outfd->fd, "  Error count: %llu\n", (llu_t) errcount);
        bck_errors_errcount += errcount;
        errcount=0;
        
    } else {
        fprintf(outfd->fd, "OK\n");
    }
    

    
    // (bck_id, objname, obj_instance) -> bck_objects
    fprintf(outfd->fd, "  bck_errors(bck_id, objname, obj_instance) -> bck_objects: ");
    fnd_sts = db_read_bck_errors_fault2(dbh, &bck_errors, FND_FIRST);
    if (fnd_sts) {
        fprintf(outfd->fd, "ERROR\n");
        if (BLIB.verbose) fprintf(outfd->fd, "%s\n", BCK_ERR_HDR);
        while (fnd_sts) {
            errcount++;
            if (BLIB.verbose) db_display_bck_errors(outfd->fd, &bck_errors);
            fnd_sts = db_read_bck_errors_fault2(dbh, &bck_errors, FND_NEXT);
        }
        fprintf(outfd->fd, "  Error count: %llu\n", (llu_t) errcount);
        bck_errors_errcount += errcount;
        errcount=0;
    } else {
        fprintf(outfd->fd, "OK\n");
    }
    if (bck_errors_errcount) fprintf(outfd->fd, "  bck_errors: Error count: %llu\n", (llu_t) bck_errors_errcount);
    total_errcount += bck_errors_errcount;
    bck_errors_errcount=0;

   
//////////////////////////////
// check table: BCK_OBJECTS //
//////////////////////////////

 /*
 bck_objects      [bck_id|objname|obj_instance   |start|end|size]
    bck_objects (bck_id>0) -> backups 
    bck_objects (bck_id, objname, obj_instance) -> vol_obj
 */
    fprintf(outfd->fd, "\nVerifying table: bck_objects\n");
    fprintf(outfd->fd, "  bck_objects(bck_id>0) -> vol_obj: ");
    fnd_sts = db_read_bck_objects_fault1(dbh, &bck_objects, FND_FIRST);
    if (fnd_sts) {
        fprintf(outfd->fd, "ERROR\n");
        if (BLIB.verbose) fprintf(outfd->fd, "%s\n", BCKOBJ_FMT_HDR);
        while (fnd_sts) {
            errcount++;
            if (BLIB.verbose) db_display_bck_object(outfd->fd, &bck_objects);
            fnd_sts = db_read_bck_objects_fault1(dbh, &bck_objects, FND_NEXT);
        }
        fprintf(outfd->fd, "  Error count: %llu\n", (llu_t) errcount);
        bck_objects_errcount += errcount;
        errcount=0;
        
    } else {
        fprintf(outfd->fd, "OK\n");
    }

    
    fprintf(outfd->fd, "  bck_objects(bck_id, objname, obj_instance) -> vol_obj: ");
    fnd_sts = db_read_bck_objects_fault2(dbh, &bck_objects, FND_FIRST);
    if (fnd_sts) {
        fprintf(outfd->fd, "ERROR\n");
        if (BLIB.verbose) fprintf(outfd->fd, "%s\n", BCKOBJ_FMT_HDR);
        while (fnd_sts) {
            errcount++;
            if (BLIB.verbose) db_display_bck_object(outfd->fd, &bck_objects);
            fnd_sts = db_read_bck_objects_fault2(dbh, &bck_objects, FND_NEXT);
        }
        fprintf(outfd->fd, "  Error count: %llu\n", (llu_t) errcount);
        bck_objects_errcount += errcount;
        errcount=0;
        
    } else {
        fprintf(outfd->fd, "OK\n");
    }
    
    
    if (bck_objects_errcount) fprintf(outfd->fd, "  bck_objects: Error count: %llu\n", (llu_t) bck_objects_errcount);
    total_errcount += bck_objects_errcount;
    bck_objects_errcount=0;

    
//////////////////////////////
// check table: VOL_OBJ     //
//////////////////////////////
 /* 
 vol_obj    [bck_id|objname|obj_instance      |label|fileno|start|end|size]
    (bck_id, objname, obj_instance) -> bck_objects
    (bck_id, label) -> volumes
 */
    
    fprintf(outfd->fd, "\nVerifying table: vol_obj\n");
    fprintf(outfd->fd, "  vol_obj(bck_id, objname, obj_instance) -> bck_objects: ");
    fnd_sts = db_read_vol_obj_fault1(dbh, &vol_objs, FND_FIRST);
    if (fnd_sts) {
        fprintf(outfd->fd, "ERROR\n");
        if (BLIB.verbose) fprintf(outfd->fd, "%s\n", VOLOBJ_FMT_HDR);
        while (fnd_sts) {
            errcount++;
            if (BLIB.verbose) db_display_vol_obj(outfd->fd, &vol_objs);
            fnd_sts = db_read_vol_obj_fault1(dbh, &vol_objs, FND_NEXT);
        }
        fprintf(outfd->fd, "  Error count: %llu\n", (llu_t) errcount);
        vol_objs_errcount += errcount;
        errcount=0;
        
    } else {
        fprintf(outfd->fd, "OK\n");
    }
    
    fprintf(outfd->fd, "  vol_obj(bck_id, label) -> volumes: ");
    fnd_sts = db_read_vol_obj_fault2(dbh, &vol_objs, FND_FIRST);
    if (fnd_sts) {
        fprintf(outfd->fd, "ERROR\n");
        if (BLIB.verbose) fprintf(outfd->fd, "%s\n", VOLOBJ_FMT_HDR);
        while (fnd_sts) {
            errcount++;
            if (BLIB.verbose) db_display_vol_obj(outfd->fd, &vol_objs);
            fnd_sts = db_read_vol_obj_fault2(dbh, &vol_objs, FND_NEXT);
        }
        fprintf(outfd->fd, "  Error count: %llu\n", (llu_t) errcount);
        vol_objs_errcount += errcount;
        errcount=0;
        
    } else {
        fprintf(outfd->fd, "OK\n");
    }
    

    if (vol_objs_errcount) fprintf(outfd->fd, "  vol_obj: Error count: %llu\n", (llu_t) vol_objs_errcount);
    total_errcount += vol_objs_errcount;
    vol_objs_errcount=0;

    
//////////////////////////////
// check table: VOLUMES     //
//////////////////////////////
    
 /*
 volumes    [bck_id|label|state|media|usage|groupname|location|librarydate|recorddate|offsitedate]
    (bck_id>0, label) -> vol_obj
    (bck_id > 0 )   -> backups
 */
    
    fprintf(outfd->fd, "\nVerifying table: volumes\n");
    fprintf(outfd->fd, "  volumes(bck_id>0, label) -> vol_obj: ");
    fnd_sts = db_read_vol_volumes1(dbh, &volumes, FND_FIRST);
    if (fnd_sts) {
        fprintf(outfd->fd, "ERROR\n");
        if (BLIB.verbose) fprintf(outfd->fd, "%s\n", VOLOBJ_FMT_HDR);
        while (fnd_sts) {
            errcount++;
            if (BLIB.verbose) db_display_volume(outfd->fd, &volumes);
            fnd_sts = db_read_vol_volumes1(dbh, &volumes, FND_NEXT);
        }
        fprintf(outfd->fd, "  Error count: %llu\n", (llu_t) errcount);
        volumes_errcount += errcount;
        errcount=0;
        
    } else {
        fprintf(outfd->fd, "OK\n");
    }
    
    fprintf(outfd->fd, "  volumes(bck_id>0 )   -> backups: ");
    fnd_sts = db_read_vol_volumes2(dbh, &volumes, FND_FIRST);
    if (fnd_sts) {
        fprintf(outfd->fd, "ERROR\n");
        if (BLIB.verbose) fprintf(outfd->fd, "%s\n", VOLOBJ_FMT_HDR);
        while (fnd_sts) {
            errcount++;
            if (BLIB.verbose) db_display_volume(outfd->fd, &volumes);
            fnd_sts = db_read_vol_volumes2(dbh, &volumes, FND_NEXT);
        }
        fprintf(outfd->fd, "  Error count: %llu\n", (llu_t) errcount);
        volumes_errcount += errcount;
        errcount=0;
        
    } else {
        fprintf(outfd->fd, "OK\n");
    }
    
    
//////////////////////////////
// check table: BACKUPS     //
//////////////////////////////

/*
 backups    [bck_id|node|start|end|expiredate|desc]
Nothing we havent already checked 
 */
    
     
    
    if (vol_objs_errcount) fprintf(outfd->fd, "  volumes: Error count: %llu\n", (llu_t) volumes_errcount);
    total_errcount += volumes_errcount;
    volumes_errcount=0;
    
    
/////////////////////////////////////// 
    
    if (total_errcount) fprintf(outfd->fd, "TOTAL ERRORS: %llu\n",(llu_t) total_errcount);
    
    return(0);
}



