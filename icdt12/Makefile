###########################################################
#
# Makefile for LaTeX docs
#
###########################################################

.SUFFIXES:
.SUFFIXES: .ps .dvi .tex

TEXINPUTS := .:${TEXINPUTS}
BIBINPUTS := .:${TEXINPUTS}

#all: eps dvi ps pdf # res
all: pdf
eps: fig obj

PAPER=icdt12

LATEX=pdflatex

ps:  $(PAPER).ps

pdf: $(PAPER).pdf
dvi: $(PAPER).dvi

obj: $(patsubst %.obj, %.eps, $(wildcard *.obj))
fig: $(patsubst %.fig, %.eps, $(wildcard *.fig))

res:
	make -C results
	make -C figs

clean:
	$(RM) *.aux *.log *.bbl *.blg *~ \#*\# *.toc *.idx
	$(RM) $(patsubst %.tex, $(PAPER).ps, $(wildcard *.tex))
	$(RM) $(patsubst %.tex, %.dvi, $(wildcard *.tex))
	$(RM) $(patsubst %.tex, $(PAPER).pdf, $(wildcard *.tex))

distclean: clean
	$(RM) $(patsubst %.fig, %.eps, $(wildcard *.fig))
	$(RM) $(patsubst %.obj, %.eps, $(wildcard *.obj))

# only need -Ppdf if using CM fonts
#%.pdf: %.dvi $(wildcard *.bib)
#	dvips -Ppdf -G0 -t letter -o $*.tmp.ps $<
#	ps2pdf $*.tmp.ps $*.pdf
#	$(RM) $*.tmp.ps $*.dvi *.aux *.log *.bbl *.blg *.toc

figs:
	neato meta.dot -Tpdf -o meta.pdf

%.pdf: $(wildcard *.tex) $(wildcard *.bib)
	$(LATEX) $*
	bibtex $*
	$(LATEX) $*
	if [ -e $*.toc ] ; then $(LATEX) $* ; fi
	if [ -e $*.bbl ] ; then $(LATEX) $* ; fi
	if egrep Rerun $*.log ; then $(LATEX) $* ; fi
	if egrep Rerun $*.log ; then $(LATEX) $* ; fi
	if egrep Rerun $*.log ; then $(LATEX) $* ; fi

	$(RM) *.aux *.log *.bbl *.blg *.toc

%.ps: %.dvi
	dvips -Ppdf -G0 -tletter -o $@ $<
%.ps.gz: %.ps
	gzip -v9 $<

%.eps: %.obj
	tgif -print -eps $*

%.eps: %.fig
	fig2dev -L eps $< $@
