# $Id: Makefile,v 1.1 2007/05/05 15:08:34 maniatis Exp $

PAPER=madEnsembles
BIBFILE=$(PAPER).bib
TEXFILE=$(PAPER).tex
PDFFILE=$(PAPER).pdf
PSFILE=$(PAPER).ps
DVIFILE=$(PAPER).dvi

GRAPHS=$(wildcard script/*.eps)
FIGS=$(wildcard figs/*.fig)
EPSFIGS=$(FIGS:.fig=.eps) 
VISIOEPSES=$(wildcard visio/*.eps)


all: $(PAPER).pdf

dvi: $(PAPER).dvi

subdirs:
	$(MAKE) -C visio


bib:
	$(BIBTEX) $(PAPER)

pdf: $(PAPER).pdf

$(DVIFILE): $(TEXFILE) $(EPSFIGS) $(GRAPHS) $(BIBFILE) subdirs
	latex $(PAPER)
	bibtex $(PAPER)
	latex $(PAPER)
	latex $(PAPER)

%.pdf: %.ps
	ps2pdf -dCompatibilityLevel=1.4 $< $@
 
%.ps: %.dvi
	dvips -t letter -Ppdf -G0 -o $@ $<

clean:
	/bin/rm -f *.dvi *.aux $(PAPER).pdf *~ *.log *.lot *.lof *.toc *.blg *.bbl
	$(MAKE) -C visio clean



