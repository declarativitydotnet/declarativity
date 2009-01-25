NAME=sosp
VIEWPDF=open
USE_PDFLATEX=1
BIBTEXSRCS=sosp.bib

ifeq ($(shell uname), Linux)
  # linux; let distro manage latex-make
  include LaTeX.mk
else
  # macos; hardcode non-fhs path
  include /opt/local/share/latex-mk/latex.gmk
endif
