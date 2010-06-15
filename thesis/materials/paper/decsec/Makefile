# $Id: Makefile,v 1.1 2007/08/10 18:56:55 pmahajan Exp $

PAPER=decsec
BIBFILE=$(PAPER).bib
TEXFILE=$(PAPER).tex
PDFFILE=$(PAPER).pdf
PSFILE=$(PAPER).ps
DVIFILE=$(PAPER).dvi

VISIOEPSES=$(wildcard visio/*.eps)
GPFIGS=$(wildcard graphs/*.gp)
GPEPSES=$(GPFIGS:.gp=.eps)


all: $(PAPER).pdf

dvi: $(PAPER).dvi

pdf: $(PAPER).pdf

ps: $(PAPER).ps


subdirs:
	$(MAKE) -C visio
	$(MAKE) -C graphs


bib:
	$(BIBTEX) $(PAPER)


$(DVIFILE): $(TEXFILE) $(BIBFILE) subdirs
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
	$(MAKE) -C visio clean
	$(MAKE) -C graphs clean

