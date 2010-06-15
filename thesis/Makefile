NAME=thesis
USE_PDFLATEX=1
TEXSRCS=thesis.tex background.tex boom.tex conc.tex dataflow.tex optimization.tex hop.tex overview.tex title.tex
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
