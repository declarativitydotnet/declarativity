
lib_LTLIBRARIES = libp2core.la
libp2core_la_SOURCES = \
	element.C \
	routerthread.C \
	timer.C \
	task.C \
	hooker.C \
	tuple.C \
	router.C \
	master.C \
	elementSpec.C \
	elemental.C \
	thooker.C \
	telemental.C \
	pel_lexer.C \
	pel_vm.C \
        IdTable.C

INCLUDES = @SFS_INC@

CLEANFILES = core *.core *~ *.gen.h *.gen.txt pel_lexer.C

EXTRA_DIST = .cvsignore

MAINTAINERCLEANFILES = Makefile.in

pel_opcode_decls.gen.h: pel_gen.py
	python pel_gen.py

pel_opcode_defns.gen.h: pel_gen.py
	python pel_gen.py

pel_opcode_tokens.gen.h: pel_gen.py
	python pel_gen.py

pel_opcode_descriptions.gen.txt: pel_gen.py
	python pel_gen.py
 
pel_lexer.C: pel_lexer.lex pel_opcode_tokens.gen.h
	$(LEX) $(AM_LFLAGS) $(LFLAGS) -opel_lexer.C -8 pel_lexer.lex


