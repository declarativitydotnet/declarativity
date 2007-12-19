### Stolen and munged from http://www.vtk.org/Wiki/CMake_FAQ#How_do_I_use_CMake_to_build_LaTeX_documents.3F

## This could be more efficient, but it seems to be correct.  You have
## to edit the makefile every time you add a new figure to the .tex,
## and it rebuilds the document every time you compile.

# I'd rather just do this:
# SET( ENV{BIBINPUTS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )
# SET( ENV{TEXINPUTS} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )
# but that sets the environment variables while cmake
# (not make) is running.

if(PDFLATEX_COMPILER)

## need to explicitly copy each file that the .tex includes.
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/UserGuide.bib
        ${CMAKE_CURRENT_BINARY_DIR}/UserGuide.bib
        COPYONLY)
## need to explicitly copy each file that the .tex includes.
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Oni.jpg
        ${CMAKE_CURRENT_BINARY_DIR}/Oni.jpg
        COPYONLY)

ADD_CUSTOM_COMMAND(
  OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/UserGuide.aux
  DEPENDS   ${CMAKE_CURRENT_SOURCE_DIR}/UserGuide.tex
  	    ${CMAKE_CURRENT_BINARY_DIR}/Oni.jpg
  COMMAND   ${PDFLATEX_COMPILER}
  ARGS      -interaction=batchmode ${CMAKE_CURRENT_SOURCE_DIR}/UserGuide.tex
  COMMENT   "Latex (first pass)"
  )

ADD_CUSTOM_COMMAND(
  OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/UserGuide.bbl
  DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/UserGuide.aux
  COMMAND   ${BIBTEX_COMPILER}
  ARGS      -terse ${CMAKE_CURRENT_BINARY_DIR}/UserGuide
  COMMENT   "Bibtex"
  )

ADD_CUSTOM_COMMAND(
  OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/UserGuide.pdf
  DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/UserGuide.bbl
  COMMAND   ${PDFLATEX_COMPILER}
  ARGS      -interaction=batchmode ${CMAKE_CURRENT_SOURCE_DIR}/UserGuide.tex
  COMMENT   "Latex (second pass)"
  )

ADD_CUSTOM_COMMAND(
  OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/UserGuide.log
  DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/UserGuide.bbl
            ${CMAKE_CURRENT_BINARY_DIR}/UserGuide.pdf
  COMMAND   ${PDFLATEX_COMPILER}
  ARGS      -interaction=batchmode ${CMAKE_CURRENT_SOURCE_DIR}/UserGuide.tex
  COMMENT   "Latex (third pass)"
  )
# Eventually trigger the whole process
ADD_CUSTOM_TARGET(UserGuide ALL echo
   DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/UserGuide.log
   )

### End stolen and munged stuff.

endif(PDFLATEX_COMPILER)
