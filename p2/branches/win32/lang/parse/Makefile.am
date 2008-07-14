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

lib_LTLIBRARIES = libp2parse.la

GENERATED_SOURCES = \
	olg_parser.H \
	olg_lexer.C \
	olg_parser.C

REAL_SOURCES = \
	parseContext.C

libp2parse_la_SOURCES = \
	$(REAL_SOURCES) \
	$(GENERATED_SOURCES)

INCLUDES = @PYTHON_CSPEC@ @BOOST_INC@ \
           -I../ -I../../p2core -I../../elements -I../../net -I../../p2cpp

CLEANFILES = core *.core *~ *.gen.h *.gen.txt olg_lexer.C \
	olg_parser.output olg_parser.H olg_parser.C \
	*.works

EXTRA_DIST = .cvsignore \
		$(REAL_SOURCES:.C=.h) \
		olg_lexer.lex \
		olg_parser.y \
		olg_lexer.h


MAINTAINERCLEANFILES = Makefile.in 

olg_parser.C: olg_parser.y olg_lexer.C
	$(YACC) -t -y olg_parser.y -o olg_parser.C

olg_parser.H: olg_parser.C

olg_lexer.C: olg_lexer.lex
	$(LEX) $(AM_LFLAGS) $(LFLAGS) -oolg_lexer.C -8 olg_lexer.lex

parseContext.C: olg_parser.H
