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
PRNS =			$(wildcard Figures/*.prn)
VISIOEPSES :=		$(addsuffix .eps, $(basename $(PRNS)))
SHRINK := Figures/shrink.sh

all: $(VISIOEPSES) eps dvi ps pdf # res
eps: fig obj

PAPER=bft-scale

ps:  $(PAPER).ps
pdf: $(PAPER).pdf
dvi: $(PAPER).dvi

obj: $(patsubst %.obj, %.eps, $(wildcard *.obj))
fig: $(patsubst %.fig, %.eps, $(wildcard *.fig))

res:
	make -C results
	make -C figs

clean:
	$(RM) *.aux *.log *.bbl *.blg *~ \#*\# *.toc *.idx *.tmp.ps
	$(RM) $(patsubst %.tex, %.ps, $(wildcard *.tex))
	$(RM) $(patsubst %.tex, %.dvi, $(wildcard *.tex))
	$(RM) $(patsubst %.tex, %.pdf, $(wildcard *.tex))
	$(RM) $(VISIOEPSES)

distclean: clean
	$(RM) $(patsubst %.fig, %.eps, $(wildcard *.fig))
	$(RM) $(patsubst %.obj, %.eps, $(wildcard *.obj))

tarball: $(PAPER).tgz

$(PAPER).tgz: pdf
	tar czvf $@ ./$(PAPER).tex ./$(PAPER).bib ./Makefile ./$(PAPER).pdf


# only need -Ppdf if using CM fonts
%.pdf: %.dvi $(wildcard *.bib) $(VISIOEPSES) 
	dvips -Ppdf -G0 -t letter -o $*.tmp.ps $<
	ps2pdf -dCompatibilityLevel=1.3 -dMaxSubsetPct=100 -dSubsetFonts=true -dEmbedAllFonts=true $*.tmp.ps $*.pdf


%.dvi: $(wildcard *.tex) $(wildcard *.bib) $(VISIOEPSES) 
	latex $*
	bibtex $*
	latex $*
	if [ -e $*.toc ] ; then latex $* ; fi
	if [ -e $*.bbl ] ; then latex $* ; fi
	if egrep Rerun $*.log ; then latex $* ; fi
	if egrep Rerun $*.log ; then latex $* ; fi
	if egrep Rerun $*.log ; then latex $* ; fi


%.ps: %.dvi $(VISIOEPSES) 
	dvips -Ppdf -G0 -tletter -o $@ $<

%.ps.gz: %.ps
	gzip -v9 $<

%.eps: %.obj
	tgif -print -eps $*

%.eps: %.fig
	fig2dev -L eps $< $@

%.eps:		%.prn Figures/Figures
	${SHRINK} $< Figures/Figures


