# ~/.profile: executed by the command interpreter for login shells.  
#
# shell environment for Narai Mud security.

# Restrict group and others access.
umask 0077

# set PATH to include mud/bin 

if [ -d "$HOME/mud/bin" ] ; then
    PATH="$PATH:$HOME/mud/bin"
fi

export LC_ALL=ko_KR.UTF-8
export LANG=ko_KR.UTF-8

export BLOCKSIZE=K 

alias lt="ls -ltr"
alias cp="cp -ip"
alias mv="mv -i"
alias vi="view"
alias ps="ps -x"

export PAGER=less
alias m="$PAGER"

