#ifndef __DO_CMD_REPORTBACKUP_H__
#define __DO_CMD_REPORTBACKUP_H__
/*
 *  @(#) $Id: do_cmd_reportbackup.h,v 1.3 2013/01/21 16:53:44 mark Exp $
 *
 *  do_cmd_reportbackup.h
 *  blib
 *
 *  Created by mark on 08/10/2010.
 *  Copyright 2010 Garetech Computer Solutions. All rights reserved.
 *
 *   $Log: do_cmd_reportbackup.h,v $
 *   Revision 1.3  2013/01/21 16:53:44  mark
 *   MG add -w with BLIB.date_width to allow the Start-End date output to be varied in length
 *
 *   Revision 1.2  2011/04/14 02:30:12  mark
 *   fix format of text output
 *
 *   Revision 1.1  2010/11/16 04:04:23  root
 *   Initial revision
 *
 *
 */

#include "util.h"
#include "fileio.h"
#include "mail_utils.h"
#include "data_access.h"
#include "copycheck.h"

#define NOTAG   NULL
#define NOCLASS NULL
#define NOVAL   NULL

void	do_cmd_reportbackup(fio_t *outfd, cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh);

int	create_xhtml_report(dbh_t *dbh, fio_t *outfd, backups_t *bckrec, char *title2, char *style_sheet_name);
void    write_xhtml_header(fio_t *outfd, backups_t *bckrec,char *title2, char *style_sheet_name);
void    write_xhtml_table_header(fio_t *outfd, backups_t *bckrec, char *title2);

int	create_text_report(dbh_t *dbh,fio_t *outfd, backups_t *bckrec, char *title2);
void    write_text_header(fio_t *outfd, backups_t *bckrec, char *title2);
void    write_text_table_header(fio_t *outfd);

int     read_bck_objects(dbh_t *dbh, fmt_type_e fmttype, fio_t *outfd, backups_t *bckrec);
void    table_row(fio_t *outfd,fmt_type_e fmttype,  char *class, char *path, char *bcode, int fileno, char *stime, char *etime, double duration, bcount_t bytes, bcount_t errs);

int     html_write(fio_t *outfd, char flag, char *tag, char *class, char *val, ... );
int     tabout(fio_t *outfd, int offset);
void    style_sheet(char *base, fio_t *outfd);
void    create_dash_line(char *buf, size_t max, size_t dash_count);
void    text_div(FILE *fd);

void text_row(FILE *fd, char *path, char *barcode, char *start_end, char *duration, char *bytes, char *mbytes, char *gbytes, char *errs);
void    text_hdr(FILE *fd, char *hdr_txt);
void text_err(FILE *fd, char *err_lineno, char *when, char *errmsg);
#endif /* __DO_CMD_REPORTBACKUP_H__ */

