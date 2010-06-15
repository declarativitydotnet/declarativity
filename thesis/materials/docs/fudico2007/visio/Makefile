PRNS =			$(wildcard *.prn)

VISIOEPSES =		$(addsuffix .eps, $(basename $(PRNS)))

SHRINK = 		./shrink.sh


all:			$(VISIOEPSES)

clean:
	${RM} $(VISIOEPSES)

%.eps :			%.prn Figures
	${SHRINK} $< Figures


