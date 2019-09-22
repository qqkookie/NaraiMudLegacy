#!/bin/sh
#
# For debuging use only, not for production use
#

port=5002	# $usrdir/PORT is primary

prog=mud-narai
usrdir=$HOME/mud/lib

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
