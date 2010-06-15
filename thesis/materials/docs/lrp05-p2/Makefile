###########################################################
#
# Makefile for LaTeX docs
#
# $Id: 
#

.SUFFIXES:
.SUFFIXES: .ps .dvi .tex

TEXINPUTS := .:/homes/rmm1002/docs/lib/:
BIBINPUTS := .:/homes/rmm1002/docs/bibs/:/usr/groups/netos/bib/:

TEX = $(wildcard *.tex)
all: eps dvi ps 
eps: fig obj

ps:  paper.ps  eps
pdf: paper.pdf eps
dvi: paper.dvi eps

obj: $(patsubst %.obj, %.eps, $(wildcard *.obj))
fig: $(patsubst %.fig, %.eps, $(wildcard *.fig))

clean:
	$(RM) *.aux *.log *.bbl *.blg *~ \#*\# *.toc *.idx *.eps 
	$(RM) $(patsubst %.tex, %.ps, $(wildcard *.tex))
	$(RM) $(patsubst %.tex, %.dvi, $(wildcard *.tex))
	$(RM) $(patsubst %.tex, %.pdf, $(wildcard *.tex))

distclean: clean
	$(RM) $(patsubst %.fig, %.eps, $(wildcard *.fig))
	$(RM) $(patsubst %.obj, %.eps, $(wildcard *.obj))

# only need -Ppdf if using CM fonts

%.pdf: %.dvi $(wildcard *.bib)
	dvips -tletter -Ppdf -G0 -o $*.tmp.ps $<
	ps2pdf $*.tmp.ps $*.pdf
	$(RM) $*.tmp.ps $*.dvi *.aux *.log *.bbl *.blg *.toc

%.dvi: $(TEX) $(wildcard *.bib)
	latex $*
	-bibtex $*
	if [ -e $*.toc ] ; then latex $* ; fi
	if [ -e $*.bbl ] ; then latex $* ; fi
	if egrep Rerun $*.log ; then latex $* ; fi
	if egrep Rerun $*.log ; then latex $* ; fi
	if egrep Rerun $*.log ; then latex $* ; fi
	$(RM) *.aux *.log *.bbl *.blg *.toc

%.ps: %.dvi
	dvips -tletter -Ppdf -G0 -o $@ $<

%.ps.gz: %.ps
	gzip -v9 $<

%.eps: %.obj
	tgif -color -print -eps $*

%.eps: %.fig
	fig2dev -L eps $< $@

toc: $(wildcard *.tex)
	@egrep -h "\\\((sub)*section|chapter)" $+ | egrep -v "^%" | \
                sed -e 's/\\chapter//;                          \
                s/\\section/    /;                              \
                s/\\subsection/         /;                      \
                s/\\subsubsection/                      /;      \
                s/\\.*{//g;s/[{}]//g' > $@
