static const char *rcsCALTIME="@(#)$Id: caltime.c,v 1.3 2008/05/13 04:40:25 root Exp $";
/* Calc time functions
 ** $Log: caltime.c,v $
 ** Revision 1.3  2008/05/13 04:40:25  root
 ** add -c to display c time as well
 **
 ** Revision 1.2  2000/09/06 02:48:37  garrettm
 ** add Month Sep :)
 **
 * Revision 1.1  2000/04/07  05:39:54  garrettm
 * Initial revision
 *
 **
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#include <unistd.h>


#ifndef FALSE
#define FALSE	0
#endif
#ifndef TRUE
#define	TRUE !FALSE
#endif



main(argc,argv)
int	argc;
char	*argv[];
{
    int	daydelta;
    time_t	now, newdate;
    struct	tm *newtm;
    int     c, errflg = 0;
    int	    doctime;
    char    *prog_id = argv[0];
    
    
    static	char *sMon[] = {
    "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };
    
    doctime=0;
    
    if ((argc > 1 ) && ((argv[1][0] == '-') && (argv[1][1] == 'c' ))) {
	doctime++;
	argc--;
	argv++;
    }
 
    if ((argc>1 ) && (argv[1])) {
	daydelta= atoi(argv[1]);
    } else {
	errflg++;
    }
    
    if ( errflg ) 
    {
	printf("Usage: [-c] %s days\nreturn VMS delta time format days hence\n-c is optional and will output time as ctime\n", prog_id);
	exit(EINVAL);
    }
    
    
    
    now = time((time_t *) NULL);
    
    newdate =  now + (daydelta * 24 * 60 * 60 );
    
    newtm = localtime(&newdate);
    
    if (doctime) {
	fprintf(stdout, "%d ", newdate);
    } 
    fprintf(stdout,"%02d-%3s-%04d:%02d:%02d:%02d.00\n", 
	    newtm->tm_mday,
	    sMon[newtm->tm_mon],
	    newtm->tm_year+1900,
	    newtm->tm_hour, 
	    newtm->tm_min,
	    newtm->tm_sec );
    
}	
