#ifndef __DATA_STRUCTURE_H__
#define __DATA_STRUCTURE_H__
/*
 // @(#) $Id: data_structures.h,v 1.5 2014/06/17 05:07:57 mark Exp mark $
 //  data_structures.h
 //  blib
 //
 //  Created by mark on 18/10/2010.
 //  Copyright (c) 2010 Garetech Computer Solutions. All rights reserved.
 // $Log: data_structures.h,v $
 // Revision 1.5  2014/06/17 05:07:57  mark
 // seems I had added changedate, offsite date since last ci
 //
 // Revision 1.4  2013/01/21 16:46:45  mark
 // MG add missing recorddate to vol_t structure
 // reformating
 //
 // Revision 1.3  2013/01/20 10:06:26  mark
 // MG changes for time_t to blib_tim_t
 //
 // Revision 1.2  2011/04/11 03:52:00  mark
 // add val_t
 //
 // Revision 1.1  2010/11/16 04:04:31  root
 // Initial revision
 //
 */

#include    "sqlite/sqlite3.h"
#include    <strings.h>
#include    <string.h>
#include    "list.h"
#include    "timefunc.h"

#define	    SQL_MAX	42

typedef enum {
    FND_UNKNOWN=0,
    FND_EQUAL,
    FND_FIRST,
    FND_NEXT,
    FND_FINSIHED
} find_type_t;

typedef enum {
    FLD_UNKNOWN=0,
    FLD_TEXT,
    FLD_INT,
    FLD_INT64,
    FLD_DATE,
    FLD_DOUBLE
} fld_type_t;

typedef union val_s val_t;
union val_s {
    void       *val_voidptr;
    char       *val_charptr;
    int        val_int;
    llu_t      val_int64;
};

typedef struct blibdbfld_s dbfld_t;
struct blibdbfld_s {
    fld_type_t	fldtype;
    int         fldidx;
    const char *fldname;
    size_t      fldlen;
    void       *fldval;  // always a pointer
};

typedef struct {
    sqlite3_stmt    *stmt;
    int             sqllen;
    char            *sqltxt;
    int             needfinalized;
    list_t          *putflds;
    list_t          *getflds;
} sqlcmd_t;

typedef struct {
    mode_t      saved_umask;
    bool_t      open;
    int         status;
    char        *errmsg;
    char        *fnm;
    sqlite3     *dbf;
    int         sqlidx;
    sqlcmd_t    *sqlcmd;
    sqlcmd_t	sqlstack[SQL_MAX];
} dbh_t;

// if these change remember to review the format statment in do_cmd_report()
#define		LABEL_SIZ		     9
#define		MEDIA_NAM_SIZ		 9
#define		STATE_SIZ		     1
#define		DESC_SIZ		   256
#define		NODE_SIZ		   256
#define		OBJ_NAME_SIZ	  1024
#define		GROUPNAM_SIZ	 	 9
#define		LOCNAM_SIZ		    64
#define		ERRMSG_SIZ		   256

#define		LABEL_SIZ_STR		"9"
#define		MEDIA_NAM_SIZ_STR	"9"
#define		STATE_SIZ_STR		"1"
#define		DESC_SIZ_STR		"256"
#define		NODE_SIZ_STR		"256"
#define		OBJ_NAME_SIZ_STR	"1024"
#define		GROUPNAM_SIZ_STR	"9"
#define		LOCNAM_SIZ_STR		"64"
#define		ERRMSG_SIZ_STR		"256"

typedef uint64_t        bckid_t;
typedef uint64_t        bcount_t;
typedef unsigned int	objid_t;

typedef struct {
    char str[LABEL_SIZ+1];
} blabel_t;

typedef struct {
    char str[DESC_SIZ+1];
} desc_t;

typedef struct {
    char str[NODE_SIZ+1];
} node_t;

typedef struct {
    char str[ERRMSG_SIZ+1];
} errmsg_t;


typedef struct {
    char str[MEDIA_NAM_SIZ+1];
} media_t;

typedef struct {
    char str[GROUPNAM_SIZ+1];
} groupname_t;

typedef struct {
    char str[LOCNAM_SIZ+1];
} location_t;

typedef struct {
    char str[OBJ_NAME_SIZ+1];
} objname_t;

typedef struct {
    char str[2];
} state_t;

typedef struct backups_s backups_t;
struct backups_s {
    // bck_id|node|start|end|expiredate|desc
    bckid_t     bck_id;		// backup id of this backup set
    node_t      node;		// backup for node
    blib_tim_t	start;
    blib_tim_t	end;		// 0 or null when backup hasnt completed
    blib_tim_t	expiredate;
    desc_t      desc;
};

typedef struct vol_s vol_t;
struct vol_s {
    // bck_id|label|state|media|usage|groupname|location|librarydate|recorddate|offsitedate|changedate
    bckid_t     bck_id;			// what ever was written onto the tape last
    blabel_t	label;			// the volume label eg ABC123D
    state_t     state;
    media_t     media;
    uint32_t	usage;
    groupname_t	groupname;
    location_t	location;
    blib_tim_t	librarydate;
    blib_tim_t  recorddate;
    blib_tim_t	offsitedate;
    blib_tim_t  changedate;    // State Change date
};

typedef struct bckobj_s	bckobj_t;
struct bckobj_s {
    // bck_id|objname|obj_instance|start|end|size
    bckid_t	    bck_id;
    objname_t	objname;
    objid_t	    obj_instance;
    blib_tim_t	start;
    blib_tim_t	end;
    bcount_t	size;
};

typedef struct vol_obj_s vol_obj_t;	// link table between volumes and backup objects
struct vol_obj_s {
    // bck_id|objname|obj_instance|label|fileno|start|end|size
    bckid_t	    bck_id;		// link bckobj_t.bck_id or backups_t.bck_id
    objname_t	objname;	// link bckobj_t.objname
    objid_t	    obj_instance;
    blabel_t	label;		// link vol_t.label
    uint32_t	fileno;		// the file number on the volume
    blib_tim_t	start;
    blib_tim_t	end;
    bcount_t	size;		// how much written in this segment
};

typedef struct vol_obj_id_s vol_obj_id_t;
struct vol_obj_id_s {
    bckid_t     bck_id;
    objname_t   objname;
    objid_t     obj_instance;
    blabel_t    label;
    int         fileno;
    blib_tim_t  start;
    blib_tim_t  end;
    bcount_t    size;
    bckid_t     bobck_id;
};

typedef struct bck_errors_s bck_errors_t;
struct bck_errors_s {
    // bck_id|label|objname|obj_instance|errtime|errmsg
    bckid_t	    bck_id;
    blabel_t	label;
    objname_t	objname;
    objid_t	    obj_instance;
    blib_tim_t	errtime;
    errmsg_t	errmsg;
};


typedef struct qual_s qual_t;
struct qual_s {
    bckid_t	    bck_id;
    desc_t	    desc;
    blib_tim_t  end;
    int		    errcount;
    errmsg_t    errmsg;
    blib_tim_t  expiredate;
    int		    filecount;
    int		    fileno;
    groupname_t	groupname;
    blabel_t	label;
    blib_tim_t	librarydate;
    location_t	location;
    media_t	    media;
    node_t	    node;
    objname_t	objname;
    objid_t	    obj_instance;
    blib_tim_t	offsitedate;
    blib_tim_t	recorddate;
    bcount_t	size;
    blib_tim_t	start;
    state_t	    state;
    char	    *stylesheet;
    int		    usage;
    blib_tim_t	since;
    blib_tim_t	until;
};

#endif /* __DATA_STRUCTURE_H__ */
