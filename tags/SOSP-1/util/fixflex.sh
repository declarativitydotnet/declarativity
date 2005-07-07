#!/bin/sh

IN_FILE="$3"
OUT_FILE=`echo $IN_FILE | sed 's/.lex\$/.C/'`
WORKING_FILE="$OUT_FILE.works"

echo "Trying to find a flexer to convert $IN_FILE to $OUT_FILE"
for i in /usr/local/bin /usr/bin /bin ; do
    if [ ! -x "$i/flex" ]; then continue; fi
    case "`$i/flex --version`" in 
    *2.5.3[1-9])
        echo "Found a suitable flex in $i, running it..."
	$i/flex -d $* &&
	echo "Copying $OUT_FILE to $WORKING_FILE ..." &&
	cp -v $OUT_FILE $WORKING_FILE &&
	exit 0
	;;
    esac
done
echo "Couldn't find a reasonable flex; fixing things up..."
IN_FILE="$3"
OUT_FILE=`echo $IN_FILE | sed 's/.lex\$/.C/'`
cp -v $OUT_FILE.works $OUT_FILE &&
cp -v FlexLexer.h.works FlexLexer.h 
touch $OUT_FILE

