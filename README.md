Narai Mud Legacy
======================

This is source repository of Narai text Mud.
Narai is very old Korean text MUD, circa 1995.

It is in lineage of KIT-MUD, which is precursor of
modern Korean on-line RPG game, and direct heir of EVE MUD.

It preserves tradition and history of old Korean text mud in the 1995 era.
It has easy-going, anything-goes style of Korean on-line games.
Think of Grand Theft Auto of 1995!
So nickname of Narai was aka kkagpae mud, outlaw bully.

To make it run on modern Unix like FreeBSD or Linux,
I revised and modernized the original source of circa year 2000.

But its play is nearly identical to old, legacy Narai.
Zone, map, balance, commands are nearly same after 25 years ago.
It is well reserved history of old ancestor of modern online game.

## Versions

There are few versions of the source.

**VERSION COMMITTED NOW is merged version of Legacy 2010 and Legacy2003**

### Legacy 2003

The oldest one is version used around year 2000-2003.
It was run on MS Windows with CygWin system.
To run it, you need CygWin DLL's,
**"cygwin1.dll"** and **"cygcrypt-0.dll"**.

But original Narai was run on Sun 3 Workstation,
(Motorola 68030 CPU, old SunOS based on BSD 4.3 before Solaris OS)
it can be easily ported back to BSD Unix varieties.

### Legacy 2010

This version is nearly identical to 2003 version,
with little update and bug fix.
It runs on modern FreeBSD Unix OS and is being played today.
But it suffers from its old age, long neglect and poor maintenance.
From the beginning, Narai code was written very poorly and hard to maintain. 
For Linux machine, little Makefile editing is needed,
due to difference between GNU make and BSD make syntax.

### Modern Legacy 2019

To preserve history and legacy of Korean text MUD and Narai,
I revised it, based on Legacy 2010 version, for modern Unix
including Linux and FreeBSD and did little code clean up and bug fix.
It preserves same user expectation of 1995 era
but improved for modern environment like Linux, gcc, GNU make,
UTF-8 code and 64 bit CPU.

This is on-going effort and work in progress, is in alpha stage.

### Classic 1998

This is other fork of original Narai source around year 1998.
The source code was extensively clean up and many improvements were made.
It added many new features and improved quality of mud life,
but preserves similar user experience of original Sun 3 days.

But text MUD itself was already out of date by then,
and eclipsed by modern graphical PC/console online games.
Not many people played it, so it become urban legend.

## Directories and contents

`'src'` and `'include'` are C source code in old ANSI C.
`'utils'` contains source code of tools and scripts
for operation and game development.
`'lib'` is directory where mud server actually run in, 
containing all game data like zone, and player info and items.
You will need `'bin'` and `'log'` directory for mud operation.
`'OLD'` contains unused code and data like old version,
back up, and left-overs collected here for history. 
 
## Current Narai

You can play the Legacy 2010 version and 2019 version
using telnet on narai.mynetgear.com port 5001 and 5002.
5001 is using EUC-KR code. Port 5002, UTF-8 code.

You can contact operator and players on Kakao Talk messenger open group:

[https://open.kakao.com/o/gAx75aN](https://open.kakao.com/o/gAx75aN)

[User BBS forum](http://narai.forumkorean.com) is not so active.

