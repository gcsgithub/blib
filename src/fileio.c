static char *rcsid="@(#) $Id: fileio.c,v 1.2 2010-04-06 04:29:30+10 mark Exp $";
/*
 *  fileio.c
 *  fmtbckrep_xcode
 *
 *  Created by mark on 17/05/2009.
 *  Copyright 2009 Garetech Computer Solutions. All rights reserved.
 *
 * $Log: fileio.c,v $
 * Revision 1.2  2010-04-06 04:29:30+10  mark
 * tweek to error message on malloc failure
 *
 * Revision 1.1  2009/05/20 18:42:47  mark
 * Initial revision
 *
 *
 */
#include "fileio.h"


static char *version()
{
    return(rcsid);
}

/*//////////////////////////////////////////////////////////////////////////////
//////////// FILE IO FUNCTIONS /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////*/
fio_t    *fio_new(size_t    bufsiz)
{
    fio_t	*new;
    
    if ((new = malloc(sizeof(fio_t))) == (fio_t *) NULL ) {
	fprintf(stderr,"#BLIB:  error allocating memory for file struct " __ATLINE__ "\n");
	exit(ENOMEM);
    }
    bzero(new, sizeof(fio_t));
    new->bufsiz = bufsiz;
    if ((new->buf = malloc(bufsiz)) == (char *) NULL ) {
	fprintf(stderr,"#BLIB:  error allocating memory for buffer in " __ATLINE__ "\n");
	exit(ENOMEM);
    }
    bzero(new->buf, bufsiz);
    return(new);
}

void	fio_close_and_free(fio_t **fiop)
{
    fio_t *fio;
    
    if (fiop) {
	fio = *fiop;
	if (fio) {
	    if (!fio->useropenflag) {
		    fio_close(fio);
	    }
	    nzfree(&fio->fnm);
	    nzfree(&fio->open_mode);
	    nzfree(&fio->ext);
	    nzfree(&fio->buf);
	    free(fio);
	    *fiop = (fio_t *) NULL;
	}
    }
}

void	fio_close_and_free_unlink(fio_t **fiop)
{
    fio_t *fio;

    if (fiop) {
	fio = *fiop;
	if ((!fio->useropenflag) && (fio->fnm))  {
	    if (fio->debug == 0) {
		unlink(fio->fnm);
	    } else {
		fprintf(stderr, "#BLIB:  Debugging: %s not deleted\n", fio->fnm);
	    }
	}
	fio_close_and_free(fiop);
    }
}

int fio_close(fio_t *fio)
{
    if (fio) {
	if (!fio->useropenflag) {
	    if (fio->open) {
		if (fclose(fio->fd)) {
			fio->status = errno;
		} else {
		    fio->open = 0;
		}
		
	    }
	    return(fio->status);
	}
	fio->open = 0;	/* mark it closed */
	return(0);
    } else {
	return(ENOENT);
    }
}

int fio_reopen(fio_t *fio, char *open_mode)
{
    if (fio->open_mode) {
	free(fio->open_mode);
    }
    fio_close(fio);
    fio->open_mode = newstr(open_mode);
    fio_open(fio);
    return(fio->status);
}

char *fio_fgets(fio_t *fio)
{
    char    *rval;
    
    bzero(fio->buf, fio->bufsiz);
    rval = fgets(fio->buf, fio->bufsiz, fio->fd);
    if (rval) {
	zapcrlf(fio->buf);
	fio->reads++;
    } else {
	fio->status = errno;
    }
    return(rval);
}

fio_t *fio_open_temp(char *prefix, char *ext, size_t bufsiz)
{
    char    *tmpfnam, *tmpfnam_ext;
    fio_t   *rval;
    
    tmpfnam = tempnam("/tmp", prefix);
    if (ext) {
	tmpfnam_ext = newstr("%s.%s", tmpfnam, ext);
	replace_dynstr(&tmpfnam, tmpfnam_ext);
    }
	
    rval = fio_alloc_open(tmpfnam, NULL, "w", bufsiz);
    return(rval);
}
    
fio_t	*fio_dup(fio_t *fio)
{
    fio_t *rval;
    int   filenum;
    
    rval = (fio_t *) NULL;
    if (!fio) {
	return(rval);
    }
    
    rval               = fio_new(fio->bufsiz);
    rval->debug        = fio->debug;
    rval->open         = fio->open;
    rval->status       = fio->status;
    rval->useropenflag = fio->useropenflag;
    rval->open_mode    = strdupz(fio->open_mode);
    rval->fnm          = strdupz(fio->fnm);
    rval->ext          = strdupz(fio->ext);
    rval->reads        = 0;
    rval->writes       = 0;
    
    if (fio->open) {
	filenum = fileno(fio->fd);
	switch(filenum) {
	    case 0:
		replace_dynstr(&rval->open_mode, newstr("r"));
		replace_dynstr(&rval->fnm, newstr("stdin"));
		break;
	    case 1:
		replace_dynstr(&rval->open_mode, newstr("a"));
		replace_dynstr(&rval->fnm, newstr("stdout"));
		break;
	    case 2:
		replace_dynstr(&rval->open_mode, newstr("a"));
		replace_dynstr(&rval->fnm, newstr("stderr"));
		break;
	    default:
		break;
	}
	if ((rval->open_mode) && (fileno >= 0)) {
    		rval->fd   = fdopen(filenum,rval->open_mode);
	}
    }
    return(rval);
}

fio_t	*fio_fd(FILE *fd)
{
    fio_t	*fio;
    char	*filename;
    char 	*ext;
    
    fio = fio_new(MAX_BUF);    
    ext="";
    switch(fileno(fd)) {
	case 0:
	    filename="stdin";
	    break;
	case 1:
	    filename="stdout";
	    break;
	case 2:
	    filename="stderr";
	    break;
	default:
	    filename="unknown";
	    break;
    }
    fio->ext = newstr(ext);
    fio->fnm = newstr(filename);
    fio->open=1;	// its open
    fio->useropenflag = 1; // not our job to open/close this fd
    fio->fd = fd;
    return(fio);
    
}
    
fio_t    *fio_alloc_open(char *filename, char *new_ext, char *open_mode, size_t bufsiz)
{
    fio_t	*fio;
    
    fio = fio_new(bufsiz);
    
    if (new_ext) {
	fio->ext = newstr(new_ext);
	fio->fnm = replace_ext(filename, new_ext);
    } else {
	fio->ext = newstr("");
	fio->fnm = newstr(filename);
    }
    if (open_mode) {
	fio->open_mode = newstr(open_mode);
    } else {
	fio->open_mode = newstr("r");
    }
    fio_open(fio);
    return(fio);
}

int fio_open(fio_t *fio)
{
    if (!fio->useropenflag) {
	if (fio->open) {
	    fio_close(fio);
	}
	
	if (strcmp(fio->fnm, "stdin")==0) {
	    fio->fd = stdin;
	    fio->open = 1;
	    fio->useropenflag = 1;
	    
	} else if (strcmp(fio->fnm, "stdout")==0) {
	    fio->fd = stdout;
	    fio->open = 1;
	    fio->useropenflag = 1;
	    
	} else if (strcmp(fio->fnm, "stderr")==0) {
	    fio->fd = stderr;
	    fio->open = 1;
	    fio->useropenflag = 1;
	    
	} else {
	    if ((fio->fd = fopen(fio->fnm, fio->open_mode)) == (FILE *) NULL ) {
		fio->status = errno;
		fprintf(stderr,"#BLIB:  Error opening file \"%s\" mode: \"%s\" %d:%s\n", fio->fnm, fio->open_mode, fio->status, strerror(fio->status));
	    } else {
		fio->open = 1;	/* make it open */
		fio->reads = fio->writes = 0;
	    }
	}
    } else {
	fio->open = 1;	/* make it open */
	fio->reads = fio->writes = 0;
    }
	
    return(fio->status);
}    


char *replace_ext(char *base, char *new_ext)
{
    /* given a base name strip off any .old_ext and return a newstr() malloc'd with the new_ext */
    char *fnamstr;
    size_t  fnmlen;
    char *dotptr;
    
    fnmlen = strlen(base) + strlen(new_ext) + 1;
    
    if ((fnamstr = (char *) malloc(fnmlen)) == (char *) NULL ) {
	fprintf(stderr,"#BLIB:  Error allocating memory for file name string\n");
	exit(ENOMEM);
    }
    strcpy(fnamstr, base);
    if ( (dotptr = rindex(fnamstr, '.')) ) *dotptr = '\0';
    strcat(fnamstr, new_ext);
    return(fnamstr);
}

char	    *fio_basename(fio_t *fio)
{ // return a pointer to the base file name since we dont need to modify fio->fnm we will just return the pointer or ""
    char *basefnm;
    basefnm = (char *) NULL;
    if (fio) {
	if (fio->fnm) {
	    basefnm = rindex(fio->fnm, '/');
	    
	}
    }
    if (basefnm) {
	basefnm++; // skip to the 1st char after the slash
    } else {
	basefnm="";
    }
    return(basefnm);
}


int fio_rewind(fio_t *fio)
{
    int rval = EBADF;
    int   old_errno;
    
    if (fio) {
	old_errno=errno;
	rewind(fio->fd);
	if (old_errno != errno) {
		rval = fio->status = errno;
    	}
    }
    return(rval);
}

int  fio_copy_file(fio_t *src, fio_t *outfd)
{
    fio_rewind(src);
    fio_fgets(src);
    while(!feof(src->fd)) {
	fprintf(outfd->fd, "%s\n", src->buf);
        fio_fgets(src);
    }
    fio_close(src);
    fio_close(outfd);
    return(errno);
}
