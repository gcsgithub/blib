#ifndef __MAIL_UTILS_H__
#define	__MAIL_UTILS_H__
/*
 * @(#) $Id: mail_utils.h,v 1.3 2011/04/14 02:33:22 mark Exp mark $
 *
 *  mail_utils.h
 *  fmtbckrep_xcode
 *
 *  Created by mark on 19/05/2009.
 *  Copyright 2009 Garetech Computer Solutions. All rights reserved.
 *
 *  $Log: mail_utils.h,v $
 *  Revision 1.3  2011/04/14 02:33:22  mark
 *  optional file include
 *
 *  Revision 1.2  2011/04/11 03:53:53  mark
 *  add include log stuff
 *
 *  Revision 1.1  2010/11/16 04:04:00  root
 *  Initial revision
 *
 *
 */
#include "fileio.h"
#define _PATH_SENDMAIL	"/usr/sbin/sendmail"

int mailto(files_t **mailfd_head, char *mailtoaddr, char *subject, int dbgflag, int include_flg);
int	attach_mime_file(fio_t **mailfd_ptr, fio_t *mailtmp, char *bounary_str);

#endif /* __MAIL_UTILS_H__ */
