# This is a simple, general makefile for LaTeX documents.

# Put the base name of the main file here
# WARNING: 'make clean' will remove $(NAME).ps and $(NAME).pdf
# Also switch commenting on the 'clean' lines if this is a beamer project
NAME = paper

# These options are suggested by the NSF Fastlane PDF-creation page
#PDFOPTIONS = -dMaxSubsetPct=100 -dCompatibilityLevel=1.2 -dSubsetFonts=true -dEmbedAllFonts=true
PDFOPTIONS = -dSubsetFonts=true -dEmbedAllFonts=true

.SECONDARY:

default: pdf

html: $(NAME).html

#dvi: $(NAME).dvi
#ps: $(NAME).ps
pdf: $(NAME).pdf

all: html pdf

%.pdf: %.tex *.tex paper.bib
	pdflatex $<
	bibtex $*
	pdflatex $<
	pdflatex $<
	pdflatex $<

%.html: %.tex *.tex paper.bib
	latex2html -split 0 -show_section_numbers -local_icons -no_navigation $*.tex

# The second one also removes files created by beamer
clean:
	rm -f *.aux *.log *.dvi *.bbl *.blg *~ $(NAME).ps $(NAME).pdf
#	rm -f *.aux *.log *.dvi *.bbl *.blg *.nav *.snm *.toc *~ $(NAME).out $(NAME).ps $(NAME).pdf
