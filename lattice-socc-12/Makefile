NAME=socc17-onscreen socc17-print
USE_PDFLATEX=1

# Common LaTeX source files
TEXSRCS=intro.tex lang.tex relwork.tex cart-case.tex kvs-case.tex conclusion.tex acks.tex background.tex discussion.tex impl.tex socc.tex
BIBTEXSRCS=socc.bib

socc-onscreen_TEXSRCS=socc17-onscreen.tex
socc-print_TEXSRCS=socc17-print.tex

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

# Regenerate perf graph if needed
EXP_DATA=$(wildcard bench/*.data)
EXP_SUMMARY=$(wildcard bench/*.summary)
fig/sn_perf.pdf: bench/sn_perf.gp $(EXP_DATA) $(EXP_SUMMARY)
	gnuplot -e "set output \"$@\"" $<

# Generate tarball containing public code listings
PUBLIC_SRC=$(wildcard code/public-tarball/**/*)
blooml-case-studies.tar.gz: $(PUBLIC_SRC)
	tar czf $@ -C code/public-tarball blooml-case-studies

public-tgz: blooml-case-studies.tar.gz
