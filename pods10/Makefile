NAME=pods
USE_PDFLATEX=1
TEXSRCS=pods.tex
BIBTEXSRCS=pods.bib

ifeq ($(shell uname), Linux)
  VIEWPDF=evince
  include /usr/share/latex-mk/latex.gmk
else
  # OSX
  VIEWPDF=open
  include /opt/local/share/latex-mk/latex.gmk
endif
