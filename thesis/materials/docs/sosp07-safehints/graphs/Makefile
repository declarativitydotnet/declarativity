GNUPLOTS =		$(wildcard *.gnuplot)

GNUEPSES =		$(addsuffix .eps, $(basename $(GNUPLOTS)))


all:			$(GNUEPSES)

clean:
	${RM} $(GNUEPSES)

%.eps :			%.gnuplot
	gnuplot $<



