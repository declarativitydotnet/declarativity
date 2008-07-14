#!/bin/sh

IN_FILE="$1"
OUT_FILE=`echo $IN_FILE | sed 's/.y\$/.C/'`
TAB_HFILE=`echo $IN_FILE | sed 's/.y\$/.tab.h/'`
TAB_CFILE=`echo $IN_FILE | sed 's/.y\$/.tab.c/'`

echo "Trying to find a bison to convert $IN_FILE to $OUT_FILE"
for i in /usr/local/bin /usr/bin /bin ; do
    if [ ! -x "$i/bison" ]; then continue; fi
    case "`$i/bison --version` | grep '(GNU Bison)'" in 
    *1.87[5-9]*)
        echo "Found a suitable bison in $i, running it..."
	$i/bison -y $IN_FILE && cp $TAB_CFILE $OUT_FILE
	for f in $OUT_FILE $TAB_HFILE; do 
	    echo "Copying $f to $f.works ..."
	    cp -v $f ${f}.works
	done
	exit 0
	;;
    esac
done
echo "Couldn't find a reasonable bison; fixing things up..."
for f in $OUT_FILE $TAB_HFILE; do 
    echo "Copying $f.works to $f ..."
    cp -v $f.works ${f}
    touch $f
done
