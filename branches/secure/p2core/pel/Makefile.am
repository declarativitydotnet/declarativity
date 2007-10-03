########################################################################
#
#  @(#)$Id: Makefile.am 1391 2007-09-26 19:12:58Z maniatis $
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

lib_LTLIBRARIES =	libp2pel.la

SOURCE_FILE_ROOTS =	pel_lexer \
			pel_vm \
			secureUtil \
			secureUtil1 


libp2pel_la_SOURCES =	$(SOURCE_FILE_ROOTS:=.h) $(SOURCE_FILE_ROOTS:=.C) \
			pel_lexer.lex \
			pel_gen.py \
			pel_program.h 



libp2pel_la_LDFLAGS =	@BOOST_LIB@ \
			@LDFLAGS@ \
			@OPENSSL_LIB@ \
			-lboost_regex 

libp2pel_la_CXXFLAGS =	-DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG

libp2pel_la_CPPFLAGS = @BOOST_INC@ \
			@OPENSSL_INC@ \
			-I../ \
			-I../secureUtil

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


