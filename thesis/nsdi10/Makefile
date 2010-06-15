NAME=nsdi
USE_PDFLATEX=1
TEXSRCS=nsdi.tex intro.tex background.tex pipelining.tex onlineaggs.tex continuous.tex relwork.tex conclusion.tex perf.tex
BIBTEXSRCS=nsdi.bib

ifeq ($(shell uname), Linux)
  # Linux
  VIEWPDF=evince
  include /usr/share/latex-mk/latex.gmk
else
  # OSX
  VIEWPDF=open
  include /opt/local/share/latex-mk/latex.gmk
endif
