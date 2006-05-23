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

PYTHONPATH = $(top_builddir)/python/dfparser/yapps:$(top_builddir)/python/lib

all: dataflow.g
	@PYTHON@ $(top_builddir)/python/dfparser/yapps/yapps2.py dataflow.g dfparser.py

CLEANFILES = core *.core *.pyc dfparser.py *.dot *.ps

EXTRA_DIST = dataflow.g

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = yapps
