#!/bin/sh
#
# For debuging use only, not for production use
#

port=4001	# $usrdir/PORT is primary

prog=mud-kit
usrdir=$HOME/mud/users

pids="`ps -cx | grep $prog | cut -c1-6 `"

if [ "x$pids" != x ] ; then
	echo "Killing mud PID: $pids"
	kill -TERM  $pids
	sleep 1
fi

cd $usrdir

if [ -f PORT ] ; then
	port=`cat PORT`
fi

rm -f mud-$port.pid
rm -f $prog.run.core $prog.new.core
