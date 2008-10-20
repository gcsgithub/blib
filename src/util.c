static char *rcsid="@(#) $Id: util.c,v 1.2 2008/09/27 13:11:22 mark Exp mark $";
/*
 *  $Log: util.c,v $
 *  Revision 1.2  2008/09/27 13:11:22  mark
 *  initial checkin
 *
 *
 *  util.c
 *  blib
 *
 *  Created by mark on 26/09/2008.
 *  Copyright 2008 Garetech Computer Solutions. All rights reserved.
 *
 */
static char *ver()
{
    return(rcsid);
}

#include "util.h"
typedef struct tm tm_t;

char    *zapcrlf(char *bp)
{
    char *sp;
    
    if ((sp=rindex(bp,'\r')) != (char *) NULL ) *sp = '\0'; // zap carrage return
    if ((sp=rindex(bp,'\n')) != (char *) NULL ) *sp = '\0'; // zap newline
    return(bp);
}

char    *newstr(char *fmt,...)
{
    va_list args;
    char    *dynstr;
    size_t  slen;
    
    if (!fmt) {
	fmt="";	// just in case they pass a null pointer
    }
    va_start(args,fmt);
    slen = VASPRINTF(&dynstr, fmt,args);
    va_end(args);
    if ( dynstr  == (char *) NULL ) {
        fprintf(stderr,"Out of memory allocating string %s\n", fmt);
        errno = ENOMEM;
        exit(errno);
    }
    return(dynstr);
}


int *newintstr(char *val)
{
    return(newint(atoi(val)));
    
}

int *newint(int val)
{
    int    *dynstr;
    
    if ((dynstr = (int *) malloc(sizeof(int))) == (int *) NULL ) {
        fprintf(stderr,"Out of memory allocating for int %s\n", val);
        errno = ENOMEM;
        exit(errno);
    }
    *dynstr = val;
    return(dynstr);
}


time_t	*newdate(char *val)
{
    time_t  *dynstr;
    
    if (( dynstr = (time_t *) malloc(sizeof(time_t))) == (time_t *) NULL ) {
	fprintf(stderr,"Out of memory allocating int %s\n", val);
	errno = ENOMEM;
	exit(errno);
    }
    
    *(time_t *) dynstr = scandate(val);
    return(dynstr);
}

time_t	scandate(char *datestr)
{
    tm_t    timetm;
    time_t  timec;
    int	err;
    
    timec=0;
    if (datestr && datestr[0] ) {
	bzero(&timetm, sizeof(tm_t));
	if ((strncasecmp(datestr, D_NEVER,sizeof(D_NEVER)) != 0 ) &&
	    (strncasecmp(datestr, D_VMS0, sizeof(D_VMS0)) !=0)) {
	    bzero(&timetm,sizeof(tm_t));
	    strptime(datestr, "%d-%b-%Y:%T", &timetm); 
	    if ( ( timec = mktime( &timetm)) == -1 ) {
		err = errno;
		if ( err != ERANGE ) {
		    fprintf(stderr,"Error in parsing date %s :%s\n", datestr, strerror(err));
		    errno=err;
		    timec=0;
		}
	    }
	}
    }
    return(timec);
}


char	*get_hostname(char *defhostname) 
{
    int	    err;
    char    hostnamestr[MAXHOSTNAMELEN+2];
    char    *thishost = hostnamestr;
    char    c, *cp;
    
    bzero(hostnamestr,sizeof(hostnamestr));
    
    if ( gethostname(thishost,sizeof(hostnamestr)-1) ==  -1 ) {
	err=errno;
	fprintf(stderr,"error gethostname==%d:%s\n", err,strerror(err));
	if (defhostname )	thishost = defhostname;
	else			thishost = "Amneziac";
    }
    cp=thishost;
    while (*cp) {
	c=toupper(*cp);
	if (c=='.' ) c='\0';
	*cp++ = c;
    }
    return(newstr(thishost));
}

char	*skipwspace(char *str)
{
    while (str && *str && isspace(*str)) str++;
    return(str);
}


char *fmtctime(time_t ctime)
{
    tm_t      timetm;
    static  char timestr[64];
    
    //  if (ctime == 0 )
    // snprintf(timestr,sizeof(timestr), "Never");
    bzero(&timetm,sizeof(tm_t));
    
    localtime_r(&ctime,&timetm);
    if ( strftime(timestr,sizeof(timestr),"%d-%b-%Y:%T.00",&timetm ) == (size_t) 0) {
        return(NULL);
    }
    return ( timestr );
}

time_t	now(void)
{      
    return( time((time_t*)0 ));
}

void    nzfree(void *ptr)
{
    if (ptr) free(ptr);
}

int replace_dynstr(char **dynptr, char *newval)
{
    if (!dynptr) return(0);	// passed in null give them what they deserve (null)
    nzfree((char *) *dynptr);	// toss away any previous malloc'd value
    *dynptr = newval;		// put in the new value
    return(strlen(newval));	// return new value length
}

char *ltrim(char *str,int len)
{
    /* check str of len bytes make the first white space a null */
    
    char    *strend, *sp;
    
    strend = str+len;
    sp = str;
    
    while (sp && *sp && (sp<=strend)  && isspace(*sp) ) sp++;
    
    if (sp <= strend ) {
	strncpy(str,sp,(strend-sp+1));
    }
    
    return(str);
}

char *rtrim(char *str,int len)
{
    /* check str of len bytes work back from the end str+len backward while whitespace */
    char    *strend;
    
    strend = str+len-1;
    
    while (strend &&  (strend >= str)  && isspace(*strend) ) strend--;
    
    strend++;
    if ( strend >= str )
	*strend = '\0';
    
    return(str);
}

char	invalidchars(char *str, const char *validchars)
{ // return invalid char in str or null if we all ok
    char    *sp, *match;
    char    c;
    
    if (!str) {
	return(0xff);  // null pointer you idiot
    }
    sp =str;
    
    while(c=*sp) {
	if ((match=index(validchars, c)) == (char *) NULL ) {
	    return(c);
	}
	sp++;
    }
    return(c);
}
