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
# DESCRIPTION: Makefile for the P2 core library
#

lib_LTLIBRARIES = libp2core.la
libp2core_la_SOURCES = \
	element.C \
	routerthread.C \
	timer.C \
	task.C \
	tuple.C \
	router.C \
	master.C \
	elementSpec.C \
	pel_lexer.C \
	pel_vm.C \
        idTable.C \
	val_str.C \
	val_null.C \
	val_int32.C \
	val_uint32.C \
	val_int64.C \
	val_uint64.C \
	val_double.C

INCLUDES = @SFS_INC@

CLEANFILES = core *.core *~ *.gen.h *.gen.txt pel_lexer.C

EXTRA_DIST = .cvsignore

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
