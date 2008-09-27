static char *rcsid="@(#) $Id:$";
/*
 * $Log:$
 *
 */
#include "blib.h"


static blib_global_t BLIB;

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
    DBM		    *dbmf;
    mode_t	    saved_umask;
    
    (void)setlocale(LC_ALL, "");
    setup_blib(&BLIB);
   
    optarg = NULL;
    while (!dousage && ((c = getopt(argc, argv, "v?")) != -1)) {
        switch (c) {
            case 'v':
                dousage++;
                break;
            default:
                dousage++;
                break;
        }
    }
    
    argc -= optind;
    argv += optind;

    if (dousage) usage(rcsid+8); // will exit via usage
    if ((cmds = parseslashcmd(argc,argv)) == (cmd_t *) NULL ) usage(rcsid+8);
    
#ifdef DEBUG_ONMAC

#endif /* DEBUG_ONMAC */
    
    

    
    saved_umask = umask(007);
    
    dbmf = dbm_open("/tmp/test",O_RDWR|O_CREAT|O_EXLOCK, 0660);
    if ( dbmf == (DBM *) NULL ) {
	err =errno;
	fprintf(stderr, "Error opening dbm file %d:%s\n", err, strerror(err));
	saved_umask = umask(saved_umask);
	exit(err);
    }
    dbm_close(dbmf);
    saved_umask = umask(saved_umask);
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
	snprintf(blibgrpstr, sizeof(blibgrpstr),"%sTL1", this_host);
	sp = blibgrpstr;
    }
    blib_gp->blib_group = newstr(sp);
 
//============================================================================   
    if ((sp=getenv("BLIBDB")) == NL ) {
	snprintf(dbnamestr, sizeof(dbnamestr),"/usr/local/etc/dat/blib_%s", blib_gp->blib_group );
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
//============================================================================   
    
}

	
