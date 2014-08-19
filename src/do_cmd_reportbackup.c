static char *rcsid="@(#) $Id: do_cmd_reportbackup.c,v 1.6 2013/01/21 16:50:47 mark Exp mark $";

/*
 *  do_cmd_reportbackup.c
 *  blib
 *
 *  Created by mark on 08/10/2010.
 *  Copyright 2010 Garetech Computer Solutions. All rights reserved.
 * $Log: do_cmd_reportbackup.c,v $
 * Revision 1.6  2013/01/21 16:50:47  mark
 * MG allow -w to set the width of the Start-End field and size it to match the variablility
 *
 * Revision 1.5  2013/01/20 10:07:09  mark
 * MG changes from time_t to blib_tim_t and required support functions for decimal time
 *
 * Revision 1.4  2011/04/14 02:29:33  mark
 * fix up error reporting
 *
 * Revision 1.3  2011/04/12 00:35:54  mark
 * fix unsigned compare to !=0
 *
 * Revision 1.2  2011/04/11 03:52:25  mark
 * add include log stuff for mail
 *
 * Revision 1.1  2010/11/16 04:05:13  root
 * Initial revision
 *
 *
 */
static char *ver()
{
    return(rcsid);
}

#include "do_cmd_reportbackup.h"
#include "blib.h"
#include "timefunc.h"

extern blib_global_t BLIB;


void	do_cmd_reportbackup(fio_t *outfd,cmd_t **cmds, cmd_t *thecmd, cmd_t *qual_ptr,dbh_t *dbh)
{
    /* /reportbackup"     ,"bck_id"	     , VT_NONE     , REQVAL_REQ  , NULL,	"report backup info"
     
     blib /reportbackup=<bckid>  [/desc=] [ /output= ] [/html] [/mail= ] [/stylesheet=]
     
     /output is setup for us as outfd
     */
    // int		reqval;
    backups_t	bckrec;
    qual_t	qualval;
    cmd_t	*qual;
    fmt_type_e	fmtas = FMT_TEXT;
    char	*mailtoaddress = (char *) NULL;
    fio_t       *report_tmp_fio;
    int        bck_errs;
    
    qual=qual_ptr;
    bzero(&qualval, sizeof(qual_t));
    bck_errs = 0;
    
    qualval.bck_id = *(bckid_t *) thecmd->val;
    qualval.stylesheet = get_default(QUAL_STYSHT);
    
    while(qual) {
        switch(qual->param->cmdid) {
            case QUAL_HTML:
                fmtas = FMT_XHTML;
                break;
            case QUAL_MAIL:
                mailtoaddress = (char *) qual->val;
                break;
            case QUAL_DESC:
                copy_desc(&qualval.desc, (char *) qual->val);
                break;
            case QUAL_STYSHT:
                qualval.stylesheet = (char *) qual->param->defval;
                break;
                
            default:
                fprintf(outfd->fd, "#BLIB:  Error invalid qualifier %s given to %s\n", qual->param->cmdtxt, thecmd->param->cmdtxt);
                return;
                break;
        }
        qual=qual->next;
    }
    
    if (!db_find_backups_by_bck_id(dbh, qualval.bck_id, &bckrec)) {
        replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Error finding backup for id %llu\n", (llu_t) qualval.bck_id));
        return;
    }
    
    if ((report_tmp_fio = fio_open_temp("mailto", "xhtml" , MAX_BUF)) == (fio_t *) NULL ) {
        dbh->status=errno;
        replace_dynstr(&dbh->errmsg, newstr("#BLIB:  Error opening temp file for mailto %d:%s\n", dbh->status, strerror(dbh->status)));
        return;
        
    }
    report_tmp_fio->debug =BLIB.debug;
    
    switch(fmtas) {
        case  FMT_TEXT:
            bck_errs = create_text_report(dbh, report_tmp_fio, &bckrec, qualval.desc.str);
            break;
        case FMT_XHTML:
            bck_errs = create_xhtml_report(dbh, report_tmp_fio, &bckrec, qualval.desc.str, qualval.stylesheet);
            break;
        default:
            fprintf(stderr, "#BLIB:  Internal error unknown report format %d\n", fmtas);
            exit(EINVAL);
            break;
    }
    fio_reopen(report_tmp_fio, "r");
    if (report_tmp_fio->status != 0 ) {
        fprintf(stderr, "#BLIB:  Error reopening report temporary file %d:%s\n", report_tmp_fio->status, strerror(report_tmp_fio->status));
        exit(report_tmp_fio->status);
    }
    
    if (mailtoaddress) {
        replace_dynstr(&report_tmp_fio->mimetype, newstr("text/html"));
        files_insert_head(&BLIB.includelogs, report_tmp_fio);           // add the backup report to the top of the list
                                                                        // the list will be mime attached
        if (mailto(&BLIB.includelogs, mailtoaddress, bckrec.desc.str, BLIB.debug, bck_errs) == -1 ) {
            fprintf(stderr, "#BLIB:  Failed to send email to \"%s\"\n", mailtoaddress);
        }
    } else {
        fio_copy_file(report_tmp_fio, outfd);
    }
}

int	create_text_report(dbh_t *dbh,fio_t *outfd, backups_t *bckrec, char *title2)
{
    write_text_header(outfd, bckrec, title2);
    write_text_table_header(outfd);
    
    return(read_bck_objects(dbh, FMT_TEXT , outfd, bckrec));
}

int   create_xhtml_report(dbh_t *dbh, fio_t *outfd, backups_t *bckrec, char *title2, char *style_sheet_name)
{
    int bck_errs;
    
    write_xhtml_header(outfd, bckrec, title2, style_sheet_name);
    write_xhtml_table_header(outfd, bckrec, title2);
    
    bck_errs = read_bck_objects(dbh, FMT_XHTML , outfd, bckrec);
    
    html_write(outfd,'C', "table", NOCLASS, NOVAL);
    html_write(outfd,'C', "body", NOCLASS, NOVAL);
    html_write(outfd,'C', "html", NOCLASS, NOVAL);
    
    return(bck_errs);
}

int read_bck_objects(dbh_t *dbh, fmt_type_e fmttype, fio_t *outfd, backups_t *bckrec)
{
    blib_tim_t  	gtot_duration;
    blib_tim_t  	tot_duration;
    double  	duration;
    
    bcount_t 	tot_bytes;
    bcount_t 	gtot_bytes;
    size_t  	tapecount;
    int	    	labelsinobject;
    char    	*sp;
    bckobj_t	bckobjrec;
    vol_obj_t	volobjrec;
    bck_errors_t bckerrrec;
    vol_t       volrec;
    int         dbstatus_bckobj;
    int         dbstatus_bckerrs;
    int         dbstatus_vol_obj;
    datestr_t	start;
    datestr_t	end;
    bcount_t	tot_errs;
    bcount_t	gtot_errs;
    bcount_t	errs;
    objname_t	cur_objname;
    objname_t	prev_objname;
    blabel_t	cur_label;
    blabel_t	prev_label;
    char        *objname;
    int         volduration;
    bcount_t    volsize;
    
    
    tot_duration = gtot_duration = 0;
    tot_bytes	 = gtot_bytes	 = 0;
    tot_errs     = gtot_errs     = 0;
    
    if (fmttype == FMT_XHTML) html_write(outfd,'O', "tbody", NULL, NULL);
    
    
    bzero(&cur_objname, sizeof(cur_objname));
    bzero(&prev_objname, sizeof(prev_objname));
    bzero(&prev_label, sizeof(prev_label));
    bzero(&cur_label, sizeof(cur_label));
    
    labelsinobject = 0;
    objname = (char *) &bckobjrec.objname;
    
    dbstatus_bckobj = db_find_bck_objects_by_bckid(dbh, bckrec->bck_id, &bckobjrec, FND_FIRST);
    // LOOPing  OBJNAMES  (BCK_OBJECTS keyed by bck_id)
    while (dbstatus_bckobj) {
        dbstatus_vol_obj = db_find_vol_obj_from_objects(dbh, &bckobjrec, &volobjrec, FND_FIRST);
        
        //      LOOPing LABELS (VOL_OBJ keyed by bck_objects.bck_id, bck_objects.objname)
        while (dbstatus_vol_obj) {
            // prev = cur, cur=new
            copy_label(&prev_label, cur_label.str);
            copy_label(&cur_label, volobjrec.label.str);
            labelsinobject++;
            
            duration = difftime(volobjrec.end, volobjrec.start);
            
            copy_datestr(&start, (datestr_t *) time_cvt_blib_to_str(volobjrec.start));
            copy_datestr(&end  , (datestr_t *) time_cvt_blib_to_str(volobjrec.end));
            
            errs =  db_count_bck_errors(dbh, &volobjrec);
            
            table_row(outfd , fmttype      ,NULL        ,objname, volobjrec.label.str, volobjrec.fileno, start.str, end.str, duration, volobjrec.size, errs);
            objname="";
            tot_bytes    += volobjrec.size;
            tot_duration += (blib_tim_t ) duration;
            
            
            dbstatus_bckerrs = db_find_bck_errors(dbh, &volobjrec, &bckerrrec ,FND_FIRST); // key on bck_id, objname
            tot_errs=0;
            while (dbstatus_bckerrs) {
                copy_datestr_time(&start, (datestr_t *) time_cvt_blib_to_str(bckerrrec.errtime));
                table_row(outfd, fmttype , "err", bckerrrec.objname.str, bckerrrec.label.str, -1, start.str ,bckerrrec.errmsg.str  , -1 , -1,  ++tot_errs);
                dbstatus_bckerrs = db_find_bck_errors(dbh, &volobjrec, &bckerrrec ,FND_NEXT);
            }
            
            dbstatus_vol_obj = db_find_vol_obj_from_objects(dbh, &bckobjrec, &volobjrec, FND_NEXT); // Next label
                                                                                                    //if (cmp_labels(&cur_label, &volobjrec.label)) { // if a new label
                                                                                                    //}
        }
        
        dbstatus_bckobj = db_find_bck_objects_by_bckid(dbh, bckrec->bck_id, &bckobjrec, FND_NEXT); // next backup object
        
        if (labelsinobject > 1)  {
            table_row(outfd, fmttype, "sub" ,"", "", -1, NL,NL, tot_duration, tot_bytes, tot_errs); /* total for a objname */
        }
        gtot_duration += tot_duration;
        tot_duration = 0;
        gtot_bytes += tot_bytes;
        tot_bytes = 0;
        gtot_errs += tot_errs;
        tot_errs = 0;
        labelsinobject = 0;
        
        // prev = cur, cur=new
        copy_objname(&prev_objname, cur_objname.str);
        copy_objname(&cur_objname, volobjrec.objname.str);
        objname = (char *) &bckobjrec.objname;
    }
    
    table_row(outfd,fmttype ,"total", "Totals", "", -1, NL,NL,gtot_duration, gtot_bytes, gtot_errs);
    if (fmttype == FMT_XHTML) html_write(outfd,'C', "tbody", NULL, NULL);
    tapecount=0;
    
    if (fmttype == FMT_XHTML) html_write(outfd,'O', "tfoot", NULL, NULL);
    else			          text_div(outfd->fd);
    
    sp="Tape Summary";
    dbstatus_vol_obj = db_find_volumes_by_bckid(dbh, bckrec->bck_id, &volrec, FND_FIRST);
    while (dbstatus_vol_obj) {
        tapecount++;
        volsize = db_get_size(dbh, &volrec);
        volduration = db_get_duration(dbh, &volrec);
        table_row(outfd, fmttype,"tapesum", sp , volrec.label.str,-1,NL,NL,volduration, volsize, 0);
        sp=NL;
        dbstatus_vol_obj = db_find_volumes_by_bckid(dbh, bckrec->bck_id, &volrec, FND_NEXT);
    }
    if (fmttype == FMT_XHTML)   html_write(outfd,'C', "tfoot", NULL, NULL);
    else                        text_div(outfd->fd);
    return(gtot_errs);
}


void write_xhtml_header(fio_t *outfd, backups_t *bckrec, char *title2, char *style_sheet_name)
{
    
    html_write(outfd,'L',NOTAG, NOCLASS, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"  \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">");
    html_write(outfd,'O',"html xmlns=\"http://www.w3.org/1999/xhtml\"", NOCLASS, NOVAL);
    html_write(outfd,'O',"head", NOCLASS, NOVAL);
    
    if (style_sheet_name) style_sheet(style_sheet_name, outfd);
    
    html_write(outfd,'I',"title", NOCLASS, "%llu %-11.11s Backup report of %s:\"%s\"",
               (llu_t) bckrec->bck_id,
               time_cvt_blib_to_str(bckrec->start),
               pstr(bckrec->node.str,""),
               pstr(bckrec->desc.str,""));
    if (title2) {
        html_write(outfd,'I',"title", NOCLASS, "%s", title2);
    }
    html_write(outfd,'C',"head", NOCLASS, NOVAL);
    html_write(outfd,'O',"body", NOCLASS, NOVAL);
    
}

void write_xhtml_table_header(fio_t *outfd, backups_t *bckrec, char *title2)
{
    html_write(outfd,'O',"table", NOCLASS, NOVAL);
    
    
    html_write(outfd,'O',"caption"  , NOCLASS,NOVAL);
    html_write(outfd,'I',"p"  , NOCLASS,  "%llu %-11.11s Backup report of %s:\"%s\"",
               (llu_t) bckrec->bck_id,
               time_cvt_blib_to_str(bckrec->start),
               pstr(bckrec->node.str,""),
               pstr(bckrec->desc.str,""));
    if (title2) {
        html_write(outfd,'I',"p", NOCLASS, "%s", title2);
    }
    html_write(outfd,'C',"caption"  , NOCLASS,NOVAL);
    
    
    html_write(outfd,'O', "thead"	, NOCLASS, NOVAL);
    html_write(outfd,'O', "tr"	, NOCLASS, NOVAL);
    html_write(outfd,'I', "th"	, NOCLASS, "Pathname");
    html_write(outfd,'I', "th"	, NOCLASS, "Barcode:file#");
    html_write(outfd,'I', "th"	, NOCLASS, "Start-End Time");
    html_write(outfd,'I', "th"	, "right", "Seconds");
    html_write(outfd,'I', "th"	, "right", "Bytes");
    html_write(outfd,'I', "th"	, "right", "MBytes");
    html_write(outfd,'I', "th"	, "right", "GBytes");
    html_write(outfd,'I', "th"    , "right", "Errs");
    html_write(outfd,'C', "tr"	, NOCLASS, NOVAL);
    html_write(outfd,'C', "thead"	, NOCLASS, NOVAL);
}
/*
 +--------------------------------------+-------------+-------------------+--------+------------+----------+----------+
 | Pathname                             |Barcode:file#| Start-End Time    | Seconds|    Bytes   |  MBytes  |  GBytes  |
 +--------------------------------------+-------------+-------------------+--------+------------+----------+----------+
 | /                                    |  OBG028D:0  | 21:01:47-21:02:43 |     56 |   152465731|    145.40|    0.1420|
 |12345678901234567890123456789012345678|1234567890123|1234567890123456789|12345678|123456789012|1234567890|1234567890|
 38                             13             19                8          12           10         10
 
 123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
 1         2         3         4         5         6         7         8         9         11        12        13
 | /usr                                 |  OBG028D:1  | 21:02:43-21:09:02 |    379 |  1818238368|  1734.01|   1.6934|
 */
void table_row(fio_t *outfd, fmt_type_e fmttype ,char *iclass,  char *path, char *bcode, int fileno, char *stime, char *etime, double duration, bcount_t bytes, bcount_t errs)
{
    static	char	*class;
    static  int	row=0;
    static  int last_was_err=0;
    int		errline;
    char	path_str[28];
    char	bcode_str[80];
    char	time_str[80];
    char	stime_str[32];
    char	etime_str[32];
    char	duration_str[80];
    char	bytes_str[80];
    char	kbytes_str[80];
    char	mbytes_str[80];
    char	gbytes_str[80];
    char	err_str[80];
    blib_global_t *gbl = &BLIB;
    
    if (path==NL) path="";
    shrink_string_by_middle(path_str, sizeof(path_str)-1, path); // reduce string from the middle to fit a give length so we can see start..end
    
    errline=0;
    if (iclass) {
        if (strncmp(iclass,"err", 3) == 0) {
            errline=1;
            last_was_err=1;
            if (row) {
                class = "l1errline";
                row=0;
            } else {
                class = "l0errline";
                row=1;
            }
        } else {
            class = iclass;
        }
    } else {
        if (row) {
            if (errs)	class = "l1err";
            else	class = "l1";
            row=0;
        } else {
            if (errs)	class = "l0err";
            else	class = "l0";
            row=1;
        }
    }
    
    
    if (bcode) {
        if (fileno>=0)	 snprintf(bcode_str,sizeof(bcode_str) , "%s:%d", bcode, fileno);
        else		     snprintf(bcode_str,sizeof(bcode_str) , "%s"   , bcode);
    } else {
        bzero(bcode_str,sizeof(bcode_str));
    }
    
    if (stime) {
        snprintf(stime_str, sizeof(stime_str) , "%-*.*s",gbl->date_width, gbl->date_width, stime);
    }
    if (etime) {
        snprintf(etime_str, sizeof(etime_str) , "%-*.*s",gbl->date_width, gbl->date_width, etime);
    }
    
    if ((stime) && (etime)) {
        snprintf(time_str, sizeof(time_str) , "%s-%s", stime_str, etime_str);
    } else if (stime) {
        snprintf(time_str, sizeof(time_str) , "%s-%s", stime_str, "");
    } else if (etime) {
        snprintf(time_str, sizeof(time_str) , "%s-%s", ""   , etime_str);
    } else {
        bzero(time_str, sizeof(time_str));
    }
    
    if (duration >= 0 ) {
        snprintf(duration_str, sizeof(duration_str), "%.0f", duration);
    } else {
        bzero(duration_str, sizeof(duration_str));
    }
    
    if (bytes != 0 ) {
        snprintf(bytes_str,  sizeof(bytes_str)  , "%llu",  (llu_t) bytes);
        snprintf(kbytes_str, sizeof(kbytes_str) , "%.2f", ((double)  bytes)/1024.0);
        snprintf(mbytes_str, sizeof(mbytes_str) , "%.4f", ((double)  bytes)/1024.0/1024.0);
        snprintf(gbytes_str, sizeof(gbytes_str) , "%.4f", ((double)  bytes)/1024.0/1024.0/1024.0);
    } else {
        bzero(bytes_str ,  sizeof(bytes_str));
        bzero(kbytes_str,  sizeof(kbytes_str));
        bzero(mbytes_str,  sizeof(mbytes_str));
        bzero(gbytes_str,  sizeof(gbytes_str));
        
    }
    
    if (errs) {
        snprintf(err_str, sizeof(err_str), "%llu",  (llu_t) errs);
    } else {
        bzero(err_str, sizeof(err_str));
    }
    
    if (fmttype == FMT_XHTML) {
        
        html_write(outfd,'O', "tr", class    , NOVAL);
        
        if (errline ) {
            html_write(outfd,'I', "td", NOCLASS  , "%s" , err_str);  // err line number
            html_write(outfd,'I', "td", NOCLASS  , "%s" , stime); // when
            html_write(outfd,'I', "td", "errfld" , "%s" , etime); // error message string
            
        } else {
            html_write(outfd,'I', "td", NOCLASS  , path);
            html_write(outfd,'I', "td", NOCLASS  , bcode_str);
            html_write(outfd,'I', "td", NOCLASS  , time_str);
            html_write(outfd,'I', "td", "right"  , duration_str);
            html_write(outfd,'I', "td", "right"  , bytes_str);
            html_write(outfd,'I', "td", "right"  , mbytes_str);
            html_write(outfd,'I', "td", "right"  , gbytes_str);
            html_write(outfd,'I', "td", "right"  , err_str);
        }
        html_write(outfd,'C', "tr", NOCLASS  , NOVAL);
        
    }  else {
        // FMT_TEXT
        if (errline ) {
            text_err(outfd->fd, err_str,  // err line number
                     stime,   // when
                     etime    // error message string
                     );
        } else {
            if (last_was_err) {
                text_div(outfd->fd);
                last_was_err=0;
            }
            text_row(outfd->fd, path, bcode_str, time_str, duration_str,            bytes_str, mbytes_str, gbytes_str, err_str);
            text_div(outfd->fd);
        }
    }
}

int	html_write(fio_t *outfd, char flag, char *tag, char *class, char *val, ... )
{
    va_list args;
    
    static  int offset=0;
    int	    rval;
    
    rval=0;
    
    switch(flag) {
        case 'O':
            /* offset++; */
            break;
        case 'I':
            break;
        case 'C':
            if (offset > 0 ) offset--;
            break;
        case 'L':
            break;
        default:
            break;
    }
    
    if (!outfd) return(EINVAL);
    // Close, Inline, Literal, Open tag flags
    if ((flag == 'C') || (flag == 'I') || (flag == 'L') || (flag == 'O')) {
        tabout(outfd, offset);
    }
    if ((flag == 'O') || (flag =='I')) {
        /* Open tag  */
        
        if ( class && *class  ) {
            rval = fprintf(outfd->fd, "<%s class=\"%s\">",tag, class);
        } else {
            rval = fprintf(outfd->fd, "<%s>",tag);
        }
        
        if (flag == 'I') {	/* inline tag <tag>val</tag> with val and close val maybe null <tag></tag> */
            if (val) {
                va_start(args, val);
                vfprintf(outfd->fd, val, args);
                va_end(args);
            }
        }
    }
    if ((flag == 'C') || (flag == 'I')) {	/* closing tag only  */
        rval = fprintf(outfd->fd, "</%s>", tag);
    }
    
    if (flag == 'L') { /* Literal value only no tags involved or they included in the string */
        va_start(args, val);
        vfprintf(outfd->fd, val, args);
        va_end(args);
    }
    
    if ((flag == 'C') || (flag == 'I') || (flag == 'L') || (flag == 'O')) {
        fprintf(outfd->fd, "\n"); outfd->writes++;
    }
    
    switch(flag) {
        case 'O':
            offset++;
            break;
        case 'I':
            break;
        case 'C':
            /* if (offset > 0 ) offset--; */
            break;
        case 'L':
            break;
        default:
            break;
    }
    
    return (rval);
}

int tabout(fio_t *outfd, int offset)
{
    int rval, idx;
    rval = 0;
    for (idx=offset;idx> 0;idx--) {
        rval = fprintf(outfd->fd, "\t");
    }
    return(rval);
}



void	style_sheet(char *base, fio_t *outfd)
{
    char	ibuf[8192];
    fio_t	*stylesheet_fio;
    
    if (!base) return;
    if (!outfd) return;
    
    stylesheet_fio = fio_alloc_open(base, ".stylesheet", "r", MAX_BUF);
    
    if ((stylesheet_fio->status == 0 ) && (stylesheet_fio->open)) {
        
        html_write(outfd,'O',"style type=\"text/css\"", NOCLASS, NOVAL);
        html_write(outfd,'L',NOTAG, NOCLASS, "/*<![CDATA[*/");
        
        fgets(ibuf, sizeof(ibuf), stylesheet_fio->fd);
        while(!feof(stylesheet_fio->fd)) {
            zapcrlf(ibuf);
            html_write(outfd, 'L', NOTAG, NOCLASS,  ibuf);
            fgets(ibuf, sizeof(ibuf), stylesheet_fio->fd);
        }
        fio_close_and_free(&stylesheet_fio);
        
        html_write(outfd,'L',NOTAG, NOCLASS, "/*]]>*/");
        html_write(outfd,'C',"style",NOCLASS,NOVAL);
    } else {
        fprintf(stderr, "#BLIB:  Warning: error opening stylesheet \"%s\" %d:%s\n", stylesheet_fio->fnm, stylesheet_fio->status, strerror(stylesheet_fio->status));
    }
}



void write_text_header(fio_t *outfd,backups_t *bckrec, char *title2)
{
    char hdr_str[256];
    
    snprintf(hdr_str, sizeof(hdr_str), "%llu %-11.11s Backup report of %s:\"%s\"",
             (llu_t) bckrec->bck_id,
             (char *) time_cvt_blib_to_str(bckrec->start),
             pstr(bckrec->node.str,""),
             pstr(bckrec->desc.str,""));
    text_div(outfd->fd);
    text_hdr(outfd->fd, hdr_str);
    if (title2) {
        text_hdr(outfd->fd, title2);
    }
}

void write_text_table_header(fio_t *outfd)
{
    
    text_div(outfd->fd);
    text_row(outfd->fd, "Pathname", "Barcode:file#", "Start-End Time", "Seconds", "Bytes", "MBytes", "GBytes", "Errs");
    text_div(outfd->fd);
}




void create_dash_line(char *buf, size_t max, size_t dash_count)
{
    size_t idx;
    
    max--; // make sure we have at least 1 for null termination;
    
    for (idx=0;idx<max;idx++) {
        buf[idx] = '-';
        if (idx == dash_count) {
            break;
        }
        
    }
    // idx will always be either dash_count ot max-1 which ever the lesser
    // ie the buffer is safe with in 0..max-1
    buf[idx] = '\0';
}

/*********************/
void text_div(FILE *fd)
{
    blib_global_t *gbl = &BLIB;
    char    start_end_buf[132];
    int     start_end_len;
    
    start_end_len = 2 + (gbl->date_width*2);
    // create a slash line of length
    create_dash_line(start_end_buf, sizeof(start_end_buf), start_end_len);
    
    fprintf(fd, "+----------------------------+--------------+%s+--------+---------------------+---------------+------------+----+\n", start_end_buf);
    
}

/************************************/
void text_hdr(FILE *fd, char *hdr_txt)
{
    blib_global_t *gbl = &BLIB;
    int msglen;
    
    msglen = 110 +  (2*gbl->date_width);
    fprintf(fd, "| %-*s|\n", msglen, hdr_txt);
}

/****************************************************************/
void text_err(FILE *fd, char *err_lineno, char *when, char *errmsg)
{
    blib_global_t *gbl = &BLIB;
    int msglen;
    
    msglen = 68 +  (2*gbl->date_width);
    fprintf(fd,"| %-27.27s| %-13s| %-*s|\n", err_lineno, when, msglen,  errmsg);
}

/**********************************************************************/
void text_row(FILE *fd, char *path, char *barcode, char *start_end, char *duration, char *bytes, char *mbytes, char *gbytes, char *errs)
{
    blib_global_t *gbl = &BLIB;
    int start_end_len;
    
    // | 15-Oct-2010:05:32:54.00-15-Oct-2010:05:45:08.00|
    // | 15-Oct-2010:05:32:54-15-Oct-2010:05:45:08|
    // | 15-Oct-2010:05:32-15-Oct-2010:05:45|
    // | 15-Oct-2010-15-Oct-2010|
    start_end_len = 2 + (gbl->date_width*2)-1;
    
    fprintf(fd, "| %-27.27s| %-13s| %-*s|%8s|%20s |%14s |%11s |%4s|\n",
            path,
            barcode,
            start_end_len, start_end,
            duration,
            bytes,
            mbytes,
            gbytes,
            errs);
}
/*
 +--------------------------------------+-------------+-------------------+--------+------------+----------+----------+------+
 | Pathname                             |Barcode:file#| Start-End Time    | Seconds|    Bytes   |  MBytes  |  GBytes  | Errs |
 +--------------------------------------+-------------+-------------------+--------+------------+----------+----------+------+
 | /                                    |  OBG028D:0  | 21:01:47-21:02:43 |     56 |   152465731|    145.40|    0.1420| 0    |
 | %-42.42s | %9.9s:%3d | %20s-%20s | %5d | %20llu | %14llu | %11llu | %3d |
 | %-42.42s | %-13ds| %-41s | %s5s | %s20s | %s14s | %s11s | %s3s |
 |12345678901234567890123456789012345678|1234567890123|1234567890123456789|12345678|123456789012|1234567890|1234567890|
 38                             13             19                8          12           10         10
 
 123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
 1         2         3         4         5         6         7         8         9         11        12        13
 | /usr                                 |  OBG028D:1  | 21:02:43-21:09:02 |    379 |  1818238368|  1734.01|   1.6934|
 
 
 fd);
 #define DIV_TEXT_FMT "+----------------------------+--------------+------------------------------------------+--------+---------------------+---------------+------------+----+\n"
 #define HDR_TEXT_FMT "| %-150s|\n"
 #define ROW_TEXT_FMT "| %-27.27s| %-13s| %-41s|%8s|%20s |%14s |%11s |%4s|\n"
 #define ERR_TEXT_FMT "| %-27.27s| %-13s| %-106s|\n"
 */