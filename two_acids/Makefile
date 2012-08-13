NAME=twoacids-cidr13
USE_PDFLATEX=1
TEXSRCS=twoacids-cidr13.tex
BIBTEXSRCS=cidr.bib

OTHER_PDF=$(wildcard fig/*.pdf)
OTHER_CODE=$(wildcard code/*.ded)
OTHER+=$(OTHER_PDF) $(OTHER_CODE)

ifeq ($(shell uname), Linux)
  VIEWPDF=evince
  include /usr/share/latex-mk/latex.gmk
else
  # OSX
  VIEWPDF=open
  include /opt/local/share/latex-mk/latex.gmk
endif
