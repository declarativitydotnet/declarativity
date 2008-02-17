########################################################################
#
#  @(#)$Id: Makefile.am 1243 2007-07-16 19:05:00Z maniatis $
#
# This file is distributed under the terms in the attached LICENSE file.
# If you do not find this file, copies can be found by writing to:
# Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
# Or
# UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
# Berkeley, CA,  94707. Attention: P2 Group.
#
# DESCRIPTION: CMakefile for the P2 datalog parser
#

ADD_LIBRARY(p2overlog parser_util.C ol_context.C ol_lexer.C ol_parser.C)

BISON_TARGET(P2Parser ${P2_SOURCE_DIR}/overlog/ol_parser.y ${P2_BINARY_DIR}/overlog/ol_parser.C)
FLEX_TARGET(P2Lexer ${P2_SOURCE_DIR}/overlog/ol_lexer.lex ${P2_BINARY_DIR}/overlog/ol_lexer.C)
#ADD_FLEX_BISON_DEPENDENCY(P2Lexer P2Parser)

# Include binary directory to include lexer.c in parser.c
INCLUDE_DIRECTORIES(${P2_BINARY_DIR}/overlog)
ADD_DEPENDENCIES(p2overlog p2core)

INSTALL(TARGETS p2overlog LIBRARY DESTINATION lib
			  ARCHIVE DESTINATION lib)
##
## COULD ALSO TRY http://public.kitware.com/pipermail/cmake/2002-September/003028.html
##
