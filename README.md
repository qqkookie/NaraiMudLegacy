Narai Mud Legacy
======================

This is source archive of Narai text MUD.
Narai is very old Korean text MUD, circa 1995.

It is in lineage of KIT-MUD, and its ancestor Diku-MUD, which is
precursor of modern Korean on-line RPG game.
Narai is direct heir of EVE MUD, a version of KIT-MUD.

It preserves tradition and history of old Korean text mud in the 1995 era.
It has easy-going, anything-goes style of Korean on-line games.
Think of Grand Theft Auto of 1995!
So nickname of Narai was aka kkag-pae mud, outlaw bully.

To make it run on modern Unix like Linux or FreeBSD,
I revised and modernized the original source of circa year 2000.

But its play is nearly identical to old, legacy Narai.
Zone, map, balance, commands are nearly same as 25 years ago.
It is well reserved history of old ancestor of modern online game.

## Versions

There are few versions of the source.

For current version and changes, see VERSION file.

Source git respository is
https://github.com/qqkookie/NaraiMudLegacy
ssh:git@github.com/qqkookie/NaraiMudLegacy

Legacy 2003, 2010, 2019 versions are maintained
as **`legacy`** branch in the **`git`** repository

### Legacy 2003

The oldest one is version used around year 2000-2003.
It was run on MS Windows with CygWin system.
To run it, you need CygWin DLL's,
**"cygwin1.dll"** and **"cygcrypt-0.dll"**.

But original Narai was run on Sun 3 Workstation,
(Motorola 68030 CPU, old SunOS based on BSD 4.3 before Solaris OS)
it can be easily ported back to BSD Unix varieties.
It is tagged as **legacy-2003**

### Legacy 2010

This version is nearly same as 2003 version, with little update and bug fix.
It runs on modern FreeBSD Unix OS and is being played today.
But it suffers from its old age, long neglect and poor maintenance.
From the beginning, Narai code was written very poorly and hard to maintain.
For Linux machine, little Makefile editing is needed,
due to difference between GNU `make` and BSD `make` syntax.
It is tagged as **legacy-2010** in **legacy** branch of git repository.

### Legacy 2019

To preserve history and legacy of Korean text MUD and Narai,
I, Cookie, revised it, based on Legacy 2010 version, for modern Unix
including Linux and FreeBSD and did little code cleanup and bug fix.
It preserves same user expectation of 1995 era,
but improved for modern environment like Linux, `gcc/clang`, GNU `make`,
UTF-8 character code and 64 bit CPU. Ported to modern Linux environment.
This is the version this archive contains now.
It is tagged as **legacy-2019** in **legacy** branch of git repository.

### KIT-MUD 2002

This is a fork of original KIT-MUD, known as **"Coconut MUD"**,
which was forked around 1999 and operated until 2002.
It is not outlaw-oriented, but more traditional and closer to original KIT-MUD.
It shares common ancenstor of KIT-MUD with Narai but added
few new zones like Rome and preserved old KIT mud school zone.
It also did quite a code clean up and improvement.
Not directly related to old Legacy Narai or New Narai 1998.
It is tagged as **coconut-2002** in **coconut** branch of git repository.

### New Narai 1998

This is other fork of original Narai source year 1997 and operated till 1998.
The source code was extensively cleaned up and many improvements were made.
It added many new features and improved quality of mud life,
but preserves similar user experience of original Sun 3 days.
It used to be referred as **"New Narai"**.

But text MUD itself was already out of date by then,
and eclipsed by modern graphical PC/console online games.
Not many people played it, so it become urban legend.

It is tagged as **newnarai-1998** in **master** branch of git repository.

### KIT Classic 2019

I plan to restart new development based on New Narai 1998 version
and merge all the versions and forks above and
make it definitive classic version of KIT MUD.

## Directories and contents

* `'src'` and `'include'` are source code written in old ANSI C.
* `'utils'` contains source code of tools and scripts
  for operation and game development.
* `'lib'` is directory where mud server actually run in,
  containing all static game data like zone and mud resources.
* `'users'` is directory where logs, player info and item stashes are saved.
* `'OLD'` contains unused code and data like old version,
  back up, and left-overs collected here for history.

## Current Narai

You can play the Legacy 2010 version and 2019 version
using telnet on narai.synology.me port 5001 and 5002.
5001 is using EUC-KR code. Port 5002, UTF-8 code.

You can contact operator and players on Kakao Talk messenger open group:

[https://open.kakao.com/o/gAx75aN](https://open.kakao.com/o/gAx75aN)

[User BBS forum](http://narai.forumkorean.com) is not so active.

