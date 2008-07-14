ADD_LIBRARY(p2overlog parser_util.C ol_context.C ol_lexer.C ol_parser.C)

BISON_TARGET(P2Parser ${P2_SOURCE_DIR}/overlog/ol_parser.y ${P2_BINARY_DIR}/overlog/ol_parser.C)
FLEX_TARGET(P2Lexer ${P2_SOURCE_DIR}/overlog/ol_lexer.lex ${P2_BINARY_DIR}/overlog/ol_lexer.C)
#ADD_FLEX_BISON_DEPENDENCY(P2Lexer P2Parser)

# Include binary directory to include lexer.c in parser.c
INCLUDE_DIRECTORIES(${P2_BINARY_DIR}/overlog)

##
## COULD ALSO TRY http://public.kitware.com/pipermail/cmake/2002-September/003028.html
##
