PAPER=overlog

.SUFFIXES: .pdf .tex

TEXINPUTS := .:${TEXINPUTS}
BIBINPUTS := .:${TEXINPUTS}

all: pdf

pdf: $(PAPER).pdf

clean:
	$(RM) *.aux *.log *.bbl *.blg *~ \#*\# *.toc *.idx
	$(RM) $(patsubst %.tex, %.ps, $(wildcard *.tex))
	$(RM) $(patsubst %.tex, %.dvi, $(wildcard *.tex))
	$(RM) $(patsubst %.tex, %.pdf, $(wildcard *.tex))

%.pdf: $(wildcard *.tex) $(wildcard *.bib)
	pdflatex $*
