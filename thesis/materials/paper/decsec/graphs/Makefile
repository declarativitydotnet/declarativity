FIGS = $(wildcard *.fig)
EPSFILES = $(FIGS:.fig=.eps)
PNGFILES = $(FIGS:.fig=.png)
GNUPLOTS = $(wildcard *.gp)
GPEPSES = $(GNUPLOTS:.gp=.eps)

%.eps: %.jgr.py
	python $< | jgraph > $@

%.eps: %.fig
	fig2dev -L eps $< $@

%.png: %.fig
	fig2dev -L png $< $@

%.eps: %.jgr
	jgraph $< > $@

all: $(EPSFILES) $(PNGFILES) $(GPEPSES)

%.eps: %.gp
	gnuplot $<

clean:
	rm -f $(EPSFILES) $(PNGFILES) $(GPEPSES)

