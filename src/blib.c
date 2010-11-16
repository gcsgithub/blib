static char *rcsid="@(#) $Id: blib.c,v 1.2 2008/10/20 13:01:35 mark Exp mark $";
/*
 * $Log: blib.c,v $
 * Revision 1.2  2008/10/20  13:01:35  mark
 * checkpoint
 *
 * Revision 1.1  2008/09/27 13:05:59  mark
 * Initial revision
 *
 *
 */
#include "blib.h"
#include "parseslashcmd.h"
#include "data_access.h"
#include "execute_cmds.h"


blib_global_t BLIB;

void    usage(const char *prg)
{
    fprintf(stderr, "#BLIB:  Backup LIBrary (blib) %s\n", COPYRIGHT);
    fprintf(stderr,"#BLIB:  Usage: %s\t[/cmd [/arg] \n", prg);
    do_cmd_help(stderr);
    exit(EINVAL);
}   

int main(int argc,  char *argv[] /* , char *envp[] */)
{   
    int             err;
    int             dousage=0;
    char            c;
    extern char     *optarg;
    extern int      optind; 
    extern int      optopt;
    extern int      opterr;
    extern int      optreset; 
    
    dbh_t	    *execdb;
    char	    *cmdline;
    
    (void)setlocale(LC_ALL, "");
    BLIB.progid = newstr(argv[0]);
    setup_defaults();
   
    optarg = NULL;
    while (!dousage && ((c = getopt(argc, argv, "qdvVf:h?")) != -1)) {
        switch (c) {
	    case 'q':
		BLIB.quiet++;
		break;
	    case 'd':
		BLIB.debug++;
		break;
	    case 'v':
		BLIB.verbose++;
		break;
            case 'V':
                fprintf(stderr, "#BLIB:  Version: %s\n", rcsid+8);
		exit(0);
                break;
	    case 'f':
		set_default(QUAL_DATABASE, optarg); // not sure why you would but /database will over rule -f
		break;
	    case 'h':
		usage(BLIB.progid);
		exit(0);
		break;
            default:
                dousage++;
                break;
        }
    }
    
    argc -= (optind-1);
    argv += (optind-1);
    
    if (argc<=1) {
	usage(BLIB.progid);
	exit(0);
    }

    if (dousage) usage(BLIB.progid); // will exit via usage
 
    cmdline = mkcmdline(argc, argv);
    // cmdline = newstr("/newbackup '/desc=New backup test 1' /record=07-Nov-2010:00:57:20.00 /expire=14-Nov-2010:00:57:20.00");
    // cmdline = newstr("/newbackup '/desc=New backup test 1' /record=07-Nov-2010:06:26:32.00 /expire=14-Nov-2010:06:26:32.00");

    execdb = process_command_line(NULL, cmdline); // NULL because we not open yet
    err = 0;
    if (execdb) {
	err = execdb->status;
	if (err) {
	    if (strcasecmp(execdb->errmsg, "not an error")) {
		if (BLIB.debug) fprintf(stderr, "#BLIB:  status: %d ", err);
		if (execdb->errmsg ) fputs(execdb->errmsg, stderr);
		if ((BLIB.debug) || (execdb->errmsg )) fputc('\n', stderr);
	    }
	}
    	if (execdb->open == YES ) {
		db_close(execdb);
	}
    }
    if ((err == BLIBDB_DONE) || (err == BLIBDB_ROW) || (err == BLIBDB_OK)) {
	exit(0);
    } else {
   	 exit(err);
    }
}


