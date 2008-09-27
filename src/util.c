/*
 *  util.c
 *  blib
 *
 *  Created by mark on 26/09/2008.
 *  Copyright 2008 Garetech Computer Solutions. All rights reserved.
 *
 */

#include "util.h"
typedef struct tm tm_t;

char    *zapcrlf(char *bp)
{
    char *sp;
    
    if ((sp=rindex(bp,'\r')) != (char *) NULL ) *sp = '\0'; // zap carrage return
    if ((sp=rindex(bp,'\n')) != (char *) NULL ) *sp = '\0'; // zap newline
    return(bp);
}

char    *newstr(char *str)
{
    char    *dynstr;
    
    if (( dynstr = (char *) malloc(strlen(str)+1 )) == (char *) NULL ) {
        fprintf(stderr,"Out of memory allocating string %s\n", str);
        errno = ENOMEM;
        exit(errno);
    }
    strcpy(dynstr, str);
    return(dynstr);
}


int	*newint(char *val)
{
    int    *dynstr;
    
    if ((dynstr = (int *) malloc(sizeof(int))) == (int *) NULL ) {
        fprintf(stderr,"Out of memory allocating int %s\n", val);
        errno = ENOMEM;
        exit(errno);
    }
    *dynstr = atoi(val);
    return(dynstr);
}
	
time_t	*newdate(char *val)
{
    time_t  *dynstr;
    tm_t    timetm;
    time_t  timec;
    int err;
	
	if (( dynstr = (time_t *) malloc(sizeof(time_t))) == (time_t *) NULL ) {
	    fprintf(stderr,"Out of memory allocating int %s\n", val);
	    errno = ENOMEM;
	    exit(errno);
	}
    
    bzero(&timetm,sizeof(tm_t));
    strptime(val, "%d-%b-%Y:%T", &timetm);
    if ( ( timec = mktime( &timetm)) == -1 ) {
        if ( ( err = errno) != ERANGE ) {
            fprintf(stderr,"Error in parsing date %s :%s\n", val, strerror(errno));
            errno=err;
            return(0);
        }
    }
    
    *(time_t *) dynstr = timec;
    return(dynstr);
}
	    

char	*get_hostname(char *defhostname) 
{
    int	    err;
    char    hostnamestr[MAXHOSTNAMELEN+1];
    char    *thishost = hostnamestr;
    
    bzero(hostnamestr,sizeof(hostnamestr));
    
    if ( gethostname(thishost,sizeof(hostnamestr)) ==  -1 ) {
	err=errno;
	fprintf(stderr,"error gethostname==%d:%s\n", err,strerror(err));
	if (defhostname )	thishost = defhostname;
	else		thishost = "Amneziac";
    }
    return(newstr(thishost));
}

char	*skipwspace(char *str)
{
    while (str && *str && isspace(*str)) str++;
    return(str);
}