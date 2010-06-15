TEXS=  \
problem-statement.tex\
main.tex 



BIBS = p2p.bib

default: main.ps 

preview: main.dvi
	xdvi main.dvi

prevps: main.ps
	ghostview main.ps

ispell:
	ispell -b -t $(TEXS)

ps: main.ps

.SUFFIXES: .ps .pdf .dvi .eps .prn

pdf: main.pdf

.ps.pdf:
	ps2pdf -dMaxSubsetPct=100 -dCompatibilityLevel=1.2 -dSubsetFonts=true -dEmbedAllFonts=true $< $@

.prn.eps:
	ps2pdf $< tmp.pdf
	acroread -toPostScript tmp.pdf
	ps2eps tmp.ps
	mv tmp.eps $@
	rm -f tmp.pdf tmp.ps

.dvi.ps:
	dvips -Pcmz -t letter -f < $< > $@

main.dvi: bblfix.pl $(EPS_FILES) $(BIBS) $(TEXS)
	latex main.tex
	bibtex main
	mv main.bbl main.bblx
	perl bblfix.pl < main.bblx > main.bbl
	latex main.tex
	latex main.tex

clean:
	rm -f *~ *.aux *.log *.dvi *.bbl *.blg      

