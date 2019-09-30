#!/bin/sh
#
# daily backup script by jhpark
#
# 매일 아침 6 시에 백업, 7일동안 보관
# crontab : 0 6 * * * /home/grad/game/narai/bin/backup
# 
#	backup written by Cookie on Sun Sep 14 01:50:36 KST 1997

PATH=/bin:/usr/bin:/usr/local/bin:
trap "" 1 2 3 13 14 

HDIR=/home/grad/game/narai
#	backup directory should exist and writable 
BDIR=$HDIR/users 

FLIST='Makefile CHANGES README src include lib bin users' 
FN=narai
DAY=7

if [ ! -r $HDIR -o ! -w $BDIR ]; then
	echo echo $0: check directory: $HDIR or $BDIR
	exit 1
fi

HBASE="`basename $HDIR`"
FILES="`echo $FLIST | sed s:^:$HBASE/:g`" 
DATE=`/bin/date "+%y%m%d"`
TARF=${FN}-$DATE.tar

# make tar file, compress it and protect
cd $HDIR/.. && 
tar cf $TARF $FILES && 
gzip $TARF &&
/bin/chmod 400 $TARF.gz

DLIST=`/bin/find $BDIR -mtime +$DAY -name ${FN}-'*' -print `
for f in "$DLIST"
do 
	if [ -f "$f" ]; then
		/bin/chmod 700 $f
		/bin/rm -f $f
	fi
done

