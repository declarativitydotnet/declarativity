
lib_LTLIBRARIES = libp2core.la
libp2core_la_SOURCES = \
	routerthread.C \
	timer.C \
	task.C \
	hooker.C \
	tuple.C \
	element.C \
	router.C \
	master.C \
	elementSpec.C \
	elemental.C \
	pel_lexer.C \
	pel_vm.C

INCLUDES = @SFS_INC@

CLEANFILES = core *.core *~ *.gen.h pel_lexer.C

EXTRA_DIST = .cvsignore

MAINTAINERCLEANFILES = Makefile.in

pel_opcode_decls.gen.h: pel_gen.py
	python pel_gen.py

pel_opcode_defns.gen.h: pel_gen.py
	python pel_gen.py

pel_opcode_tokens.gen.h: pel_gen.py
	python pel_gen.py
 
pel_lexer.C: pel_lexer.lex pel_opcode_tokens.gen.h
	$(LEX) $(AM_LFLAGS) $(LFLAGS) -o pel_lexer.C -8 pel_lexer.lex


