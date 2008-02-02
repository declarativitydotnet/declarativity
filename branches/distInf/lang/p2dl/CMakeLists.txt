########################################################################
#
# This file is distributed under the terms in the attached LICENSE file.
# If you do not find this file, copies can be found by writing to:
# Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
# Or
# UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
# Berkeley, CA,  94707. Attention: P2 Group.
#
#

ADD_LIBRARY(p2dl
            p2dlContext.C
            p2dl_lexer.C
            p2dl_parser.C
)

BISON_TARGET(P2dlParser ${P2_SOURCE_DIR}/lang/p2dl/p2dl_parser.y ${P2_BINARY_DIR}/lang/p2dl/p2dl_parser.C)
FLEX_TARGET(P2dlLexer ${P2_SOURCE_DIR}/lang/p2dl/p2dl_lexer.lex ${P2_BINARY_DIR}/lang/p2dl/p2dl_lexer.C)
#ADD_FLEX_BISON_DEPENDENCY(P2dlLexer P2dlParser)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

INSTALL(TARGETS p2dl LIBRARY DESTINATION lib)
