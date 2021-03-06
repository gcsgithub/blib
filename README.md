BLIB - Backup LIBrary

Q/ I have a tape robot library with barcode scanner, a UNIX backup utility,tar,
cpio,vxdump,vdump,dump etc but no other software how do I keep track of backups?

A/ BLIB.

blib will
 *  record backup events against a label with an expiration time
 *  manage the state of labels FREE or ALLOCATED
a simple session would look like

blib /add=TST123 /MEDIA=TZ89
export BLIB_BCK_ID=0;
export BLIB_VOLUME='TST123';
export BLIB_STATE=FREE;
export BLIB_MEDIA='TZ89';
export BLIB_USAGE=0;
export BLIB_GROUP='NODE';
export BLIB_LOCATION='UNKNOWN';
export BLIB_LIBRARYDATE=25-Aug-2014:14:48:06.65;
export BLIB_OFFSITEDATE=NotSet;
export BLIB_CHANGEDATE=25-Aug-2014:14:48:06.65;
export BLIB_RECORDDATE=NotSet;
export BLIB_EXPIREDATE=NotSet;
export BLIB_DESC='';

blib /newbackup /desc="test simple" /expire=26-Aug-2014 /record=25-aug-2014
BLIB_BCKID='1408942216'

blib /startbackup="/usr" /record=$( caltime 0 ) /bck_id=1408942216 /label=TST123
export BLIB_VOLID=1;

blib /endbackup="/usr" /bck_id=1408942216 /objinstance=1 /label="TST123" /end=$( caltime 0)  /size=1748

blib /finishbackup /bck_id=1408942216

blib /listbackups=1408942216
====================================================================================
bck_id         Start                     End                      Expire                   Description
1408942216     25-Aug-2014:00:00:00.00   25-Aug-2014:14:53:11.00  26-Aug-2014:00:00:00.00  test simple
-------------------------------------------------------------------------------
Volume    #Files  #use  Group     Location   Media     State            Offsite
--------- ------ ------ --------- ---------- --------- -----  -----------------
TST123         1      1 NODE      UNKNOWN    TZ89      ALLOC NotSet
* End of Report *

blib /reportbackup=1408942216
+----------------------------+--------------+------------------------------------------+--------+---------------------+---------------+------------+----+
| 1408942216 25-Aug-2014 Backup report of NODE:"test simple"                                                                                             |
|                                                                                                                                                       |
+----------------------------+--------------+------------------------------------------+--------+---------------------+---------------+------------+----+
| Pathname                   | Barcode:file#| Start-End Time                           | Seconds|               Bytes |        MBytes |     GBytes |Errs|
+----------------------------+--------------+------------------------------------------+--------+---------------------+---------------+------------+----+
| /usr                       | TST123:0     | 25-Aug-2014:14:51:57-25-Aug-2014:14:53:11|      74|                1748 |        0.0017 |     0.0000 |    |
+----------------------------+--------------+------------------------------------------+--------+---------------------+---------------+------------+----+
| Totals                     |              |                                          |      74|                1748 |        0.0017 |     0.0000 |    |
+----------------------------+--------------+------------------------------------------+--------+---------------------+---------------+------------+----+
+----------------------------+--------------+------------------------------------------+--------+---------------------+---------------+------------+----+
| Tape Summary               | TST123       |                                          |      73|                1748 |        0.0017 |     0.0000 |    |
+----------------------------+--------------+------------------------------------------+--------+---------------------+---------------+------------+----+
+----------------------------+--------------+------------------------------------------+--------+---------------------+---------------+------------+----+


blib /help
/help          - This help text
/env           - Display environment in effect
/add="label"           - Add a new volume name to database
/display="label"               - Display existing volume and any fsets
/modify="label"                - Modify an existing volume
/remove="label"                - Remove and existing volume and any fsets from database
/reportfree[=0-2147483647]             - Report free volumes for compatibility
/reportexpired[=0-2147483647]          - Report expired volumes for compatibility
/runexpiration[=0-2147483647]          - free up all or up too n expired tapes
/report                - Report all or filtered list of volumes in database
/replaylog="filename"          - Replay log file [/new to begin a new database]
/newbackup             - Establish a new backup and return its bck_id
/startbackup="objname"         - start backup of object on a given /label=
/change_volume="objname"               - close current volume of object start new on /label=
/endbackup="objname"           - finish backup of object
/errbackup="objname"           - report an error during backup against /errbackup= /bck_id= /label=
/finishbackup          - finish a backup id (verify and summary updates)
/removebackup=0-9223372036854775807            - Remove all data refering to backup id given
/modifybackup=0-9223372036854775807            - Modify backup info for given bck_id
/reportbackup[=0-9223372036854775807]          - report backup info for given bck_id
/listbackups[=0-9223372036854775807]           - list all backups or given backupid or for a given /label=
/listobjects[="objname"]               - list all backups for all backups or a given object name
/verify                - Verify internal conistancy of database tables
/errcount=0-9223372036854775807                - Return BLIB_ERRCOUNT for a given backup id
/new           - only valid for /import
/since=dd-Mmm-YYYY:HH:MM:SS.CC         - with /replaylog /since=ctime:n[.n] to replay log since n
/until=dd-Mmm-YYYY:HH:MM:SS.CC         - with /replaylog /until=ctime:n[.n] to replay log since n
/log[="/usr/local/etc/dat/blib/blib_node.log"]               - log command to optional log file def: BLIB_LOG
/nolog         - Do NOT log this command to BLIB_LOG
/state=ALLOCATED|FREE          - Volume state Free or Allocated
/media="TZ89"          - media type def: TZ89...
/usage[=0-2147483647]          - Increment/Set usage counter
/group="CENTS7"                - The Backup group this volume belongs too MYBLIB_GROUP
/location="location"           - where is it
/database="/usr/local/etc/dat/blib/blib_CENTS7.sqlite3"                - name of the database BLIBDBS
/record=dd-Mmm-YYYY:HH:MM:SS.CC                - start time
/end=dd-Mmm-YYYY:HH:MM:SS.CC           - end time
/libdate=dd-Mmm-YYYY:HH:MM:SS.CC               - date volume was added to library normally calculated
/offsite=dd-Mmm-YYYY:HH:MM:SS.CC               - suggested Date the tape should be offsite by
/expire=dd-Mmm-YYYY:HH:MM:SS.CC                - Date the backup id will expire and return volumes to /state=FREE
/size=0-9223372036854775807            - Bytes to record
/desc="desc"           - Description of the backup or error
/node="ZED"            - name of node the backup was recorded on
/objinstance=0-2147483647              - BLIB_OBJINSTANCE returned by /startbackup usually 1
/bck_id=0-9223372036854775807          - bck_id that we intend working on
/label="label"         - Volume label to be used
/html          - output in html rather than plain text
/stylesheet="/usr/local/etc/dat/blib/fmtbckrep.stylesheet"             - stylesheet to include for /html output
/mail="string"         - email backup report to address
/output="filename"             - output result to file
/includelog="filename"         - include log file in output


When using command /report results can be filtered by most of the volume qualifiers eg. /state=FREE
comparisons other than equal are allowed to simplify keyboard input , can be used for < and . for >
otherwise < and > need to be escaped from the shell using them as input/output redireciton
NOTE is the symbol !  ie NOT equal  /state!=FREE for state not equal to free this can also be just /state!FREE
Less Than   ,  or <   /expire,$( caltime 0)  or "/expire<$(caltime 0)"
Less than or Equal  ,= or <=
Equal  = or ==
Not Equal  !=
Greater Than or equal .= or >=
Greater than . or >
Optional ?= or =? valid only for /includelogs will optional include the file only when errors are found
in the backup beging reported
Times maybe entered as absolute ctime values using the format /qual=ctime:1408941961.983657
