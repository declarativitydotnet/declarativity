#!/bin/sh

cvs update -d || exit 1
make clean
make distclean
./setup || exit 1
./configure --with-sfsinc=/projects/phi/sfs-grouchy/include/sfs --with-sfslib=/projects/phi/sfs-grouchy/lib/sfs CXXFLAGS="-g -Wall" || exit 1
(cd p2core; python ./pel_gen.py ) || exit 1
cp -v ./p2core/FlexLexer.h.works ./p2core/FlexLexer.h
touch ./p2core/FlexLexer.h
cp -v ./p2core/pel_lexer.C.works ./p2core/pel_lexer.C
touch ./p2core/pel_lexer.C
make 
