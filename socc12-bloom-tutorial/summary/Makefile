NAME=bloom-tutorial-onscreen bloom-tutorial-print
USE_PDFLATEX=1

# Common LaTeX source files
TEXSRCS=summary.tex
BIBTEXSRCS=summary.bib

bloom-tutorial-onscreen_TEXSRCS=bloom-tutorial-onscreen.tex
bloom-tutorial-print_TEXSRCS=bloom-tutorial-print.tex

OTHER_PDF=$(wildcard fig/*.pdf)
OTHER+=$(OTHER_PDF)

ifeq ($(shell uname), Linux)
  VIEWPDF=evince
  include /usr/share/latex-mk/latex.gmk
else
  # OSX
  VIEWPDF=open
  include /opt/local/share/latex-mk/latex.gmk
endif
