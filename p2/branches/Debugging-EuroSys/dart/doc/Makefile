manual:
	jade -V nochunks -t sgml -i html -d /homes/bnc/docbook/ldp.dsl\#html DART-HOWTO.sgml > singlepage.html
	jade -t sgml -i html -d /homes/bnc/docbook/ldp.dsl\#html DART-HOWTO.sgml
	docbook2pdf -d /homes/bnc/docbook/ldp.dsl DART-HOWTO.sgml
	docbook2ps -d /homes/bnc/docbook/ldp.dsl DART-HOWTO.sgml

clean:
	rm -f *~ *.aux *.dvi *.log *.pdf *.ps *.html DART-HOWTO.tex
