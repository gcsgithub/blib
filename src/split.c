static char *rcsid="@(#) $Id: split.c,v 1.2 2010/11/16 04:10:23 mark Exp mark $";
/*
 * $Log: split.c,v $
 * Revision 1.2  2010/11/16 04:10:23  mark
 * rc1
 *
 * Revision 1.1  2008/10/19  22:18:59  root
 * Initial revision
 *
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

int split(char *buf, char *sep,list_t **flds)
{
    int		rval, eol;
    char        *sp, qchar, *fldptr, *fld;
    entry_t	*ent;
    
    qchar ='\0';
    sp = buf;
    eol= FALSE;
    rval = SPLIT_OK;
    fldptr = sp;
    while ( !eol ) {
        switch(*sp) {
            case '\"':
                if ( qchar == '\"' ) { // this must be a closing quote
                    qchar = '\0'; // stop quote
                } else { // not already in a quote
                    qchar = '\"';	 // set quote char
                }
                break;
                
            case '\'':
                if ( qchar == '\"' ) { // this must be a closing quote
                    qchar = '\0'; /* no longer quote */
                } else { // not already in a quote
                    qchar = '\"';	 // set quote char
                }
                break;
                
            case '\n':
            case '\r':
                *sp='\0'; /* null it and fall through */
            case '\0':
                if ( qchar != '\0' ) { // have a start of quoted text but found eol before closing
                    rval=SPLIT_MISSQUOT;
                }
                fld = newfld(fldptr);
                ent = new_entry(fld);
                list_insert_tail(flds, ent);
                eol = TRUE; // and we done
                break;
                
            default: /* anything else must be start of non quoted field */
                if ( qchar == '\0' ) { // if not quoting then we can look for sep
                    if (index(sep,*sp) != (char *) NULL ) {
                        *sp='\0';	// null terminate it as a C String
                        fld = newfld(fldptr);
                        ent = new_entry(fld);
                        list_insert_tail(flds, ent);
                        
                        fldptr = (sp+1); // set a point to the start
                    }
                }
                break;
        }
        sp++; /* next char */
    }
    return(rval);
}

char *newfld(char *fldptr)
{
    char *rval;
    
    if ( (rval = strdup(rtrim(fldptr, strlen(fldptr)))) == (char *) NULL ) {
        fprintf(stderr, "#BLIB:  outof memory in " __ATLINE__ "\n");
    }
    return(rval);
}
