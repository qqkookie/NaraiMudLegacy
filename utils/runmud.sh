#!/bin/sh
#	Run MUD forever.
#
#	mudrun is script to run narai mud in contolled way.
#	by Cookie on Sun Sep 14 01:50:36 KST 1997
#
# $pidfile (like "mud-5001.pid") is used as port lock.

PATH=/bin:/usr/bin/:/usr/local/bin:
trap "" 1 2 3 13 14 

port=5002	# port number

datefmt="%Y-%m-%d %a %H:%M:%S %Z"

# mud home and belows should be absolute path
export MUDHOME=$HOME/work
mudhome="$MUDHOME"

prog=$mudhome/bin/mud-narai
prog_new=$prog.new
prog_run=$prog.run
srcdir=$mudhome/src
# GDB=/usr/local/bin/gdb

# belows are relative to $mudhome
mudlink=run-mud
loglink=run-log
checklink=run-check 
# All logs, player files and stash files are saved in $usrdir 
usrdir=lib
    
# belows are relative to $usrdir
pidfile=mud-$port.pid
logdir=log
logfile=$logdir/run-log 
checkfile=$logdir/run-check 

cd $mudhome || exit 1

echo "Running [$prog_run] in [$mudhome/$usrdir] diretory."
echo "Log file: [$mudhome/$usrdir/$logfile]"
echo "See [$loglink] and [$checkfile] in MUD home: [$mudhome]."
echo ""

script=`basename $0`
if [ ! -L $mudlink ] ; then ln -s bin/$script $mudlink; fi
if [ ! -L $loglink ] ; then ln -s $usrdir/$logfile $loglink; fi
if [ ! -L $checklink ] ; then ln -s $usrdir/$checkfile $checklink; fi

export COUNT=1

# all commands are execued in $usrdir
cd $usrdir || exit 2 

while expr $COUNT \< 100 > /dev/null
do 
    export TIMESTAMP="`date +%y-%m-%d_%H-%M`"
    start=`env LC_ALL=C date +"$datefmt"`
    (
	#  from here, all output goes to log file 

	if  [ -e BLOCK.MUD ] ; then exit 3; fi
	if  [ -e $pidfile ] ; then exit 4; fi

	echo "=========    BEGIN   === Start of LOG ===  Run No." $COUNT
	env LC_ALL=C date +"$datefmt"

	echo "=====    Backup stash files" 
	if [ -e stash.tar ] ; then 
	    /bin/mv -f stash.tar stash.tar.old
	fi
	tar -cf stash.tar stash/*

	if [ -f RUNME.sh ] ; then
		echo "=====    Excute RUNME before reboot.... "
		/bin/sh RUNME.sh
		/bin/mv -f RUNME.sh RUNME.DONE
		echo "=====    RUNME Done. renamed to RUNME.DONE.... "
		echo ""
	fi
	if [ -x $prog_new ] ; then
	    if [ -f $prog_run ] ; then /bin/mv -f $prog_run $prog.old; fi
	    echo Found new version, renaming [$prog_new] to [$prog_run]
	    /bin/mv -f $prog_new $prog_run
	fi
	echo '************* NARAI-MUD BOOT -- Run No.' $COUNT ' *********'
	echo ""
    ) >> $logfile 2>&1 

    (
	sleep 1 	# run in backgroup shell

	# run mud in the $usrdir
	$prog_run $port
	mudexit=$?

# #---------------------------------------- 
# 
# 	$GDB -q -cd $mudhome/$usrdir -d $srcdir $prog_run << GDB_INPUT 
# 
# handle SIGHUP nostop print pass
# handle SIGINT nostop print pass
# y
# handle SIGPIPE nostop print pass
# handle SIGALRM nostop print pass
# handle SIGTERM nostop print pass
# handle SIGUSR2 nostop print pass
# handle SIGVTALRM nostop print pass
# run $port 
# bt 
# quit
# y
# GDB_INPUT
# #----------------------------------------

	echo "" 
	echo '************* Narai MUD Died!!!  Run No.' $COUNT ' *********'
		
	env LC_ALL=C date +"$datefmt"
	echo ""


	if [ -f $pidfile ] ; then 
	    echo "Mud: run finished. loop: $COUNT" >> $checkfile
	    tail -20 $logfile >> $checkfile
	    echo "----------------------------------------" >> $checkfile

	    # echo "=====    Restore missing stash files"
	    # # Restore vanished stash files -k : keep existing file
	    # tar -xkf $usrdir/stash.tar > /dev/null 2>&1

	    echo "=====    Backup players file"
	    if [ -f players.4 ] ; then /bin/mv -f players.4 players.5 ; fi
	    if [ -f players.3 ] ; then /bin/mv -f players.3 players.4 ; fi
	    if [ -f players.2 ] ; then /bin/mv -f players.2 players.3 ; fi
	    /bin/cp -p  players players.2 
	fi

	env LC_ALL=C date +"$datefmt"
	echo "=====    DONE.    ====    End of LOG   ====  Run No." $COUNT

        exit $mudexit
    ) >> $logfile 2>&1 &
    bgpid=$!

    (
	sleep 10 	# wait mud to boot up.

	if [ "x$USER" != x ] ; then
	    acct="$USER"
	else
	    acct=SYSTEM
	fi
	echo "----------------------------------------" >> $checkfile
	startdate=env LC_ALL=C date +"$datefmt"
	if [ -f $pidfile ] ; then 
	    mudpid=`cat $pidfile`
	    check="Mud: [$prog_run] started in \
[$mudhome/$usrdir] by [$acct]. PID: $mudpid on $start loop: $COUNT"
	else
	    check="Mud: [$prog_run] failed to run in \
[$mudhome/$usrdir] by [$acct] on $start loop: $COUNT"
	fi 
	echo  $check >> $checkfile
	echo ""
	echo $check
	echo ""
    ) >> $logfile 2>&1 

    runfail=0
    bgexit=0
    if [ -f $pidfile ] ; then

	wait $bgpid
	bgexit=$?

	echo "Mud: Run $COUNT finished."

	if  [ -f $pidfile ] ; then runfail=5; fi	# crashed
	rm -f $pidfile
    else
	runfail=6;
    fi

    if [ -s $logfile ] ; then
	tail -100 $logfile >> $logdir/CRASH-LOG
	echo "Moving $logfil to $logdir/mud@$TIMESTAMP-$COUNT.log"  
	/bin/mv -f $logfile "$logdir/mud@$TIMESTAMP-$COUNT.log"
    else
	 exit 7
    fi

    if [ -f $prog_run.core ] ; then 
        runfail=8
	/bin/rm -f $prog_run.core
    fi

    if [ $runfail != 0 ] ; then exit $runfail; fi
    if [ $bgexit != 0 ] ; then exit $bgexit; fi
    if [ -f BLOCK.MUD ] ; then break; fi

    #  Sleep > 60 secs before next run to wait port to be freed.  
    sleep 100

    COUNT="`expr $COUNT + 1 `"
done 
