#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

static char *rcsCALTIME="@(#)$Id: caltime.c,v 1.1 2000/04/07 05:39:54 garrettm Exp garrettm $";

/* Calc time functions
** $Log: caltime.c,v $
 * Revision 1.1  2000/04/07  05:39:54  garrettm
 * Initial revision
 *
**
*/

main(argc,argv)
int	argc;
char	*argv[];
{
	int	plusdays;
	time_t	now, newdate;
	struct	tm *newtm;

	static	char *sMon[] = {
	"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };

	if ( argc < 2 ) {
		printf("Usage: %s days\nreturn VMS delta time format days hence\n", argv[0]);
		exit(22);
	}
	plusdays= atoi(argv[1]);
	
	now = time((time_t *) NULL);

	newdate =  now + (plusdays * 24 * 60 * 60 );

	newtm = localtime(&newdate);

	fprintf(stdout,"%02d-%3s-%04d:%02d:%02d:%02d.00\n", 
		newtm->tm_mday,
		sMon[newtm->tm_mon],
		newtm->tm_year+1900,
		newtm->tm_hour, 
		newtm->tm_min,
		newtm->tm_sec );
}	
