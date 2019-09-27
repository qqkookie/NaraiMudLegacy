#!/bin/sh
#	Run MUD forever.
#
#	mudrun is script to run narai mud in contolled way.
#	by Cookie on Sun Sep 14 01:50:36 KST 1997
#	Revised by Cookie on Sun Sep 15 00:14:35 KST 2019
#
# $pidfile (like "mud-5001.pid") is used as port lock.

# go backgorund
if [ -z "$background" ] ; then
        background=1 exec "$0" "$@" &
	exit
fi

PATH=/bin:/usr/bin/:/usr/local/bin:
trap "" 1 2 3 13 14

umask 0077

port=5002	# $usrdir/PORT is primary

# mud home and belows should be absolute path
export MUDHOME=$HOME/mud
mudhome="$MUDHOME"

prog=$mudhome/bin/mud-narai
prog_new=$prog.new
prog_run=$prog.run
srcdir=$mudhome/src

# belows are relative to $mudhome
mudlink=run-mud
loglink=log-run
lastlink=log-last
checklink=log-check
# All logs, player files and stash files are saved in $usrdir
usrdir=lib

if [ -f $mudhome/$usrdir/PORT ] ; then
	port=`cat $mudhome/$usrdir/PORT`
fi

# belows are relative to $usrdir
pidfile=mud-$port.pid
logdir=log
logfile=$logdir/run-log
checkfile=$logdir/run-check

datefmt="%Y-%m-%d %a %H:%M:%S %Z"

# Set $DEBUGGER if you want post-mortem backtrace (Linux + gdb only)
#
# DEBUGGER="gdb -q -cd=$mudhome/$usrdir -d $srcdir $prog_run"
# DEBUGGER=
DEBUGGER="gdb -q -cd=$mudhome/$usrdir -d $srcdir $prog_run"
#---------------------------------------------------------------

cd $mudhome || exit 1

echo ""
cat <<- STARTMSG
	Running [$prog_run] in [$mudhome/$usrdir] diretory.
	Log file: [$mudhome/$usrdir/$logfile]
	See [$loglink] and [$checklink] in MUD home: [$mudhome]
	STARTMSG

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

	if  [ -e BLOCKMUD ] ; then break; fi
	if  [ -e $pidfile ] ; then
		echo Port $port lock file $pidfile already exists. >> $checkfile
		exit 3
	fi
	(
		#  run subshell in backgorund and all output goes to log file
		echo "==========    BEGIN   === Start of LOG Run No. $COUNT =========="
		env LC_ALL=C date +"$datefmt"

		echo "----------    Backup players file"
		if [ -f players.4 ] ; then /bin/mv -f players.4 players.5 ; fi
		if [ -f players.3 ] ; then /bin/mv -f players.3 players.4 ; fi
		if [ -f players.2 ] ; then /bin/mv -f players.2 players.3 ; fi
		/bin/cp -p  players players.2

		echo "----------    Backup stash files"
		if [ -e stash.tar ] ; then
			/bin/mv -f stash.tar stash.tar.old
		fi
		tar -cf stash.tar players stash/*

		if [ -f RUNME.sh ] ; then
			echo "---------    Excute RUNME before reboot.... "
			/bin/sh RUNME.sh
			/bin/mv -f RUNME.sh RUNME.DONE
			echo "---------    RUNME Done. renamed to RUNME.DONE.... "
			echo ""
		fi
		if [ -x $prog_new ] ; then
			if [ -f $prog_run ] ; then /bin/mv -f $prog_run $prog.old; fi
			echo Found new version, renaming [$prog_new] to [$prog_run]
			/bin/mv -f $prog_new $prog_run
		fi

		sleep 1
		echo "************* NARAI-MUD BOOT --  Run No. $COUNT ****************"
		echo ""

		if [ -n "$DEBUGGER" -a "`uname`" = "Linux" ] ; then
			#----------------------------------------
		
			# lldb -f $prog_run <<- DBG_INPUT
			$DEBUGGER <<- DBG_INPUT
		
			handle SIGHUP nostop print pass
			# handle SIGINT nostop print pass
			handle SIGPIPE nostop print pass
			handle SIGALRM nostop print pass
			# handle SIGTERM nostop print pass
			handle SIGUSR2 nostop print pass
			handle SIGVTALRM nostop print pass
			run $port
			backtrace
			quit
			y
			DBG_INPUT
			#----------------------------------------
		
		else
			# run mud in the $usrdir
			exec $prog_run $port
			exit
		fi

	) >> $logfile 2>&1 &

	mudpid=$!

	# main shell conntinues here.

	sleep 5 	# wait mud to boot up.

	(
		if [ "x$USER" != x ] ; then
			acct="$USER"
		else
			acct=SYSTEM
		fi

		echo "============================================================================"
		if [ -f $pidfile ] ; then
			check="STARTED PID: `cat $pidfile`"
			running="Running...."
		else
			check="FAILED to run"
		fi
		cat <<- CHKMSG
		Mud: $check on port $port loop: $COUNT by [$acct]
		     [$prog_run] in [$mudhome/$usrdir]
		     Time: `env LC_ALL=C date +"$datefmt"`
		$running
		CHKMSG
	) >> $checkfile

	runfail=0
	mudexit=0

	if  [ -f $pidfile ] ; then
		echo "Staring Run $COUNT success. Now waiting ..."  >> $logfile
		wait $mudpid
		mudexit=$?
	else
		# Failed to run
		runfail=4;
	fi

	(
		echo "Mud: run finished. loop: $COUNT"
		env LC_ALL=C date +"$datefmt" 

		if [ -s $logfile -a -f $pidfile ] ; then
			echo "Killed or crashed: log ==> "
			echo "----------------------------------------"
			tail -20 $logfile
			echo "----------------------------------------"
		fi
	) >> $checkfile

	if [ $runfail = 0 ] ; then

		(
			echo ""
			env LC_ALL=C date +"$datefmt"
			echo "************* Narai-MUD Died!!!  PID: $mudpid Exit: $mudexit"
			echo ""

			# echo "----------    Restore missing stash files"
			# # Restore vanished stash files -k : keep existing file
			# tar -xkf $usrdir/stash.tar > /dev/null 2>&1

			env LC_ALL=C date +"$datefmt"
			echo "==========    END     === End of LOG Run No. $COUNT ============"

		) >> $logfile 2>&1

		#  Clean up
		if  [ -f $pidfile ] ; then
			runfail=5
			rm -f $pidfile
		fi
		if [ -f $prog_run.core ] ; then
			runfail=6
			rm -f $prog_run.core
		fi
	fi

	if [ -s $logfile ] ; then
		tail -100 $logfile >> $logdir/CRASH-LOG

		lastlog="$logdir/mud_$TIMESTAMP-$COUNT.log"
		echo "Moving $logfile to $lastlog" >> $checkfile
		/bin/mv -f $logfile $lastlog
		rm -f $mudhome/$lastlink
		ln -s $usrdir/$lastlog $mudhome/$lastlink
	else
		echo "No log"
		runfail=7
	fi
	echo "" >> $checkfile

	if [ $runfail != 0 ] ; then exit $runfail; fi
	if [ $mudexit != 0 ] ; then exit $mudexit; fi

	#  Sleep > 60 secs before next run to wait port to be freed.
	sleep 100

	COUNT="`expr $COUNT + 1 `"
done
