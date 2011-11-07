NAME=vldb
USE_PDFLATEX=1
TEXSRCS=vldb.tex intro.tex causal.tex impl.tex relwork.texx
BIBTEXSRCS=vldb.bib

ifeq ($(shell uname), Linux)
  VIEWPDF=evince
  include /usr/share/latex-mk/latex.gmk
else
  # OSX
  VIEWPDF=open
  include /opt/local/share/latex-mk/latex.gmk
endif
