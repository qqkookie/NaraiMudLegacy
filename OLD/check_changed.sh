d1=narai-utf8
d2=working

for ff in `cat /tmp/f1`
do 
    f1=$d1/$ff
    f2=$d2/$ff

    if [ -f $f1 -a -f $f2 ] ; then
       if cmp -s $f1 $f2 ; then
        ls -l $f1 >> samefile
        ls -l $f2 >> samefile
           echo same file
         touch -r $f1 $f2
       else     
        ls -l $f1 >> difffile
        ls -l $f2 >> difffile
           echo diff file
       fi
    fi
done
    
