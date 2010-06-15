###########################################################
#
# Makefile for LaTeX docs
#
# $Id: 
#

.SUFFIXES:
.SUFFIXES: .ps .dvi .tex

TEXINPUTS := .:${TEXINPUTS}
BIBINPUTS := .:${TEXINPUTS}

all: eps dvi ps pdf # res
eps: fig obj

PAPER=libasync

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
	$(RM) $(patsubst %.tex, %.ps, $(wildcard *.tex))
	$(RM) $(patsubst %.tex, %.dvi, $(wildcard *.tex))
	$(RM) $(patsubst %.tex, %.pdf, $(wildcard *.tex))

distclean: clean
	$(RM) $(patsubst %.fig, %.eps, $(wildcard *.fig))
	$(RM) $(patsubst %.obj, %.eps, $(wildcard *.obj))

tarball: $(PAPER).tgz

$(PAPER).tgz: pdf
	tar czvf $@ ./$(PAPER).tex ./$(PAPER).bib ./Makefile ./$(PAPER).pdf


# only need -Ppdf if using CM fonts
%.pdf: %.dvi $(wildcard *.bib)
	dvips -Ppdf -G0 -t letter -o $*.tmp.ps $<
	ps2pdf $*.tmp.ps $*.pdf
	$(RM) $*.tmp.ps $*.dvi *.aux *.log *.bbl *.blg *.toc

%.dvi: $(wildcard *.tex) $(wildcard *.bib)
	$(RM) $*.ind $*.idx
	latex $*
	bibtex $*
	latex $*
	makeindex $*
	if [ -e $*.toc ] ; then latex $* ; fi
	if [ -e $*.bbl ] ; then latex $* ; fi
	if egrep Rerun $*.log ; then latex $* ; fi
	if egrep Rerun $*.log ; then latex $* ; fi
	if egrep Rerun $*.log ; then latex $* ; fi
	$(RM) *.aux *.log *.bbl *.blg *.toc 

%.ps: %.dvi
	dvips -Ppdf -G0 -tletter -o $@ $<
%.ps.gz: %.ps
	gzip -v9 $<

%.eps: %.obj
	tgif -print -eps $*

%.eps: %.fig
	fig2dev -L eps $< $@
