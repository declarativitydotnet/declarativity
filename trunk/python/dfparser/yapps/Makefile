all: $(shell echo *.g | sed s/.g/.py/g )

%.py: %.g yapps2.py yappsrt.py Makefile
	python yapps2.py $<

clean:
	${RM} -f *.pyc

distclean:
	${RM} -f *.pyc


