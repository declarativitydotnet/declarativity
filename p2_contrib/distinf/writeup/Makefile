
PAPER=distinf
BIBFILE=distinf.bib
TEXFILE=$(PAPER).tex body.tex
PDFFILE=$(PAPER).pdf
PSFILE=$(PAPER).ps
DVIFILE=$(PAPER).dvi

GRAPHS=$(wildcard script/*.eps)
FIGS=$(wildcard figs/*.fig)
EPSFIGS=$(FIGS:.fig=.eps) 

all: $(PAPER).pdf

dvi: $(PAPER).dvi

bib:
	$(BIBTEX) $(PAPER)

pdf: $(PAPER).pdf

$(DVIFILE): $(TEXFILE) $(EPSFIGS) $(GRAPHS) $(BIBFILE) 
	latex $(PAPER)
	bibtex $(PAPER)
	latex $(PAPER)
	latex $(PAPER)

%.pdf: %.ps
	ps2pdf -dCompatibilityLevel=1.4 $< $@

%.ps: %.dvi
	dvips -t letter -Ppdf -G0 -o $@ $<


%.eps: %.svg
	inkscape -E "`cygpath -a -w $@`" "`cygpath -a -w $<`"

clean:
	/bin/rm -f *.dvi *.aux $(PAPER).ps $(PAPER).pdf *~ *.log *.lot *.lof *.toc *.blg *.bbl
