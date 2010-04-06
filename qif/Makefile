NAME=qif
USE_PDFLATEX=1
TEXSRCS=qif.tex agenda.tex vision.tex intro.tex team.tex
BIBTEXSRCS=qif.bib
OTHER=

ifeq ($(shell uname), Linux)
  VIEWPDF=evince
  include /usr/share/latex-mk/latex.gmk
else
  # OSX
  VIEWPDF=open
  include /opt/local/share/latex-mk/latex.gmk
endif
