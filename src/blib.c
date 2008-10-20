static char *rcsid="@(#) $Id: blib.c,v 1.1 2008/09/27 13:05:59 mark Exp $";
/*
 * $Log: blib.c,v $
 * Revision 1.1  2008/09/27 13:05:59  mark
 * Initial revision
 *
 *
 */
#include "blib.h"


blib_global_t BLIB;

void    usage(const char *prg)
{
    fprintf(stderr,"Usage: %s [/cmd [/arg] \n", prg);
    exit(EINVAL);
}   

int main(int argc,  char *argv[])
{   
    int             err;
    int             dousage=0;
    char            c;
    extern char     *optarg;
    extern int      optind; 
    extern int      optopt;
    extern int      opterr;
    extern int      optreset; 
    cmd_t	    *cmds;
    
    
    
#ifdef DEBUG_ONMAC
    // check why /report /add=obh0123D didnt issue an error?
    argc=2;
    // argv[1] = "/display=tstldate1";
    // argv[1] = "/remove=tstldate1";

    
    argc=3;
    argv[1] ="/import=/Users/mark/blib_prod.dat";
    argv[2] ="/new";
    //argv[1] = "/modify=OBG123D";
    // argv[2] = "/state=FREE";
    // argv[2] = "/media=TK50";
    //argv[2] = "/incfileno";
    

#endif /* DEBUG_ONMAC */
    
    (void)setlocale(LC_ALL, "");
    setup_blib(&BLIB);
    BLIB.progid = newstr(argv[0]);
   
    optarg = NULL;
    while (!dousage && ((c = getopt(argc, argv, "qdvVf:?")) != -1)) {
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
                fprintf(stderr, "Version: %s\n", rcsid+8);
		exit(0);
                break;
	    case 'f':
		nzfree(BLIB.blibdb_name);
		BLIB.blibdb_name = newstr(optarg);
		break;
            default:
                dousage++;
                break;
        }
    }
    
    argc -= (optind-1);
    argv += (optind-1);

    if (dousage) usage(BLIB.progid); // will exit via usage
    if ((cmds = parseslashcmd(argc,argv)) == (cmd_t *) NULL ) usage(BLIB.progid);
    if (BLIB.debug ) dump_cmd(cmds);
    
    err = execute_cmds(cmds);
    exit(err);
}


void setup_blib(blib_global_t *blib_gp)
{
    char    *sp;
    char    *this_host;
    char    dbnamestr[24+MAXHOSTNAMELEN+1];
    char    blibgrpstr[MAXHOSTNAMELEN+1];
    char    library_namestr[MAXHOSTNAMELEN+3+1];
    
    
    this_host = get_hostname(NL);

//============================================================================   
        if ((sp=getenv("MYBLIB_GROUP")) == NL ) {
	snprintf(blibgrpstr, sizeof(blibgrpstr),"%s", this_host);
	sp = blibgrpstr;
    }
    blib_gp->blib_group = newstr(sp);
 
//============================================================================   
    if ((sp=getenv("BLIBDBS")) == NL ) {
	snprintf(dbnamestr, sizeof(dbnamestr),"/usr/local/etc/dat/blib_%s.sqlite3", blib_gp->blib_group );
	sp = dbnamestr;
    }
    blib_gp->blibdb_name = newstr(sp);
    
//============================================================================   
    if ((sp=getenv("DAILYMEDIA")) == NL ) {
	sp = "TZ89";
    }
    blib_gp->default_media = newstr(sp);
//============================================================================       
    if ((sp=getenv("TAPELIB")) == NL ) {
	snprintf(library_namestr, sizeof(library_namestr),"%sTL1", blib_gp->blib_group);
	sp = library_namestr;
    }
    blib_gp->library_name = newstr(sp);
    
}

	
