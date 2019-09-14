#!/bin/sh
#
# For debuging use only, not for production use
#

port=5002
prog=mud-narai
usrdir=$HOME/mud/lib

pids="`ps -cux | grep $prog | cut -f 2 -w `"

if [ "x$pids" != x ] ; then
	echo "Killing mud PID: $pids"
	kill -1  $pids
	sleep 1
fi

cd $usrdir

rm -f mud-$port.pid
rm -f $prog.run.core $prog.new.core
