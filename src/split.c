static char *rcsid="@(#) $Id:$";
/*
 * $Log:$
 *
 *  split.c
 *  blib
 *
 *  Created by mark on 09/10/2008.
 *  Copyright 2008 Garetech Computer Solutions. All rights reserved.
 *
 */

#include "split.h"


static char *ver()
{
    return(rcsid);
}

//  fred|"mark|garrett"|
int split(char *buf, char *sep,list_t **flds)
{    
    int		rval, eol;
    char        *sp, qchar, *fldptr, *fld;
    entry_t	*ent;
    
    qchar ='\0';
    sp = buf; 
    eol= FALSE;
    // rval == SPLIT_OK;
    fldptr = sp;
    while ( !eol ) {
        switch(*sp) {
            case '\"':
                if ((qchar == '\0') || ( qchar == '\"'  )) { // this must be a closing quote
		    *sp='\0';	// null terminate it as a C String
		    if ( (fld = strdup(rtrim(fldptr, strlen(fldptr)))) == (char *) NULL ) { /* error duplicating string */
			rval = SPLIT_ENOMEM;
			eol = TRUE; // force exit
		    } else {
			ent = new_entry(fld);
			list_insert(flds, ent);
			qchar = '\0'; /* no longer quote */
			
		    }
		} else { // not already in a quote
		    qchar = '\"';	 // set quote char
		}
		fldptr = (sp+1); // set a point to the start
                break;
		
            case '\'':
                if ((qchar == '\0') || ( qchar == '\"'  )) { // this must be a closing quote
		    *sp='\0';	// null terminate it as a C String
		    if ( (fld = strdup(rtrim(fldptr, strlen(fldptr)))) == (char *) NULL ) { /* error duplicating string */
			rval = SPLIT_ENOMEM;
			eol = TRUE; // force exit
		    } else {
			ent = new_entry(fld);
			list_insert(flds, ent);
			qchar = '\0'; /* no longer quote */
		    }
		} else { // not already in a quote
		    qchar = '\"';	 // set quote char
		}
		fldptr = (sp+1); // set a point to the start
                break;
		
	    case '\n':
	    case '\r':
		*sp='\0'; /* null it and fall through */
            case '\0': 
                if ( qchar != '\0' ) { // have a start of quoted text but found eol before closing
			rval=SPLIT_MISSQUOT;
		}
		if ( (fld = strdup(rtrim(fldptr, strlen(fldptr)))) == (char *) NULL ) {
                        rval=SPLIT_ENOMEM;
		} else {
			ent = new_entry(fld);
			list_insert(flds, ent);
		}
		eol = TRUE; // and we done
		break;
		
	    default: /* anything else must be start of non quoted field */
		 if ( qchar == '\0' ) { // if not quoting then we can look for sep
			if (index(sep,*sp) != (char *) NULL ) { 
			    *sp='\0';	// null terminate it as a C String
			    if ( (fld = strdup(rtrim(fldptr, strlen(fldptr)))) == (char *) NULL ) { /* error duplicating string */
				rval = SPLIT_ENOMEM;
				eol = TRUE; // force exit
			    } else {
				ent = new_entry(fld);
				list_insert(flds, ent);
			    }
			    fldptr = (sp+1); // set a point to the start
			} 
		}
		break;
        }
        sp++; /* next char */
    }
    return(rval);
}
