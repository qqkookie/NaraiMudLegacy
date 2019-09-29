#!/usr/bin/perl
# autorun -- maintain a Circle V3.0 mud server
# Copyright (c)1995 Vi'Rage Studios
#
# Modified by JaeMok Jeong 
# 
# This program is under GPL license.
#
# csh original by Fred C. Merkel, PERL conversion by D. Hall
# .fastboot	sleep for only 5 seconds, instead of the default 20
# .killscript	script will exit, and mud will not reboot
# .pause	pause (sleep 1 minute intervals) until .pause it removed

$port	= 5000;

$home	= "/export/jmjeong/kit-mud/new-server";
$bin	= "$home/";
$lib	= "$home/lib";
$log	= "$home/log";

chdir $home;

while (1) {

  # Open SYSLOG and dup STDERR into SYSLOG
  use File::Path;

  mkpath ("./log");
  open (SYSLOG, ">> log/syslog") or die "Can't open file : $!";
  open (STDERR, ">& SYSLOG") or die "Can't open file : $!";
  
  print SYSLOG "autoscript starting game ", `date`;
  open (SERVER, "./kit $port |") or die "Pipe failed";
  
  while (<SERVER>) {
    print SYSLOG;
  }
  close(SERVER);
  close(SYSLOG);
  
  # First we open everything
  open (SYSLOG, "< log/syslog") or die "Can't open file : $!";
  open (DEATHS, ">>log/deaths") or die "Can't open file : $!";
  open (REBOOT, ">>log/reboots") or die "Can't open file : $!";
  open (LEVELS, ">>log/levels") or die "Can't open file : $!";
  open (NEWPLR, ">>log/newplrs") or die "Can't open file : $!";
  open (SYSERR, ">>log/errors") or die "Can't open file : $!";
  open (GODCMD, ">>log/godcmds") or die "Can't open file : $!";
  open (CHAT,   ">>log/chat") or die "Can't open file : $!";
  
  print DEATHS "*** LOGGING TIME : ", `date`;
  print REBOOT "*** LOGGING TIME : ", `date`;
  print LEVELS "*** LOGGING TIME : ", `date`;
  print NEWPLR "*** LOGGING TIME : ", `date`;
  print SYSERR "*** LOGGING TIME : ", `date`;
  print GODCOMD "*** LOGGING TIME : ", `date`;
  print CHAT "*** LOGGING TIME : ", `date`;

  # Then we stash everything
  while (<SYSLOG>) {
    print DEATHS if /killed/;
    print REBOOT if /Running/;
    print LEVELS if /<INFO>/;
    print NEWPLR if /new player/;
    print SYSERR if /SIG/;
    print GODCMD if /Timber/;
    print CHAT if />  /;
  }
  close(SYSLOG);
  close(DEATHS);
  close(REBOOT);
  close(LEVELS);
  close(NEWPLR);
  close(SYSERR);
  close(GODCMD);
  close(CHAT);
  
  # Rotate SYSLOG files
  unlink ('log/syslog.6');
  rename ('log/syslog.5', 'log/syslog.6');
  rename ('log/syslog.4', 'log/syslog.5');
  rename ('log/syslog.3', 'log/syslog.4');
  rename ('log/syslog.2', 'log/syslog.3');
  rename ('log/syslog.1', 'log/syslog.2');
  rename ('log/syslog'  , 'log/syslog.1');

  # added by jmjeong@oopsla.snu.ac.kr
  use File::Copy;

  if (-r '.newzone') {
	  foreach $i (<lib/tinyworld.new/*>) {
		  # copy new tinyworld file
		  copy($i, "lib/");
	  }
  }

  $date = `date`;
  @data = split(/\s+/, $date);
  system "tar zcf KITMUD-$data[1]-$data[2].tar.gz lib";
  if ( !($data[2] % 7) ) {
	# every week, copy the backup to the safe area
	copy("KITMUD-$data[1]-$data[2].tar.gz",
		 "/export/jmjeong/kit-mud/new-kit-server.tar.gz");
  }

  @delete_files = <KITMUD-*-*.tar.gz>;

  foreach $i (@delete_files) {
	# last modification time
	$last =  (time - (stat($i))[9]) / (60*60*24);
	if ($last > 7) {
	  unlink $i;
	}
  }
  
  # should we stay dead?
  if (-r '.killscript') {
	#  unlink '.killscript';
    open (SYSLOG, '>> log/syslog.1');
    print SYSLOG "autoscript killed ", `date`;
	close(SYSLOG);
    exit;
  }
  
  # or just play dead?
  while (-r '.pause') {
    sleep 60;
  }

  # or reboot as soon as possible?
  if (-r '.fastboot') {
	#  unlink '.fastboot';
    sleep 5;
  } else {
    sleep 20
  }
}
