NAME=euro041-alvaro
USE_PDFLATEX=1
TEXSRCS=intro.tex relwork.tex bg.tex hdfs.tex rely.tex scale.tex manage.tex lessons.tex conclusion.tex euro041-alvaro.tex mapreduce.tex eval.tex acknowledgments.tex
BIBTEXSRCS=euro041-alvaro.bib

ifeq ($(shell uname), Linux)
  # Linux
  VIEWPDF=evince
  include /usr/share/latex-mk/latex.gmk
else
  # OSX
  VIEWPDF=open
  include /opt/local/share/latex-mk/latex.gmk
endif
