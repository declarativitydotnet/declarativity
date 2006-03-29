########################################################################
#
#  @(#)$Id$
#
# Copyright (c) 2005 Intel Corporation. All rights reserved.
#
# This file is distributed under the terms in the attached INTEL-LICENSE file.
# If you do not find these files, copies can be found by writing to:
# Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
#
#

export PYTHONPATH=$(top_builddir)/python/dfparser/yapps:$(top_builddir)/python/lib

all: dataflow.g
	python $(top_builddir)/python/dfparser/yapps/yapps2.py dataflow.g dfparser.py

CLEANFILES = core *.core *.pyc dfparser.py *.dot *.ps
EXTRA_DIST = .cvsignore

MAINTAINERCLEANFILES = Makefile.in
