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

## No need for separate target, fixes redundant executions (PM)
ADD_CUSTOM_COMMAND(
  OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/UserGuide.pdf
  DEPENDS   ${CMAKE_CURRENT_SOURCE_DIR}/UserGuide.tex
  	    ${CMAKE_CURRENT_BINARY_DIR}/Oni.jpg
            ${CMAKE_CURRENT_BINARY_DIR}/UserGuide.bib
  COMMAND   ${PDFLATEX_COMPILER}
  ARGS      -interaction=batchmode ${CMAKE_CURRENT_SOURCE_DIR}/UserGuide.tex
  COMMAND   ${BIBTEX_COMPILER}
  ARGS      -terse ${CMAKE_CURRENT_BINARY_DIR}/UserGuide
  COMMAND   ${PDFLATEX_COMPILER}
  ARGS      -interaction=batchmode ${CMAKE_CURRENT_SOURCE_DIR}/UserGuide.tex
  COMMAND   ${PDFLATEX_COMPILER}
  ARGS      -interaction=batchmode ${CMAKE_CURRENT_SOURCE_DIR}/UserGuide.tex
  COMMENT   "LaTeX - Bibtex - LaTeX - LaTeX again"
  )

SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
"${CMAKE_CURRENT_BINARY_DIR}/UserGuide.bbl;${CMAKE_CURRENT_BINARY_DIR}/UserGuide.aux;${CMAKE_CURRENT_BINARY_DIR}/UserGuide.dvi;${CMAKE_CURRENT_BINARY_DIR}/UserGuide.log;${CMAKE_CURRENT_BINARY_DIR}/UserGuide.blg")


# Eventually trigger the whole process
ADD_CUSTOM_TARGET(UserGuide ALL echo
   DEPENDS   ${CMAKE_CURRENT_BINARY_DIR}/UserGuide.pdf
   )

### End stolen and munged stuff.

endif(PDFLATEX_COMPILER)
