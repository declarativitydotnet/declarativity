#!/bin/sh

case `hostname --fqdn` in
	grouchy.research.intel-research.net)
	# On grouchy, actually build a fresh tree...
	echo "On `hostname`; building..."
	CVSROOT=:ext:${USER}@blsshsvr.berkeley.intel-research.net:/projects/cvs	
	export CVSROOT
	cd /localhomes/${USER}/
	rm -rf phi
	cvs co phi/phi || exit 1
	cd phi/phi
	cvs update -d || exit 1
	./setup || exit 1
	./configure --with-sfsinc=/projects/phi/sfs-grouchy/include/sfs --with-sfslib=/projects/phi/sfs-grouchy/lib/sfs CXXFLAGS="-g -Wall" || exit 1
	(cd p2core; python ./pel_gen.py ) || exit 1
	cp -v ./p2core/FlexLexer.h.works ./p2core/FlexLexer.h
	touch ./p2core/FlexLexer.h
	cp -v ./p2core/pel_lexer.C.works ./p2core/pel_lexer.C
	touch ./p2core/pel_lexer.C
	make
	;;
*.research.intel-research.net)
	# If we're in research, ssh directly to grouchy.
	echo "On `hostname`; indirecting to grouchy..."
	exec ssh grouchy $0
	;;
*)
	# Otherwise, indirect through blsshsvr..
	echo "On `hostname`; indirecting through the ssh server..."
	exec ssh blsshsvr.berkeley.intel-research.net $0
	;;
esac
