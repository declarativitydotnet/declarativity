## Why a makefile?  Because ant's junit task is impossible to debug.

RUNTEST = /usr/bin/time java -cp dist/bfs.jar:lib/junit.jar:lib/jol.jar:dist/bfs-test.jar
RUNTESTJUNIT = ${RUNTEST} org.junit.runner.JUnitCore 
RUNTESTSTASIS = LD_LIBRARY_PATH=$(STASIS_DIR)/build/src/stasis $(RUNTEST) -Djava.library.path=$(JOL_DIR)/ant-build/stasis/jni
RUNTESTSTASISJUNIT = ${RUNTESTSTASIS} org.junit.runner.JUnitCore
RUNTESTSTASISCPU = ${RUNTESTSTASIS} -agentlib:hprof=cpu=samples,depth=6 org.junit.runner.JUnitCore
RUNTESTSTASISHEAP = ${RUNTESTSTASIS} -agentlib:hprof=heap=sites,depth=8 org.junit.runner.JUnitCore

.PHONY: TESTPREP
.PHONY: %.test
.PHONY: test clean-test

TARGETS=$(shell ls test/bfs/test/ | grep Test\\\.java | perl -pe 's/\.java/.test/g')
SLOWTARGETS=$(shell ls test/bfs/test/ | grep Test\\\.java | perl -pe 's/\.java/.slowtest/g')
STASISTARGETS=$(shell ls test/bfs/test/ | grep Test\\\.java | perl -pe 's/\.java/.stasistest/g')
CPUTARGETS=$(shell ls test/bfs/test/ | grep Test\\\.java | perl -pe 's/\.java/.cputest/g')
HEAPTARGETS=$(shell ls test/bfs/test/ | grep Test\\\.java | perl -pe 's/\.java/.heaptest/g')

test: ${TARGETS}
test-slow: ${SLOWTARGETS}
test-stasis: ${STASISTARGETS}
test-profile: ${PROFILETARGETS}


%.test: TESTPREP
	@echo -n `date`: Running $*
	@${RUNTESTJUNIT} bfs.test.$* 2> test-output/$*.txt 1>&2 || echo -n '      <---- FAILED'
	@echo

%.slowtest: TESTPREP
	@echo -n `date`: Running [slow] $*
	@${RUNTEST} bfs.test.$* 2> test-output/$*.slow.txt 1>&2 || echo -n '      <---- FAILED'
	@echo

%.stasistest: TESTPREP
	@rm -f storefile.txt logfile.txt
	@echo -n `date`: Running [stasis] $*
	@${RUNTESTSTASISJUNIT} bfs.test.$* 2> test-output/$*.stasis.txt 1>&2 || echo -n '      <---- FAILED'
	@echo

%.cputest: TESTPREP
	@rm -f storefile.txt logfile.txt
	@echo -n `date`: Running [cpu prof] $*
	@${RUNTESTSTASISCPU} bfs.test.$* 2> test-output/$*.stasis.txt 1>&2 || echo -n '      <---- FAILED'
	@echo

%.heaptest: TESTPREP
	@rm -f storefile.txt logfile.txt
	@echo -n `date`: Running [heapprof] $*
	@${RUNTESTSTASISHEAP} bfs.test.$* 2> test-output/$*.stasis.txt 1>&2 || echo -n '      <---- FAILED'
	@echo

TESTPREP: clean-test
	ant -q -e
	mkdir -p test-output

clean-test:
	rm -f test-output/*.txt
