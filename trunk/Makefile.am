#####################################################################
#
# This file is distributed under the terms in the attached LICENSE file.
# If you do not find this file, copies can be found by writing to:
# Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
# Or
# UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
# Berkeley, CA,  94707. Attention: P2 Group.
# 
# DESCRIPTION: Makefile P2
# 
#   $Id$
# 
#

SUBDIRS=eventLoop p2core elements net overlog planner p2cpp debugger tests unitTests \
	python mapreduce

EXTRA_DIST = LICENSE setup \
		doc/chord.olg doc/narada.olg doc/ping.olg \
		doc/reachable.olg doc/types.txt README README.OSX \
		doc/dataflow.ps doc/dataflow.pdf

CXXFLAGS=-DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG -Wall @PYTHON_CSPEC@ @CXXFLAGS@

LDFLAGS=@BOOST_LIB@ @PYTHON_LSPEC@ @LDFLAGS@ 

