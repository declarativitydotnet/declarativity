# This file is distributed under the terms in the attached LICENSE file.
# If you do not find this file, copies can be found by writing to:
# Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
# Or
# UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
# Berkeley, CA,  94707. Attention: P2 Group.
#

SUBDIRS=	\
		olg \
		c++

CLEANFILES =			*.cpp \
				*.eca \
				*.ruleStrand \
				*.dot \
				*~ \
				.\#* \
				*.localized \
				*.aux \
				*.bbl \
				*.blg \
				*.log \
				*.pdf \
				*.dvi

EXTRA_DIST =			*.olg

all : UserGuide.pdf

UserGuide.pdf :	UserGuide.tex UserGuide.bib
	pdflatex UserGuide
	bibtex UserGuide
	pdflatex UserGuide
	pdflatex UserGuide
