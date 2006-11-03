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
#
# DESCRIPTION: Makefile for the P2 core elements python library
#

lib_LTLIBRARIES =			libp2pythoncore.la

libp2pythoncore_la_SOURCES =		\
					element.cpp \
					elementSpec.cpp \
					loggerI.cpp \
					plumber.cpp \
					tuple.cpp \
					table2.cpp \
					commonTable.cpp \
					value.cpp \
					val_double.cpp \
					val_id.cpp \
					val_int32.cpp \
					val_int64.cpp \
					val_ip_addr.cpp \
					val_null.cpp \
					val_opaque.cpp \
					val_str.cpp \
					val_time.cpp \
					val_tuple.cpp \
					val_uint32.cpp \
					val_uint64.cpp \
					val_vector.cpp \
					val_matrix.cpp \
					val_list.cpp \
					reporting.cpp \
					list.cpp


libp2pythoncore_la_CPPFLAGS =		@BOOST_INC@ \
					-I$(top_builddir)/p2core \
					-I$(top_builddir)/eventLoop

CLEANFILES =				core *.core *~

MAINTAINERCLEANFILES =			Makefile.in

libp2pythoncore_la_CXXFLAGS =		-DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG \
					@PYTHON_CSPEC@
