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
	ol_parser.H \
	ol_lexer.C \
	ol_parser.C

REAL_SOURCES = \
	parseContext.C

libp2parse_la_SOURCES = \
	$(REAL_SOURCES) \
	$(GENERATED_SOURCES)

INCLUDES = @PYTHON_CSPEC@ @BOOST_INC@ \
           -I../ -I../../p2core -I../../elements -I../../net -I../../p2cpp

CLEANFILES = core *.core *~ *.gen.h *.gen.txt ol_lexer.C \
	ol_parser.output ol_parser.H ol_parser.C \
	*.works

EXTRA_DIST = .cvsignore \
		$(REAL_SOURCES:.C=.h) \
		ol_lexer.lex \
		ol_parser.y \
		ol_lexer.h


MAINTAINERCLEANFILES = Makefile.in 

ol_parser.C: ol_parser.y ol_lexer.C
	$(YACC) -t -y ol_parser.y -o ol_parser.C

ol_parser.H: ol_parser.C

ol_lexer.C: ol_lexer.lex
	$(LEX) $(AM_LFLAGS) $(LFLAGS) -ool_lexer.C -8 ol_lexer.lex

parseContext.C: ol_parser.H
