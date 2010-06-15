# $Id: Makefile,v 1.1 2007/08/20 22:59:49 maniatis Exp $

PAPER=main
BIBFILE=$(PAPER).bib
TEXFILE=$(PAPER).tex
PDFFILE=$(PAPER).pdf
PSFILE=$(PAPER).ps
DVIFILE=$(PAPER).dvi

GRAPHS=$(wildcard script/*.eps)
FIGS=$(wildcard figs/*.fig)
EPSFIGS=$(FIGS:.fig=.eps) 

all: $(PAPER).pdf

dvi: $(PAPER).dvi

subdirs:


#	$(MAKE) -C script

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
	/bin/rm -f *.dvi *.aux $(PAPER).ps $(PAPER).pdf *~ *.log *.lot *.lof *.toc *.blg *.bbl

#	$(MAKE) -C figs clean
#	$(MAKE) -C script clean
