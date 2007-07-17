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

lib_LTLIBRARIES =	libp2core.la

SOURCE_FILE_ROOTS =	reporting \
			tableManager \
			commonTable \
			compileUtil \
			table2 \
			refTable \
			aggFactory \
			val_str \
			value \
			fdbuf \
			xdrbuf \
			ID \
			element \
			tuple \
			plumber \
			elementSpec \
			pel_lexer \
			pel_vm \
			val_null \
			val_int32 \
			val_uint32 \
			val_int64 \
			val_uint64 \
			val_double \
			val_opaque \
			val_tuple \
			val_time \
			val_id \
			loggerI \
			oper \
			val_ip_addr \
			dot \
			list \
			val_list \
			val_vector \
			CSVlex \
			val_matrix \
			loop \
			scheduler \
			p2Time \
			commitManager


libp2core_la_SOURCES =	$(SOURCE_FILE_ROOTS:=.h) $(SOURCE_FILE_ROOTS:=.C) \
			inlines.h \
			pel_lexer.lex \
			pel_gen.py \
			pel_program.h \
			systemTable.h \
			iStateful.h \
			iRunnable.h			


libp2core_la_LDFLAGS =	@BOOST_LIB@ \
			@LDFLAGS@ \
			@OPENSSL_LIB@
			-lboost_regex \
			-lssl

libp2core_la_CXXFLAGS =	-DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG

libp2core_la_CPPFLAGS = @BOOST_INC@ \
			@OPENSSL_INC@

CLEANFILES = core *.core *~ *.gen.h *.gen.txt pel_lexer.C

MAINTAINERCLEANFILES = Makefile.in



pel_vm.C : pel_opcode_decls.gen.h


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


