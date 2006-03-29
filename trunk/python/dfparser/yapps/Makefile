ALL: $(shell echo *.g | sed s/.g/.py/g )

%.py: %.g yapps2.py yappsrt.py Makefile
	python yapps2.py $<
