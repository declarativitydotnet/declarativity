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
# DESCRIPTION: Makefile for the P2 core library
#

lib_LTLIBRARIES = libp2core.la

libp2core_la_SOURCES = \
	table2.C \
	val_str.C \
	value.C \
	fdbuf.C \
	xdrbuf.C \
	ID.C \
	element.C \
	tuple.C \
	plumber.C \
	elementSpec.C \
	pel_lexer.C \
	pel_vm.C \
	val_null.C \
	val_int32.C \
	val_uint32.C \
	val_int64.C \
	val_uint64.C \
	val_double.C \
	val_opaque.C \
	val_tuple.C \
	val_time.C \
	val_id.C \
	table.C \
	loggerI.C \
	oper.C \
	val_ip_addr.C \
	dot.C \
	list.C \
	val_list.C 

INCLUDES = @BOOST_INC@ -I../eventLoop

LIBS = -lboost_regex

CLEANFILES = core *.core *~ *.gen.h *.gen.txt pel_lexer.C

EXTRA_DIST = .cvsignore \
		$(libp2core_la_SOURCES:.C=.h) \
		inlines.h \
		trace.h \
		iteratorObj.h \
		scanIteratorObj.h \
		aggregateObj.h \
		pel_lexer.lex \
		pel_gen.py \
		pel_program.h

MAINTAINERCLEANFILES = Makefile.in

pel_opcode_decls.gen.h: pel_gen.py
	@PYTHON@ pel_gen.py

pel_opcode_defns.gen.h: pel_gen.py
	@PYTHON@ pel_gen.py

pel_opcode_tokens.gen.h: pel_gen.py
	@PYTHON@ pel_gen.py

pel_opcode_descriptions.gen.txt: pel_gen.py
	@PYTHON@ pel_gen.py

pel_lexer.C: pel_lexer.lex pel_opcode_tokens.gen.h
	$(LEX) $(AM_LFLAGS) $(LFLAGS) -opel_lexer.C -8 pel_lexer.lex

CXXFLAGS=-g -Wall -DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG -DTRACE_OFF @CXXFLAGS@

LDFLAGS=@BOOST_LIB@ @LDFLAGS@
