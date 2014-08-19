static const char *rcsid="@(#) $Id: util.c,v 1.5 2013/01/20 10:36:28 mark Exp mark $";
/*
 *  $Log: util.c,v $
 *  Revision 1.5  2013/01/20 10:36:28  mark
 *  MG :( just spacing changes
 *
 *  Revision 1.4  2010/11/16 04:10:27  mark
 *  rc1
 *
 * Revision 1.3  2008/10/20  13:01:39  mark
 * checkpoint
 *
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
static const char *ver()
{
    return(rcsid);
}

#include "util.h"

void dodbg(void)
{
    
}

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
        fprintf(stderr,"#BLIB:  Out of memory allocating string %s size %llu\n", fmt,(llu_t) slen);
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
        fprintf(stderr,"Out of memory allocating for int %d\n",  val);
        errno = ENOMEM;
        exit(errno);
    }
    *dynstr = val;
    return(dynstr);
}

uint64_t *newuint64str(char *val)
{
    return(newuint64(atoll(val)));
    
}

uint64_t *newuint64(uint64_t val)
{
    uint64_t    *dynstr;
    
    if ((dynstr = (uint64_t *) malloc(sizeof(uint64_t))) == (uint64_t *) NULL ) {
        fprintf(stderr,"Out of memory allocating for int %lld\n",  val);
        errno = ENOMEM;
        exit(errno);
    }
    *dynstr = val;
    return(dynstr);
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

void    nzfree(char **p2ptr)
{
    void *ptr = NULL;
    
    if (p2ptr) {
        ptr = *p2ptr;
        if (ptr) {
            free(ptr);
        }
        *p2ptr = NULL;
    }
}

int replace_dynstr(char **dynptr, char *newval)
{
    if (!dynptr) return(0);	// passed in null give them what they deserve (null)
    nzfree(dynptr);		// toss away any previous malloc'd value
    *dynptr = newval;		// put in the new value
    return(strlen(newval));	// return new value length
}

char *ltrim(char *str,int len)
{
    /* check str of len bytes remove leading white space */
    
    char    *strend, *sp;
    
    if (str) {
        strend = str+len;
        sp = str;
        
        while (sp && *sp && (sp<=strend)  && isspace(*sp) ) sp++;
        
        if (sp <= strend ) {
            strncpy(str,sp,(strend-sp+1));
        }
    }
    return(str);
}

char *rtrim(char *str,int len)
{
    /* check str of len bytes work back from the end str+len backward while whitespace */
    char    *strend;
    
    if (str) {
        strend = str+len-1;
        
        while (strend &&  (strend >= str)  && isspace(*strend) ) strend--;
        
        strend++;
        if ( strend >= str )
            *strend = '\0';
    }
    return(str);
}

char	invalidchars(char *str, const char *validchars)
{ // return invalid char in str or null if we all ok
    char    *sp;
    char    c;
    
    if (!str) {
        return(0xff);  // null pointer you idiot
    }
    sp =str;
    
    while((c=*sp)) {
        if ((index(validchars, c)) == (char *) NULL ) {
            return(c);
        }
        sp++;
    }
    return(c);
}


char	*cvt2uppercase(char *str)
{
    char    *sp;
    
    if (str) {
        sp=str;
        while(*sp) {
            *sp = toupper(*sp);
            sp++;
        }
    }
    
    return(str);
}

char *pstr(char *str, char *def)
{ // Protected string ie check if null and return "" if it is
    if (str) {
        return(str);
    } else {
        if (def) {
            return(def);
        } else {
            return("");
        }
    }
}

int	safe_inc_int(int *iptr)
{
    int ival;
    
    ival = 0;
    if (iptr) {
        ival = *iptr;
        ival++;
        if (ival == 0 ) {
            ival = 1;	// looped around
        }
        *iptr=ival;
    }
    return(ival);
}

char	*shrink_string_by_middle(char *dst, int dstlen, char *src)
{
    size_t	srclen;
    size_t	partlen;
    srclen = strlen(src);
    char	*part1;
    char	*part2;
    
    partlen = dstlen/2 - 1; // how many char from start and end do we use;
    
    if (srclen > dstlen) {
        part1 = src;
        part2 = src+(srclen-partlen);
        snprintf(dst, dstlen, "%*.*s..%s", (int) partlen, (int) partlen, part1, part2 );
    } else {
        snprintf(dst, dstlen,"%s",  src);
    }
    return(dst);
}


char *strdupz(char *str)
{
    char *rval = (char *) NULL;
    
    if (str) {
        rval = strdup(str);
        if (rval == (char *) NULL) {
            fprintf(stderr, "#BLIB:  Error allocating memory " __ATLINE__ "\n");
            exit(ENOMEM);
        }
    }
    return(rval);
}
