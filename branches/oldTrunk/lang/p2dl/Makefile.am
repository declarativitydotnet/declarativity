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
# DESCRIPTION: Makefile for the P2 datalog parser
#

lib_LTLIBRARIES = libp2dl.la

GENERATED_SOURCES = \
	p2dl_parser.H \
	p2dl_lexer.C \
	p2dl_parser.C

REAL_SOURCES = \
	p2dlContext.C

libp2dl_la_SOURCES = \
	$(REAL_SOURCES) \
	$(GENERATED_SOURCES)

INCLUDES = @BOOST_INC@ \
           -I../ -I../../p2core -I../../elements -I../../net -I../../p2cpp

CLEANFILES = core *.core *~ *.gen.h *.gen.txt p2dl_lexer.C \
	p2dl_parser.output p2dl_parser.H p2dl_parser.C \
	*.works

EXTRA_DIST = .cvsignore \
		$(REAL_SOURCES:.C=.h) \
		p2dl_lexer.lex \
		p2dl_parser.y \
		p2dl_lexer.h


MAINTAINERCLEANFILES = Makefile.in 

p2dl_parser.C: p2dl_parser.y p2dl_lexer.C
	$(YACC) -t -y p2dl_parser.y -o p2dl_parser.C

p2dl_parser.H: p2dl_parser.C

p2dl_lexer.C: p2dl_lexer.lex
	$(LEX) $(AM_LFLAGS) $(LFLAGS) -op2dl_lexer.C -8 p2dl_lexer.lex

p2dlContext.C: p2dl_parser.H
