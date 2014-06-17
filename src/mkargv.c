static const char *rcsid="@(#) $Id: mkargv.c,v 1.1 2010/11/16 04:04:59 root Exp $";
/*
 *  mkargv.c
 *  blib
 *
 *  Created by mark on 26/10/2010.
 *  Copyright 2010 Garetech Computer Solutions. All rights reserved.
 *
 *  $Log: mkargv.c,v $
 *  Revision 1.1  2010/11/16 04:04:59  root
 *  Initial revision
 *
 *
 */

#include "mkargv.h"
#include "list.h"

static const char *version(void)
{
	return(rcsid);
}


int mkargv(char ***argv, char *line)
{
    int argc = 0;
    int	sts;
    int idx;
    list_t *flds = (list_t *) NULL;
    entry_t *fldsptr;
    char **args;
    ssize_t	argsize;
    
    *argv = (char **) NULL;
    mksinglespaced(line, '\t');
    sts = split(line, "\t", &flds);
    if (sts == SPLIT_OK ) {
	argc = flds->items;
	argsize = (argc+2)*sizeof(char *);
	*argv = args = (char **) malloc(argsize);
	if (args == (char **) NULL ) {
	    fprintf(stderr, "#BLIB:  Error allocating memor in " __PRETTY_FUNCTION__ "\n");
	    exit(ENOMEM);
	}
	bzero(args,argsize );
	
	fldsptr=flds->head;
	
	args[0] = strdup("blib");
	idx=1;
	while( fldsptr ) {
	    args[idx] = (char *) fldsptr->e;
	    idx++;
	    fldsptr=fldsptr->np;
	}
    }
    if (flds) free_list(&flds, NULL);
    return(argc);
}


ssize_t mksinglespaced(char *line, char sep)
{
    // reduce none quoted duplicate white space single char sep
    
    int		rval, eol;
    char        *sp;
    char	qchar;
    char	c;
    enum { NO, YES } inspace;
    
    qchar ='\0';
    sp = line; 
    eol= FALSE;
    rval = 0;
    inspace = NO;
    ltrim(line, strlen(line));
    while ( !eol ) {
	c = *sp;
        switch(c) {
            case '\"':
                if ( qchar == '\"'  ) { // this must be a closing quote
		    qchar = '\0'; /* no longer quote */
		} else { // start quote
		    qchar = '\"';	 // set quote char
		}
                break;
		
            case '\'':
                if ( qchar == '\"' ) { // this must be a closing quote
		    qchar = '\0'; /* no longer quote */
		} else { // start quote
		    qchar = '\"';	 // set quote char
		}
                break;
		
	    case '\n':
	    case '\r':
		*sp='\0'; /* null it and fall through */
            case '\0': 
                if ( qchar != '\0' ) { // have a start of quoted text but found eol before closing
		    rval=-1;
		}
		eol = TRUE; // and we done
		break;
		
	    default: /* anything else must be start of non quoted field */
		if ( qchar == '\0' ) { // if not quoting then we can look for white space
		    if (isspace(c)) {
			*sp = sep;
			if (inspace == YES ) {
			    strcpy(sp,sp+1); // move rest of string over the space
			} else {
			    inspace = YES;
			}
		    } else {
			inspace = NO;
		    }
		}
		break;
        }
        sp++; /* next char */
    }
    return(rval);    
}
