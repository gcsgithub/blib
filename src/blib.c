static char *rcsid="@(#) $Id: blib.c,v 1.5 2013/01/20 10:37:18 mark Exp mark $";
/*
 * $Log: blib.c,v $
 * Revision 1.5  2013/01/20 10:37:18  mark
 * mg remove unused variables
 *
 * Revision 1.4  2011/04/11 03:49:50  mark
 * generally fix OSrval's, fix records being added with invalid bck_id, add /verify
 *
 * Revision 1.3  2010/11/16 04:10:03  mark
 * rc1
 *
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

    
    dbh_t	    *execdb;
    char	    *cmdline;
    
    (void)setlocale(LC_ALL, "");
    BLIB.progid = newstr(argv[0]);
    setup_defaults();
    
    optarg = NULL;
    while (!dousage && ((c = getopt(argc, argv, "qdvVf:w:h?")) != -1)) {
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
            case 'w':
                BLIB.date_width = atoi(optarg);
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
    if (BLIB.date_width == 0) {
        BLIB.date_width = 20; // default date width for display  20-Apr-2013:11:39:12
    }
    else if (BLIB.date_width > 23) {
        fprintf(stderr, "# a maximum date width is 23 defaulting to that as %u is >\n", BLIB.date_width);
        BLIB.date_width = 23;
    }
    else if (BLIB.date_width < 12) {
        fprintf(stderr, "# a minimum date length of 11 is required to show dd-Mmm-YYYY %u is too small\n", BLIB.date_width);
        BLIB.date_width = 11;
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


