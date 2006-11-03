########################################################################
#
# This file is distributed under the terms in the attached LICENSE file.
# If you do not find this file, copies can be found by writing to:
# Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
# Or
# UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
# Berkeley, CA,  94707. Attention: P2 Group.
#  
#

lib_LTLIBRARIES =			libp2pythoncpp.la

libp2pythoncpp_la_SOURCES =		\
					p2.cpp


libp2pythoncpp_la_CPPFLAGS =		@BOOST_INC@ \
					-I$(top_builddir)/p2core \
					-I$(top_builddir)/elements \
					-I$(top_builddir)/overlog \
					-I$(top_builddir)/p2cpp \
					-I$(top_builddir)/eventLoop

CLEANFILES =				core *.core *~

MAINTAINERCLEANFILES =			Makefile.in

libp2pythoncpp_la_CXXFLAGS =		-DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG \
					@PYTHON_CSPEC@



