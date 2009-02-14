## Why a makefile?  Because ant's junit task is impossible to debug.

RUNTEST = java -cp dist/bfs.jar:lib/junit.jar:lib/jol.jar:dist/bfs-test.jar

.PHONY: TESTPREP
.PHONY: %.test
.PHONY: test clean-test

TARGETS=$(shell ls test/bfs/test/ | grep Test\\\.java | perl -pe 's/\.java/.test/g')

test: ${TARGETS}

%.test: TESTPREP
	@echo -n `date`: Running $*
	@${RUNTEST} bfs.test.$* 2> test-output/$*.txt 1>&2 || echo -n '      <---- FAILED'
	@echo

TESTPREP: clean-test
	ant -q -e

clean-test:
	rm -f test-output/*.txt
