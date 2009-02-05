NAME=sosp
USE_PDFLATEX=1
BIBTEXSRCS=sosp.bib

ifeq ($(shell uname), Linux)
  # linux
  VIEWPDF=evince
  include  /usr/share/latex-mk/latex.gmk
else
  # macos
  VIEWPDF=open
  include /opt/local/share/latex-mk/latex.gmk
endif
