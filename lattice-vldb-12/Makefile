NAME=vldb
USE_PDFLATEX=1
TEXSRCS=vldb.tex intro.tex causal.tex impl.tex relwork.tex carts.tex conclusion.tex foundation.tex acks.tex
BIBTEXSRCS=vldb.bib

ifeq ($(shell uname), Linux)
  VIEWPDF=evince
  include /usr/share/latex-mk/latex.gmk
else
  # OSX
  VIEWPDF=open
  include /opt/local/share/latex-mk/latex.gmk
endif
