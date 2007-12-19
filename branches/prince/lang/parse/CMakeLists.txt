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

ADD_LIBRARY(p2parse olg_lexer.C olg_parser.C
             parseContext.C
)
BISON_TARGET(P2Parser2 ${P2_SOURCE_DIR}/lang/parse/olg_parser.y ${P2_BINARY_DIR}/lang/parse/olg_parser.C)
FLEX_TARGET(P2Lexer2 ${P2_SOURCE_DIR}/lang/parse/olg_lexer.lex ${P2_BINARY_DIR}/lang/parse/olg_lexer.C)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
