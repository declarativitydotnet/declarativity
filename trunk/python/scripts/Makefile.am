########################################################################
#
#  @(#)$Id$
#
# This file is distributed under the terms in the attached LICENSE file.
# If you do not find this file, copies can be found by writing to:
# Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
# Or
# UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
# Berkeley, CA,  94707. Attention: P2 Group.
#

bin_SCRIPTS = p2stub.py p2terminal.py p2insert.py loadManyChords.py dfrun.py

do_subst = sed -e 's,[@]PYTHON[@],$(PYTHON),g'

p2stub.py: p2stub.in Makefile
		   $(do_subst) < $(srcdir)/p2stub.in > p2stub.py

p2terminal.py: p2terminal.in Makefile
		   $(do_subst) < $(srcdir)/p2terminal.in > p2terminal.py

p2insert.py: p2insert.in Makefile
		   $(do_subst) < $(srcdir)/p2insert.in > p2insert.py

loadManyChords.py: loadManyChords.in Makefile
		   $(do_subst) < $(srcdir)/loadManyChords.in > loadManyChords.py

CLEANFILES = $(bin_SCRIPTS)

EXTRA_DIST = README \
             p2stub.in p2terminal.in loadManyChords.in p2insert.in dfrun.in \
             sanity.py dfrun.py netcc.py chord.olg \
             recv.df sender.df
