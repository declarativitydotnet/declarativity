NAME=vldb
USE_PDFLATEX=1
TEXSRCS=vldb.tex intro.tex causal.tex lang.tex relwork.tex carts.tex conclusion.tex foundation.tex acks.tex background.tex discussion.tex
BIBTEXSRCS=vldb.bib

OTHER_PDF=$(wildcard fig/*.pdf)
OTHER+=$(OTHER_PDF)

# Regenerate perf graph if needed
fig/sn_perf.pdf: bench/sn_perf.gp
	gnuplot -e "set output \"$@\"" $<

ifeq ($(shell uname), Linux)
  VIEWPDF=evince
  include /usr/share/latex-mk/latex.gmk
else
  # OSX
  VIEWPDF=open
  include /opt/local/share/latex-mk/latex.gmk
endif
