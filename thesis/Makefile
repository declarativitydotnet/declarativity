NAME=thesis
USE_PDFLATEX=1
TEXSRCS=ack.tex thesis.tex abstract.tex p2.tex evita.tex magic.tex opt.tex evitaend.tex cloud.tex hadoop.tex boom.tex hop.tex conc.tex overview.tex title.tex
BIBTEXSRCS=thesis.bib

ifeq ($(shell uname), Linux)
  # Linux
  VIEWPDF=evince
  include /usr/share/latex-mk/latex.gmk
else
  # OSX
  VIEWPDF=open
  include /opt/local/share/latex-mk/latex.gmk
endif
