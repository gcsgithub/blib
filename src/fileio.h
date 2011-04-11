#ifndef __FILEIO_H__
#define __FILEIO_H__
/*
 * @(#) $Id: fileio.h,v 1.1 2010/11/16 04:11:09 mark Exp mark $
 *
 *  fileio.h
 *  fmtbckrep_xcode
 *
 *  Created by mark on 17/05/2009.
 *  Copyright 2009 Garetech Computer Solutions. All rights reserved.
 *
 * $Log: fileio.h,v $
 * Revision 1.1  2010/11/16 04:11:09  mark
 * Initial revision
 *
 * Revision 1.1  2009/05/20 18:42:48  mark
 * Initial revision
 *
 *
 */
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <stdarg.h>
#include "util.h"

#define	MAX_BUF	16384
typedef struct fio_s fio_t;
struct fio_s {
    int	    debug;
    int	    open;
    int	    status;
    FILE    *fd;
    int	    useropenflag;
    char    *open_mode;
    char    *fnm;
    char    *ext;
    char    *mimetype;
    char    *charset;
    char    *encoding;
    size_t  bufsiz;
    char    *buf;
    size_t  reads;
    size_t  writes;
};

typedef struct files_s files_t;
struct files_s {
    fio_t	*fio;
    files_t	*next;
};

files_t *new_files(files_t **head, char *fnm);
files_t	*files_insert_head(files_t **head, fio_t *fio);
files_t *alloc_files_ent(void);

int	fio_open(fio_t *fio);
int	fio_reopen(fio_t *fio, char *open_mode);
char	*fio_fgets(fio_t *fio);
fio_t	*fio_open_temp(char *prefix, char *ext, size_t bufsiz);
void	fio_close_and_free_unlink(fio_t **fio);

fio_t	*fio_new(size_t bufsiz);
void	fio_close_and_free(fio_t **fiop);
fio_t	*fio_alloc_open(char *base, char *new_ext, char *open_mode, size_t bufsiz);
int	fio_close(fio_t *fio);
char	*replace_ext(char *base, char *new_ext);
char	*fio_basename(fio_t *fio);
int	fio_copy_file(fio_t *src, fio_t *outfd);
int	fio_rewind(fio_t *fio);
fio_t	*fio_fd(FILE *fd);
fio_t	*fio_dup(fio_t *fio);


#endif /* __FILEIO_H__ */
