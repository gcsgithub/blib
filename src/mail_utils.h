#ifndef __MAIL_UTILS_H__
#define	__MAIL_UTILS_H__
/*
 * @(#) $Id:$
 *
 *  mail_utils.h
 *  fmtbckrep_xcode
 *
 *  Created by mark on 19/05/2009.
 *  Copyright 2009 Garetech Computer Solutions. All rights reserved.
 *
 *  $Log:$
 *
 */
#include "fileio.h"
#define _PATH_SENDMAIL	"/usr/sbin/sendmail"

int mailto(fio_t *mailfd, char *mailtoaddr, char *subject, int dbgflag);

#endif /* __MAIL_UTILS_H__ */
