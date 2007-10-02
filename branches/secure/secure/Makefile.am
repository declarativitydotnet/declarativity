########################################################################
#
#  @(#)$Id: Makefile.am 1334 2007-08-17 01:42:34Z princem $
#
# This file is distributed under the terms in the attached LICENSE file.
# If you do not find this file, copies can be found by writing to:
# Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
# Or
# UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
# Berkeley, CA,  94707. Attention: P2 Group.
#
# DESCRIPTION: Makefile for the P2 core library
#

lib_LTLIBRARIES =	libp2secure.la

SOURCE_FILE_ROOTS =	sfslite \
			fdbuf 


libp2secure_la_SOURCES =$(SOURCE_FILE_ROOTS:=.h) $(SOURCE_FILE_ROOTS:=.C) 



libp2secure_la_LDFLAGS =@BOOST_LIB@ \
			@LDFLAGS@ \
			-lboost_regex \
			-L/usr/local/lib/sfslite/shared/ \
			-lasync \
			-larpc \
			-lsfscrypt \
			-lsfsmisc \
			-ltame \
			-lgmp 

libp2secure_la_CXXFLAGS =-DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG

libp2secure_la_CPPFLAGS = @BOOST_INC@ \
			-I/usr/local/include/sfslite/shared/ 

CLEANFILES = *~ 

MAINTAINERCLEANFILES = Makefile.in
