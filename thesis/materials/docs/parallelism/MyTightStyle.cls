%%
%% IEEEtran.cls 7/2001 version V1.5
%% (file is named IEEEtran_v15.cls for distribution purposes)
%% Provides formatting for authors of the Institute of Electrical
%% and Electronics Engineers (IEEE) Transactions Journals.
%% See:
%% http://www.ctan.org/tex-archive/macros/latex/contrib/supported/IEEEtran/
%% for the latest version.
%%
%% Based on the original 1993 IEEEtran.cls, but with bug fixes
%% and enhancements (from both JVH and MDS) over the 1996 version -
%% which is the most recent on the IEEE site ( www.ieee.org ) as of 7/2001.
%% (IEEE also has a 1997 version, IEEEtran2e.cls, (patched by Peter N"uchter
%% and Andrew Back), which is used by INFOCOM and is nearly identical
%% to the 1996 version.) 
%%
%% THIS IS AN ADVANCED VERSION OF IEEEtrans.cls THAT MAY NOT BE
%% OFFICIALLY ENDORSED BY IEEE! However, it has been used by many
%% people without problems and is no longer considered beta test.
%%
%% IT IS IMPORTANT THAT YOU READ THE DOCS BELOW EVEN IF YOU ARE
%% FAMILIAR WITH OTHER VERSIONS OF IEEEtran.cls! This version's
%% itemize, enumerate and description list environments, and
%% \appendix(ces) have new controls and behavior.
%%
%%
%% Contributors: 
%% Gerry Murray (1993), Silvano Balemi (1993),
%% Jon Dixion (1996), Peter N"uchter (1996),
%% Juergen von Hagen (2000), and Michael Shell (2001)
%% 
%% 
%% Copyright 1993-2001 by Gerry Murray, Silvano Balemi, 
%%                        Jon Dixion, Peter N"uchter,
%%                        Juergen von Hagen and Michael Shell
%%
%% Special thanks to Peter Wilson (CUA) and Donald Arseneau
%% for allowing the inclusion of the \@ifmtarg command 
%% from their ifmtarg LaTeX package. 
%% 
%%**********************************************************************
%% Legal Notice:
%% This code is offered as-is without any warranty either
%% expressed or implied; without even the implied warranty of
%% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE!
%% User assumes all risk.
%% In no event shall IEEE or any contributor to this code
%% be liable for any damages or losses, including, but not limited to,
%% incidental, consequential, or any other damages, resulting from the
%% use or misuse of any information contained here.
%% 
%% All statements made here are the opinions of their respective
%% authors and are not necessarily endorsed by the IEEE.
%%
%% This code is distributed under the Perl Artistic License 
%% ( http://language.perl.com/misc/Artistic.html ) 
%% and may be freely used, distributed and modified.
%% Please retain the contribution notices and credits.
%% 
%% Major changes to the user interface should be indicated by an 
%% increase in the version numbers. If a version is a beta, it will 
%% be indicated with a BETA suffix, i.e., 1.4BETA.
%% Small changes can be indicated by appending letters to the version
%% such as "IEEEtran_v14a.cls".
%% In ALL cases, the distribution filename, \Providesclass, any \typeout
%% messages to the user, \IEEEtransversionmajor and \IEEEtransversionminor
%% must reflect the correct version information.
%% The changes should also be documented via source comments.
%%**********************************************************************
%%
%
%
%*******
% 7/2001 V1.5 (MDS) changes:
% 
% 1) Fixed \and within \author bug: (! Misplaced \crcr. \endtabular ->\crcr)
%    NOTICE: A side effect of this change is that you will have to repeat
%    fontsize commands for each line in the author information as font size
%    changes will be confined to that line (or at least it does on my TeTeX 
%    system). Do not attempt enclose multiple lines within a pair of braces
%    as an error may occur (or at least it does on my system).
%    This is because the author data is now printed from within the tabular
%    environment exactly as is done in article.cls (which also exhibits the
%    same limitations). The \and feature is most useful when IEEEtran.cls is
%    used for conferences. For example:
%    
%    \author{\large First Author\\
%    \normalsize First School\\
%    \normalsize City1\\
%    \normalsize Email: first@nodomain.org
%    \and
%    \large Second Author\\
%    \normalsize Second School\\
%    \normalsize City2\\
%    \normalsize Email: second@nulldomain.org}
%     
%    Thanks to Rainer Dorsch for discovering and reporting that \and 
%    did not work.
%    
% 2) Fixed the biography environment so that if a biography's text is shorter
%    than the area allocated for the photo, a collision with the next
%    biography does not occur. You can now put real graphics (using the
%    graphicx package) into the biography photo box with a new optional 
%    argument of the biography command! For example:
%    
%    \begin{biography}[{\includegraphics[width=1in,height=1.25in,clip,keepaspectratio]{./tux.eps}}]{Linux Penguin}
%    
%    will use the specified graphic as the author's photo. The photo area is
%    exactly 1in wide by 1.25in high - as is done in IEEE Transactions. Try
%    to keep the same 4:5 aspect ratio when scanning/cropping your photos. 
%    Note the need for the extra set of enclosing braces around the
%    \includegraphics. Without it, The LaTeX parser gets confused when it
%    sees the \includegraphics's brackets within the biography's optional
%    argument. If you do not use the optional argument, or leave it empty, 
%    a standard frame box with the words "Place Photo Here" will be used. 
%    If you want the space to remain completely empty, you can do:
%    
%    \begin{biography}[\mbox{}]{The Invisible Man} 
%    
%    The interface to biography's optional argument is into a
%    1in X 1.25in minipage in which the argument text is centered both 
%    horizontally and vertically:
%    
%    \begin{minipage}[b][1.25in][c]{1in}%
%    \centering
%    #1%
%    \end{minipage}
%    
%    Within the biography environment, \unitlength is set to 1in.
%    With this in mind, you can even design your own custom frameboxes.
%    For instance:
%    
%    \begin{biography}[\framebox(1,1.25){\parbox[][\height][c]{0.9in}{\centering PLACE\\ PHOTO\\ HERE}}]{Author Name}
%    
%    will yield the same result as the default photo box.
%    
%    Thanks to Herbert Voss for discovering the collision bug, suggesting the ability
%    to handle graphics and providing some prototype code.
%    
% 3/2001 V1.4 (MDS) changes:
% 
% 1) New "draftcls" and "final" options have been added. Draft mode
%    is very popular. Authors requested that there should be a way
%    for a draft mode paper to display the figures so that the
%    reviewer can see them. Traditionally, this has been accomplished
%    by invoking all packages with the "final" option. Hence:
% 
%    \documentclass[draft]{IEEEtran}
%    .
%    .
%    \usepackage[final]{graphicx}
% 
%    Now, you can just use:
% 
%    \documentclass[draftcls]{IEEEtran}
%    .
%    .
%    \usepackage{graphicx}
% 
%    and only IEEEtran.cls will format as draft - the other packages
%    will still act as though "final" had been selected.
%    Thanks to Dragan Cvetkovic for suggesting this.
% 
%    Also, \documentclass now accepts "final" as an option. This
%    really isn't needed as it defaults to "final" unless told
%    otherwise. However, article.cls does accept "final" so IEEEtran.cls
%    should do so as well.
% 
% 2) Documentation changes to reflect the fact that this IEEEtran.cls 
%    is no longer beta test. Added usage notes #11, 12 and 13.
%
% 3) Slightly revised caption sizes. Figure and table captions are now 
%    in \footnotesize, not \small as before.
%
% 4) Allow user to control figure caption justification. IEEEtran.cls 
%    normally defaults to left justified as is done in Transactions.
%    However, for conferences, you may wish to issue the command:
%    \centerfigcaptionstrue
%    in the preamble. Short (less than one line long) figure captions
%    will then be centered. Multi-line figure captions will always be 
%    properly left justified.
%    
%    
%*******
% 1/2001 V1.3 (MDS) made extensive changes and additions:
% 
% It is true that IEEEtran.cls is not really a formatting tool since
% IEEE does their own custom typesetting. However, there is a very
% real need for authors to be able to see their work in a form that
% closely resembles what IEEE will use. Content isn't supposed to
% follow format. However, in practice, the way a document lays out
% can influence the choice of words or the structure of equations.
% If the class file makes it difficult to get results close to
% that of IEEE, authors will pay even MORE attention to the details
% of document layout! Furthermore, when submitting work in PDF or PS
% form (to editors, conferences, colleagues, etc.) appearances do matter.
% The purpose of this version is to provide easier and better formatting
% so that authors have a few less headaches to deal with. See the 
% companion file, IEEEtest.tex, (available on CTAN) for a demonstration
% of usage and of some of the issues addressed.
%
% I sincerely wish you all the best in your publication endeavors! ;)
%
% Enjoy!
%
% Michael Shell (MDS)
% mshell@ece.gatech.edu
%
%
%
% BUGS FIXED (and many others too numerous to mention!):
% 1) Fixed improper alignment with itemized, enumerated and
%    description lists. Added new controls to these three
%    environments so that it is easy to get the alignment IEEE
%    uses. Furthermore, the itemize, enumerate and description lists
%    no longer force a new paragraph to begin at the end the list
%    (\par). (Sometimes lists are used within paragraphs.) 
%    See detailed list docs below.
% 
% 2) JVH's fixes now allow things like $\mathbf{N}(0,P(0))$
%    to work properly without needing the extra braces:
%    ${\mathbf{N}}(0,P(0))$. There is no longer any dependence
%    on the "rawfonts" and "oldlfont" packages. Thanks Juergen! 
% 
% 3) Fixed underfull hbox errors and incorrect reference number
%    alignment when the number of references in the bibliography
%    exceeded 9 entries (which is almost every paper!).
%  
% 4) Removed dependence on the LaTeX sizexx.clo files.
%    Now, 9pt documents should work correctly even on systems that
%    lack a size9.clo file. This is most often used in conjunction
%    with the option "technote" for "correspondence" papers like those
%    in IEEE Transactions on Information Theory. For virtually all
%    other papers, 10pt is used and so it is the default.
%    Some improper font sizes have been corrected (one of which is that
%    IEEE uses 9pt, not 8pt, for \small (Abstracts and Index Terms)
%    in 10pt docs). \small and \footnotesize are now both 8pt in 9pt docs, 
%    so footnotes in technotes should be the correct size now.
% 
% 5) Added \interlinepenalty within the bibliography section to discourage
%    LaTeX from breaking within a reference. IEEE almost never breaks within
%    a reference and when they do it is usually in technotes
%    (correspondence papers). You may get an underfull vbox warning in the
%    bibliography indicating that the spacing just before the "REFERENCES" 
%    section is larger than normal, but the final result will be more like 
%    what IEEE will publish. See the comments in the BIBLIOGRAPHY section
%    around line 2034 below if you want to change this behavior.
%
% 6) No longer "blows up" when you use \paragraph and have a table
%    of contents.
%
% 
% USAGE NOTES:
% 1) The new IEEEtran.cls will change the length, layout and possibly fonts
%    of your documents slightly as compared to that under the older 
%    IEEEtrans.cls. However, the final result should be much closer to what
%    IEEE will actually publish. 
%
% 2) This IEEEtran.cls does provide _some_ old LaTeX 2.09 font
%    commands (\rm, \sf, \tt, \bf, \it, \sl, \sc, \cal and \mit).
%    It accomplishes this in the same way as article.cls so
%    there should be no problems here.
%    If you need the complete outdated 2.09 font commands you can
%    do a \usepackage{oldlfont} in your document, but note that
%    this introduces the \mathcal{} problem mentioned in bug #2 above.
%    So, you should try to avoid using oldlfont in LaTeX2e systems.
%
%    ALSO: The current IEEE demo file "Using the Document
%    Class IEEEtran.cls" will require \usepackage{rawfonts}
%    to be added after the \documentclass{} to work with this
%    IEEEtran.cls since the demo used an old 2.09 command \tenrm.
%    Alternatively, you can provide just the definition of \tenrm:
%    \DeclareFixedFont{\tenrm}{OT1}{cmr}{m}{n}{10} 
%
% 3) Still will not allow you to leave whitespace between \cite 
%    entries like book class will. i.e., \cite{einstein24, knuth84}
%    will not work, but \cite{einstein24,knuth84}
%    is OK. This *may* be normal.
%
% 4) You may need a blank line before you \begin a list
%    (or other environment) just after text that uses \PARstart.
%    Otherwise, \PARstart somehow manages to swallow your
%    \begin and LaTeX will complain about a lonely \item.
%    \PARstart won't do anything when using technote or draft
%    as these formats do not use a larger than normal starting letter.
%
% 5) If you invoke the draft option:
%    \documentclass[10pt,draft,twocolumn]{IEEEtran}
%    You will not be able to see your graphics in xdvi. 
%    This is normal.
%    
% 6) Provides class IEEEtran (You may wish to rename this file
%    to "IEEEtran.cls" after you save your original IEEEtran.cls)
%
% 7) Now defaults to using Times font (like IEEE uses).
%    The 1997 IEEEtran.cls used by INFOCOM also defaulted
%    to Times via inclusion of the times.sty package 
%    (which is "built-into" this IEEEtran.cls)
%    If you have font problems and/or want to go back
%    to the Computer Modern Fonts, just issue these commands
%    in the preamble of your document:
%
%    \renewcommand{\sfdefault}{cmss}
%    \renewcommand{\rmdefault}{cmr}
%    \renewcommand{\ttdefault}{cmtt}
%    
%    The big first letter of the document (\PARstart) now uses the
%    Times font. It now looks VERY much like what IEEE uses.
%    If you have font problems with Times, the Computer Modern
%    version of \PARstart is provided as \CMPARstart.
%    
%    We forewarned that the typewriter font of Times seems to be wider
%    than that of the Computer Modern. You'll only get about 42 characters 
%    (10pt) on a 21pica line. Some documents will have a problem with this. 
%    The current IEEEsample2e.tex, "Using the Document Class IEEEtran.cls"
%    by Gerry Murray and Silvano Balemi, has problems when used with the
%    Times typewriter font (overfull hboxes- Table I overfills too).
%    However, it seems that this IS the correct typewriter font IEEE uses.
%    You could try \small, but they might not like that.
%    Better to shorten (or break) your lines of text.
%   
% 8) TeX doesn't behave exactly like IEEE does (but close) when it comes
%    to line and page breaks. It is true that TeX doesn't "look ahead"
%    several pages to consider the overall document when deciding where to
%    break (IEEE software may well do this). However, I think that with some
%    more tweaking TeX can get even closer. It seems that IEEE lets the font
%    glue stretch more, and shrink a little more, than what the TeX Times 
%    fonts do. More tweaking on this will have to wait for future versions.
%    You can try:
%
%    \hyphenpenalty=800
%    \hbadness=2500
%
%    in the preamble of your document and see if you like the result better. 
%    TeX will then try harder not to use hyphenation. The \hbadness=2500 
%    raises the box badness level required for a warning to be issued. 
%    If you try this, watch out for lines that are a little too packed
%    for comfort. 
%    It may also help to build a \hyphenation list based on 
%    observations made from the IEEE Journal you plan to publish in.
%
% 9) When installing a class file into your <texmf> directory
%    (<texmf> is /usr/share/texmf on my RedHat 6.0 Linux system), 
%    i.e., <texmf>/tex/latex/base/IEEEtran.cls
%    on teTeX (UNIX) systems, you must refresh the hash table in
%    order for LaTeX to see the new class. This is because teTeX
%    systems do not actually parse the dir's for files as they
%    use a high speed lookup table instead.
%    The command:
%
%    texhash
%
%    will do the trick and refresh this table.
%    Of course, you must be root to do all this.
%    See "The teTeX Guide- Installation and Maintenance" by
%    Thomas Esser and Dirk Hillbrech for details.
%    Also, if there is a class, with the same name as the one
%    you are using, present in the current working directory, it
%    will override any class of the same name in the <texmf>
%    tree. If you chose not to rename the distribution file 
%    (IEEEtran_vXX.cls) to "IEEEtran.cls", LaTeX will issue a 
%    warning because it worries that they are not the same class.
%    The warning is harmless. 
%
%10) The rightmark (ex: "Shell: Fixing IEEEtran.cls") will no longer
%    be used if technote is selected as IEEE does not put the author's
%    name and title of the paper in the headings of technotes. 
%    The Journal name (leftmark) will be used instead.
%
%11) IEEE likes the two columns on the last page of two column
%    text to have the same length. A crude, but effective, way
%    to achieve this is to shorten the page length of the first 
%    column of the last page via:
%    \enlargethispage{-X.Xin}
%    where X.X is the value you find that makes the columns equal.
%    This hack will have to be adjusted if the document is
%    altered. It is perhaps most useful for those who need to
%    provide camera ready proofs (conferences, etc.). You may have
%    to manually copy in your bibliography (*.bbl file) if this
%    command needs to be located between \bibitems.
%
%12) To get pages without headings or page numbers (sometimes
%    required when submitting camera ready proofs) just issue:
%    \thispagestyle{empty}
%    \pagestyle{empty}
%    after \maketitle
%
%13) dvips note:  
%    Some LaTeX distributions have dvips configuration files (config.pdf)
%    which assume that the user will be working only with Computer 
%    Modern fonts. These configuration files remap certain character 
%    positions via the use of the dvips -G1 option (older versions of 
%    dvips may not have a -Gx option). If this remapping is done to a 
%    document that contains other fonts, such as Times, a problem will 
%    develop in the PostScript or PDF output. One symptom will be that
%    the ligature "fi" will appear as the British pound symbol.
%    The dvips -G1 option should be used ONLY if the document is ENTIRELY 
%    in Computer Modern fonts. In all other cases, the -G0 option should
%    be used. In particular, in MikTeX 2.0, you will have to use:
%    dvips -Ppdf -G0 <filename>
%    instead of:
%    dvips -Ppdf <filename>
%    if your document has any non Computer Modern fonts. This issue is
%    not specific to IEEEtran.cls and is applicable to all LaTeX documents.         
%    Thanks to K. Sivakumar, A. Goreham, and K. Lee 
%    for identifying this problem and providing a solution
%
%
%
%
% CHANGES AND ENHANCEMENTS:
% 1) Now has the ability to give you a publisher ID mark at the
%    bottom of the title page! Issue the command:
%    
%    \pubid{0000--0000/00\$00.00~\copyright~2001 IEEE}
%    
%    right AFTER \maketitle *AND* include the command:
%    
%    \pubidadjcol
%    
%    somewhere in the SECOND column text of the first (title)
%    page to "pull up" its text by two lines to clear the
%    ID text (as \pubid does for the first column). 
%    \pubidadjcol is one and two-column mode aware and there 
%    will be no problems if you later switch between one and
%    two column modes.
%    Future versions of IEEEtran.cls may not need \pubidadjcol,
%    but I don't know any way of eliminating it without resorting
%    to other packages like afterpage (on CTAN).
%    Consider this an open problem.
%    Note that, if in DRAFT mode, the publisher ID mark WILL NOT
%    be printed at the bottom of the titlepage, but room will be
%    cleared for it.
%    If you want further "realism" you can put
%    \thanks{Publisher Item Identifier X 0000-0000(00)00000-X.}
%    as your last \thanks in the \author command (in which ALL
%    \thanks belong). With this IEEEtran.cls, you can even fool
%    yourself into thinking that your work has already been published. ;)
%    Special thanks to Peter Wilson, Daniel Luecking, and the other
%    gurus at comp.text.tex, for helping me to understand how best to
%    implement this command in LaTeX.
%
% 2) If you are fortunate enough to have an invited paper, you can
%    try the new \specialpapernotice command:
%    
%    \specialpapernotice{(Invited Paper)}
%
%    BEFORE \maketitle. This will put the given text centered in 
%    italics after the author names.
%
% 3) The section, subsection, subsubsection and paragraph headings
%    now actually work the way IEEE wants:
%     
%          I. SECTION HEADINGS ARE IN CAPS AND CENTERED
%    
%    A. subsections are not indented
%    
%       1) subsubsections:  are indented like a paragraph. The body text
%    will wrap under the title.
%          a) paragraph sections:  are much like subsubsections, but are 
%    indented by twice that of a normal paragraph and use lowercase
%    letters as labels.
%
%    The headings for subsections and lower are all in italics. You
%    can use the *-form of the section commands if you want something
%    other than italics.
%    
%    Note that \paragraph sections are not available under technote, 
%    unless you \setcounter{secnumdepth}{4} which forces it back on
%    (nonstandard IEEE).
%    Rubber spacing lengths above section headings are now allowed to
%    stretch more like IEEE sometimes does. These lengths could possibly
%    tweaked more, but the current implementation seems OK.
%    
% 4) For a single appendix, issue \appendix before the start of your
%    appendix. If you wish your single appendix to have a title (heading),
%    use the new optional argument like:
%    
%    \appendix[Proof of the Zonklar Equations]
%
%    After issuing \appendix the \section command will be disabled
%    and any attempt to use \section will be ignored and a warning message
%    will be generated.
%    (the single appendix marks the end of the enumerated sections and the
%    section counter is fixed at zero - one does not state "see Appendix I"
%    when there is only one appendix, instead use "see Appendix".)
%
%    However, all lower \subsection commands and the \section* form
%    will work as normal as you need \section* at the end for things 
%    like acknowledgments:
%
%    \section*{Acknowledgment}
%    \addcontentsline{toc}{section}{Acknowledgment}
%
%    Use \appendices if you have more than one appendix.
%    You then declare each appendix via:
%
%    \section{Proof of First Zonklar Equation}
%    
%    You must remember to declare a \section before you create
%    any additional \subsections or \labels which refer to section
%    (or subsection, etc.) numbers!
%    The title text of an appendix can be empty, if you don't want a title:
%
%    \section{}
%
%    This IEEEtran.cls will automatically detect the empty argument
%    and "do the right thing" in the headings and table of contents 
%    (you won't have a blank line where the title should be or an 
%    ":" after the word Appendix in the table of contents.) As with
%    \appendix, the section* command and the lower \subsection commands
%    will still work as usual.
%    There are two appendix numbering conventions used by IEEE.
%    Capital letters (Appendix A) and Roman numbers (Appendix I).
%    The former appears to be more popular. However, since subsection
%    numbering conventions are preserved in the appendices, this creates
%    rather awkward subsection references like "A-A". For this reason, 
%    I have decided to have this IEEEtran.cls default to using Roman numbers
%    when labeling appendices. You can revert to using "A,B,.." if you wish 
%    by issuing the command:
%    \useRomanappendicesfalse
%    before you call \appendices.
% 
% 5) Enumerated list labels are now terminated with ) like 
%    "1)", which IEEE now uses, rather than "1."
% 
% 6) Theorem environment changed to display like 
%    "Theorem 1---Einstein:" which IEEE uses, instead of
%    "Theorem 1 (Einstein):"
% 
% 7) Figure captions adjusted: IEEE left (not center) justifies
%    figure captions and does not indent figure caption text.
% 
% 8) Adjusted some spacings in the table of contents(TOC))/list-of-figures/
%    list-of-tables so that section/table numbers will not so easily 
%    collide with the titles. Section VIII was usually the worst offender.
%    Still doesn't right justify the section numbers, but neither does 
%    article.cls (This must be why LaTeX likes the x.y.z section numbering
%    scheme unlike I, II, III, etc. of IEEE. )
%    It may be "normal" as it is (left justified). sigh.
% 
% 9) Now uses "index terms" now as a heading instead of "keywords".
%    Furthermore, the "index terms" and "abstract" headings are in bold
%    italic. This seems to be how IEEE does things now.
%
%10) \thebibliography and \biography now put entries into
%    the table of contents for you.
% 
%11) The margins IEEE uses are, in my opinion, strange. I have included
%    a new command, \overrideIEEEmargins, that when executed in the
%    beginning of your document, will change the margins to ensure that
%    the wider margin is always on the bound edge and that, when two
%    side printing, the margins on the front and back will actually line
%    up. \overrideIEEEmargins will have no effect if DRAFT is selected.
%    See the comments around line 1182 below for more details.
% 
% 
% OTHER NOTES:
% 1) See the companion file "IEEEtest.tex" for a demo of the
%    use some of the new features of this IEEEtran.cls. IEEEtest.tex
%    also contains code which allows .tex files using the new V1.3 features
%    to compile under the OLDER IEEEtran.cls (However, the formatting
%    will not be correct.) This may be useful for "comparison" or
%    within the IEEE editing department.
% 
% 
% 
% LIST USAGE DOCS (long):
% The most important thing to know about this class is that the
% itemize, enumerate and description list environments (I'll use
% acronym IED for short) are controlled in a new manner. The lower 
% level list environment, \list, remains virtually unchanged 
% (except \topsep increased to 0.3ex from 0.0ex).
%    In "normal" LaTeX, list layout is controlled via the lengths:
%
%    \leftmargin  determines the distance from the current left text
%                 margin to the left margin of the list text
%                 
%    \labelwidth  determines the width of the box reserved for the label
%                 to be justified in
%
%    \labelsep    determines the spacing between the label box and the 
%                 list text
%
% Note that the \leftmargin forms the reference point of the list from
% the main text. This is an unfortunate choice for the way of doing things
% since IEEE normally aligns the LABEL in some relation to the main text
% and then references the remainder of the list to the label boxes. For the
% record, IEEE's way of looking at lists makes a lot more sense to me---
% Is it not the label which the reader considers to be the first
% characters of a list item? In order to get a LaTeX \list to do what
% IEEE does, you have to determine your \labelwidth and then solve
% for a \leftmargin that puts the start of the label where it needs 
% to be (\labelsep is a constant). The IED environments are supposed to
% make all this easier for you by automatically taking care of some of
% these details. However, the IED environments of the older IEEEtran.cls 
% are "nuked" in that they use generic lengths which pretty much ensure 
% that the lists will be wrong almost all the time. The probable result is 
% that authors then turn to the more general \list which defeats the entire
% point of having IED environments in the first place!
%
%   Now consider the way this new IEEEtran.cls controls the IED
% environments via the lengths:
%
%   \labelindent  determines the indention of the label box from
%                 the current left text margin. Hence, THE
%                 LABEL IS THE REFERENCE POINT FOR THE LIST!
%
%   \labelwidth   as before, is the length of the label box.
%
%   \labelsep     as before, determines the spacing between the
%                 label box and the list text.
%
%   Of course, it is a matter of trivial addition and subtraction to convert
% between the way the new IEEEtran.cls and the way LaTeX normally looks
% at lists (\leftmargin = \labelindent + \labelwidth + \labelsep).
% What is important is this:
%
%   1) You can now think directly in IEEE terms.
%   2) The length controls now are available in the IED environments
%      so that you do not have to choose between good formatting (\list)
%      and automation (itemize, enumerate, and description).
%
% \labelindent and \labelsep are constants for a given list
% circumstance under IEEE. So, all you really need to do is determine which
% of your labels in a list is the longest. Everything other than that
% is usually automated correctly.
%
%                        _side note_
% Having to specify the longest label is nothing new in LaTeX. Does
% \begin{thebibliography}{99} ring a bell? LaTeX, in its current,
% form cannot determine longest labels for you as this would require
% more than one pass through your code. With bibliographies, BiBTeX
% actually sets the sample label for you. It would be nice if LaTeX
% offered some type of integrated multipass service to users- LiSTeX
% anyone?.
%
%
%
% - How to Control IED Lists -
% The length of the explanation below may give some the impression
% that IED list control is complex. It isn't. In fact, you will
% find it to be extremely easy to get your lists in proper format,
% while at the same time giving you flexibility never before possible
% with IED lists.
% 
% IF YOU DON'T READ ANYTHING ELSE, KNOW THAT YOU MUST MANUALLY
% SPECIFY THE LABELWIDTH IN THE LIST CONTROLS (discussed below) 
% FOR THE LONGEST LABEL IN YOUR LISTS FOR DESCRIPTION LISTS, 
% ENUMERATED LISTS OTHER THAN THE TOP LEVEL ENUMERATION (OR THOSE
% WITH MORE THAN 9) ITEMS) AND ITEMIZED LISTS IN WHICH YOU HAVE
% SPECIFIED A DIFFERENT LABEL IN ITEM[XX]. EVERYTHING ELSE WILL
% PRETTY MUCH RUN ITSELF ON "AUTOPILOT"!
% 
% The new IED lists ignore "outside" changes to all list lengths
% such as \leftmargin, \labelwidth, etc. The control of IED lists
% is now done exclusively via two interfaces:
% 
% 1. "group" control via the \iedlistdecl command. For example:
%    \renewcommand{\iedlistdecl}{\settowidth{\labelwidth}{Hello}}
%    at the start of your document will set the default width of the
%    labels in ALL later IED lists to be equal to the width of "Hello".
% 2. "individual" control via the OPTIONAL argument is provided to itemize,
%    enumerate, and description. For example, 
%    \begin{itemize}[\settowidth{\labelwidth}{$\gamma$}]
%    Within the list, the individual control is executed just after the
%    group control and therefore, the commands in the individual control
%    can override those in the group control. Individual controls affect
%    only the particular list they are given in.
%    
% Note: Because setting a \labelwidth is so commonly performed, this
% IEEEtrans.cls provides a command: \setlabelwidth{XX} 
% which is a shorter form of: \settowidth{\labelwidth}{XX} 
%   
% These two controls behave like the list_decl (The second 
% argument to the standard \list environment). In fact, the group
% and individual commands are passed directly to the list_decl of the
% underlying \list. The rational for this interface is four-fold:
%    
%    1. If a user doesn't like the default behavior, \iedlistdecl
%       can be defined in the beginning of the document in order
%       to affect every IED list after the definition.
%    2. The individual control can be used to override defaults for a
%       single IED list without affecting other lists.
%    3. Format controls to lists are provided in only two places, so IEEE
%       can easily redefine \iedlistdecl and provide a bogus
%       option to IED lists- which will allow them to easily 
%       isolate your formatting from your content.
%    4. Changes to IED list behavior are confined to IED lists-
%       You won't have other, seemingly unrelated, parts of the document
%       go nuts when you are trying to alter some list parameter. 
%       Furthermore, you can easily create complex nested IED lists
%       without "getting confused" as to what changes what.
%
%
%
% - Default IED Behavior -
% The default spacings the IED lists used are stored in lengths
% which begin with IEEE, i.e., \IEEEilabelindent. Changes to these
% "master" defaults are rarely needed. If you do need to change them,
% do so at the beginning of your document, NOT IN THE IED LIST 
% CONTROLS. These constants will now be briefly explained:
% 
% \IEEEilabelindent:
% IEEE seems to use at least two different values by
% which ITEMIZED list labels are indented to the right
% For The Journal of Lightwave Technology (JLT) and The Journal
% on Selected Areas in Communications (JSAC), they tend to use
% an indention equal to \parindent. For Transactions on Communications
% they tend to indent ITEMIZED lists a little more--- 1.3\parindent.
% The shorter length is stored as \IEEEilabelindentA the longer
% as \IEEEilabelindentB. The default is to use the shorter version.
% If you want to use the longer version do a
% setlength{\IEEEilabelindent}{\IEEEilabelindentB}
% at the beginning of your document.
% 
% \IEEEelabelindent
% This length is the default amount the enumerated list labels
% are indented to the right. Normally, the same as \parindent.
% 
% \IEEEdlabelindent
% ditto for description list labels. Normally, the same as \parindent.
% 
% \IEEEiednormlabelsep
% This length is the normal default spacing between the end of the IED
% list labels and the list text which will be used unless you
% specify something else.
%
% \IEEEiedmathlabelsep
% For nomenclature lists (when you have a list of math symbols with
% their explanations, IEEE usually increases the separation between 
% the labels and the explanations. This length is set to this longer
% than normal length. To invoke its use, just issue the command
% \usemathlabelsep in a list control.
%
% \IEEEiedtopsep
% This length is the extra vertical separation put above and
% below each IED list. IEEE usually puts a little extra spacing
% around each list. However, this extra spacing is barely noticeable.
%
% \IEEElabelindentfactori through \IEEElabelindentfactorvi
% IEEE normally decreases the amount of indention as the 
% list nesting level increases (there isn't much room to indent 
% with two column text). The actual amount labels will be indented is
% \labelindent multiplied by the factor above corresponding to the
% level of nesting depth. This provides a means by which the user can
% alter the effective \labelindent for deeper levels.
% There may not be such a thing as correct "standard IEEE"
% values. What IEEE actually does may depend on the specific
% circumstances.
% The first list level almost always has full indention.
% The second levels I've seen have only 75% of the normal indentation
% Three level or greater nestings are very rare. I am guessing
% that they don't use any indentation. These factors are not lengths
% but rather constant macros like \baselinestretch so you will
% have to use \renewcommand if you want to set them to something else.
% The default values are:
% \IEEElabelindentfactori   1.0     % almost always one
% \IEEElabelindentfactorii  0.75    % 0.0 or 1.0 may be used in some cases
% \IEEElabelindentfactoriii 0.0     % 0.75? 0.5? 0.0?
% \IEEElabelindentfactoriv  0.0
% \IEEElabelindentfactorv   0.0
% \IEEElabelindentfactorvi  0.0
% You may suspend the use of these factors in a list by issuing 
% the command \nolabelindentfactortrue in a list control (which
% has the same effect as setting all the indent factors to 1.0).
%
%
%
% - Other IED List Commands -
% 
% The \calcleftmargin command provides an easy way to set 
% \leftmargin based on the \labelwidth, \labelsep and the 
% argument which is the length you want to indent the label.
% Usage: \calcleftmargin{length-to-indent-the-label}
% output is in the \leftmargin variable, i.e., effectively:
% \leftmargin = argument + \labelwidth + \labelsep
% It is used internally by the IED lists, but it may be
% helpful to some users as well.
% You can't normally specify \leftmargin in a list control as 
% the IED lists calculate this value based on the \labelindent
% \labelwidth and \labelsep. To stop this auto-calculation, so that
% the list will use the \leftmargin you specify, just put 
% \nocalcleftmargintrue in a list control.
%
% The enhanced IED lists now allow you to override the
% default justification to perform on the labels by using the
% list control commands:
% \iedlabeljustifyl    for left label justification
% \iedlabeljustifyc    for center label justification
% \iedlabeljustifyr    for right label justification
% HOWEVER, THE USE OF THESE JUSTIFICATION OVERRIDES IS NOT
% RECOMMENDED FOR .tex FILES TO BE TURNED IN TO IEEE AS
% THIS MAY BE CONSIDERED NONSTANDARD FORMAT AND THEY MAY WELL
% USE THEIR DEFAULT JUSTIFICATION. 
% I include these commands as they may be helpful to
% those who are using these enhanced list controls for
% other non-IEEE related LaTeX work.
% itemize and enumerate automatically default to right
% justification, description defaults to left. 
%
%
%
% - ITEMIZED List Notes -
% The itemized lists will normally automatically calculate the length
% of whatever symbol the current list level is using so you can just
% call \begin{itemize} ... \end{itemize} without doing anything special.
% Furthermore, the auto-label-length feature will work properly even
% if you have redefined \labelitem~ (where I use ~ to indicate
% i,ii,iii, or iv, whichever is appropriate) BEFORE you call the begin 
% itemize. However, if you specify any item symbols via \item[X] 
% (this is really, really rare and may well be nonstandard as far as 
% IEEE is concerned) then you can do the following:
%
%
% \begin{itemize}[\setlabelwidth{Z}]
% \item[X] blah
% \item[Y] blah
% .
% .
% \end{itemize}
%
%   where Z is the longest label in the list
%
%
%
% - ENUMERATE List Notes -
% The important thing to note about enumerate lists is that the
% \labelwidth will default to the length of 9) in normal size and font.
% Therefore, you will have to manually specify the length of
% your longest label if:
% 
% 1. Your top level list has more than 9 items
% 2. You have redefined \labelenumi, have manually
%    specified labels with item[XX], or are using labels
%    other than the \normalfont \normalsize font.
% <or>
% 3. the enumerated list is nested (not at the top enumerate level, and is
%    therefore not using arabic digits as labels).
% 
% For example:
% 
% \begin{enumerate}[\setlabelwidth{12)}]
% \item blah
% \item blah
% .
% % 12 items total
% \end{enumerate}
%
%
%
% - DESCRIPTION List Notes -
% You will always have to specify your longest label width under
% description lists. Furthermore, you may wish to use the 
% \IEEEmathlabelsep for \labelsep when building a math symbol list.
% For example:
% 
% \begin{description}[\setlabelwidth{$\alpha\omega\pi\theta\mu$}
% \usemathlabelsep]
% \item[$\gamma\delta\beta$] Is the index of..
% \item[$\alpha\omega\pi\theta\mu$] Gives the optimal..
% .
% .
% \end{description}
%
% Be sure to include the same font commands in \setlabelwidth as
% you used with the actual label so that width calculation will be
% accurate.
% The description labels will be in normal font unless you
% specify otherwise. IEEE doesn't seem to mandate the use of
% italics. Some authors use normal, others bold, etc.
% 
% Here is how you could do a list in typewriter font:
%
% \begin{description}[\ttfamily \setlabelwidth{\texttt{10)}} 
%                     \iedlabeljustifyr
%                     \settowidth{\labelsep}{\texttt{\ }}]
% \item[\texttt{1)}] Start the algorithm;
% .
% .
% \item[\texttt{10)}] Tabulate the results.
% \end{description} 
%
% But, since the list is numbered and the justification needed to be changed
% (nonstandard IEEE), it would be better to use an enumerated list instead:
% 
% \renewcommand{\labelenumi}{\ttfamily\theenumi)}
% \begin{enumerate}[\ttfamily \setlabelwidth{\texttt{10)}} 
%                   \settowidth{\labelsep}{\texttt{\ }}]
% \item Start the algorithm;
% .
% .
% \item Tabulate the results.
% \end{enumerate}
%
%
% It is difficult to get things to line up "correctly" if you should
% mix font styles within the labels - which is nonstandard as far 
% as IEEE work is concerned.
% This has nothing to do with limitations of the IED list implementation,
% but rather differences in what constitutes a box containing each 
% font character. If you *have* to do something like this, I 
% suggest you practice with \framebox first. Commands which may be of
% help here are \newlength, \settowidth, and \makebox.
%
%
%  *** IEEE section headings can look like lists ***
% Don't confuse headings like \subsubsection, which look like:
%
% \par \textit{1) All-Optical Wavelength Conversion:} blah blah
%
% with enumerated lists. You can tell that it is not a list because 
% the text margin is not indented at all. 
% Also, sometimes they italicize the first few words in a list item
% text to make a "intralist-item-header".
%
%
%
%
% That's all folks.
% 
% ***** end of MDS comments *****
% *******
%
%
%
%
%
%
% *******
% 9/2000 (JVH) changes:
% 
% made some corrections to get closer to LaTeX2e
% 20000906 Juergen v.Hagen
% vonhagen@ihefiji.etec.uni-karlsruhe.de
% 
% Permission to redistribute granted as of December 2000.
% *******
%
%
%
%
%
%
% *******
% 
% original 1993 (JWD) LaTeX2e version:
%  
% In the most recent TeXhax digest, there was a request for a copy of
% IEEEtrans.sty modified to work with LaTeX2e.  I have a version I
% modified to make it IEEEtrans.cls, which I have sent to the person
% making the request and am now sending to you to consider posting to
% the archives.
% --
% Jon Dixon
% dixonj@colorado.edu
% http://spot.colorado.edu/~dixonj/
%
%*******
%
%
%
%
%
%
%*******
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  IEEEtran.cls  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%===========================================================================
%%% modified by JWD (dixonj@colorado.edu) to be IEEEtran.cls
%%%
%%%  @LaTeX-style-file{
%%%    author          = "Gerry Murray and Silvano Balemi",
%%%    version         = "2.5.4",
%%%    date            = "30-August-1993",
%%%    filename        = "IEEEtran.sty"- now "IEEEtran.cls"
%%%    address         = "Automatic Control Lab, ETH Zurich, Switzerland"
%%%    email           = "balemi@aut.ee.ethz.ch,
%%%    keywords        = "LaTeX, IEEE Transactions, publisher style",
%%%    mainserver      = "via E-mail from balemi@aut.ee.ethz.ch ",
%%%    docstring       = "This is a LaTeX class file for preparation
%%%                       of submission and final versions of articles
%%%                       to be published in IEEE Transactions.
%%%                       The default font size is 10 points but 9pt,
%%%                       11pt and 12pt are available too.
%%%
%%%                        Usage:
%%%                         \documentclass[..,Xpt,twoside]{IEEEtran}
%%%                         \author{..}
%%%                         \title{..}
%%%                         \maketitle
%%%                         \begin{abstract}...\end{abstract}
%%%                         \begin{keywords}...\end{keywords}
%%%                         ...
%%%                         \begin{biography}{Author's name}...\end{biography}
%%%                         \end{document}
%%%
%%%                        The command "\markboth{leftTEXT}{rightTEXT}"
%%%                       puts the text "leftTEXT" and "rightTEXT"
%%%                       with the page number on the even resp. odd
%%%                       headers when the "twoside" option is selected.
%%%                       (Attention: header of title page is
%%%                       always "leftTEXT" as it bears the journal name).
%%%                       In order to remove headers, insert
%%%                          \thispagestyle{empty}\pagestyle{empty} or
%%%                          \thispagestyle{plain}\pagestyle{plain}
%%%                       right after the \maketitle command!!
%%%
%%%                       See also IEEE.bst and IEEEsample2e.tex",
%%%  }
%%% ===========================================================================
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\typeout{Document Class `IEEEtran' V1.5 <26-July-2001> Modified by
Petros Maniatis}
\typeout{Read the source comments for docs.}
\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{MyTightStyle}[2003/01/31 revision]

% define new needed boolean variables
\newif\if@tmptwocolumn  \@tmptwocolumnfalse
\newif\if@draftversion  \@draftversionfalse
\newif\if@draftclsmode  \@draftclsmodefalse
\newif\if@technote      \@technotefalse

% IEEE uses letter paper
\setlength\paperheight{11in}
\setlength\paperwidth{8.5in}

\def\@ptsize{10}
\@namedef{ds@9pt}{\def\@ptsize{9}}
\@namedef{ds@10pt}{\def\@ptsize{10}}
\@namedef{ds@11pt}{\def\@ptsize{11}}
\@namedef{ds@12pt}{\def\@ptsize{12}}
\@namedef{ds@twoside}{\@twosidetrue \@mparswitchtrue}
% If the user selects draft, then this class AND any packages
% will go into draft mode.
\@namedef{ds@draft}{\global\@draftversiontrue \global\@draftclsmodetrue} 
% draftcls is for a draft mode which will not affect any packages
% used by the document.
\@namedef{ds@draftcls}{\global\@draftclsmodetrue} 
% we provide a final option just for completeness (article.cls has one)
\@namedef{ds@final}{\global\@draftversionfalse \global\@draftclsmodefalse}
\@namedef{ds@technote}{\global\@technotetrue}  % sets variable for techn. note
% the file twocolumn.sty is not read as it changes \textwidth.
\@namedef{ds@twocolumn}{\global\@tmptwocolumntrue}
\@options %execute the selected option commands


% The idea hinted here is for LaTeX to generate markleft{} and markright{}
% automatically for you after you enter \author{}, \journal{},
% \journaldate{}, journalvol{}, \journalnum{}, etc.
% However, there may be some backward compatibility issues here as
% well as some special applications for IEEEtran.cls like 
% conferences and special issues that may require the flexible
% \markleft{}, \markright{} and/or \markboth{}.
% We'll leave this as an open future suggestion.
%\newcommand{\journal}[1]{\def\@journal{#1}}
%\def\@journal{}



% FONT DEFINITION (No sizexx.clo file needed) 
% This actually works now.
%
% Check if we have selected 9 points
% used with technote
\def\@tempa{9}\ifx\@ptsize\@tempa
\typeout{-- This is a 9 point document}

%\def\@normalsize{\@setsize\normalsize{10.7pt}\ixpt\@ixpt
\def\@normalsize{\@setsize\normalsize{11.1pt}\ixpt\@ixpt
\abovedisplayskip 1em plus2pt minus5pt\belowdisplayskip \abovedisplayskip
\abovedisplayshortskip \z@ plus3pt\belowdisplayshortskip .6em plus3pt minus3pt}

%\def\small{\@setsize\small{9.12pt}\viiipt\@viipt}
%\def\footnotesize{\@setsize\footnotesize{8.15pt}\viipt\@vipt}
%\def\scriptsize{\@setsize\scriptsize{8pt}\vipt\@vpt}
%\def\tiny{\@setsize\tiny{5pt}\vpt\@vpt}
% footnotes are 8pt, so we'll make small 8pt too.
\def\small{\@setsize\small{9.12pt}\viiipt\@viiipt}
\def\footnotesize{\@setsize\footnotesize{9.12pt}\viiipt\@viiipt}
% I am guessing here that they don't go all the way down to 5pt
\def\scriptsize{\@setsize\scriptsize{8.15pt}\viipt\@viipt}
\def\tiny{\@setsize\tiny{8pt}\vipt\@vipt}
% I think they actually use like 10.5pt for titles rather
% than 10pt (\xpt), sigh.
\def\large{\@setsize\large{12pt}\xpt\@xpt}
\def\Large{\@setsize\Large{14pt}\xiipt\@xiipt}
\def\LARGE{\@setsize\LARGE{18pt}\xivpt\@xivpt}
\def\huge{\@setsize\huge{22pt}\xviipt\@xviipt}
\def\Huge{\@setsize\Huge{25pt}\xxpt\@xxpt}
\fi
%
% Check if we have selected 10 points
\def\@tempa{10}\ifx\@ptsize\@tempa
\typeout{-- This is a 10 point document}
%\def\@normalsize{\@setsize\normalsize{11.9pt}\xpt\@xpt
% IEEE uses a little more baselineskip
\def\@normalsize{\@setsize\normalsize{12.00pt}\xpt\@xpt
\abovedisplayskip 1em plus2pt minus5pt\belowdisplayskip \abovedisplayskip
\abovedisplayshortskip \z@ plus3pt\belowdisplayshortskip .6em plus3pt minus3pt}

% small should be 9pt, not 8pt
%\def\small{\@setsize\small{9.2pt}\viiipt\@viiipt}
\def\small{\@setsize\small{10pt}\ixpt\@ixpt}
\def\footnotesize{\@setsize\footnotesize{8.8pt}\viiipt\@viiipt}
\def\scriptsize{\@setsize\scriptsize{8pt}\viipt\@viipt}
\def\tiny{\@setsize\tiny{6pt}\vpt\@vpt}
\def\large{\@setsize\large{14pt}\xiipt\@xiipt}
\def\Large{\@setsize\Large{18pt}\xivpt\@xivpt}
\def\LARGE{\@setsize\LARGE{22pt}\xviipt\@xviipt}
\def\huge{\@setsize\huge{22pt}\xxpt\@xxpt}
\def\Huge{\@setsize\Huge{28pt}\xxvpt\@xxvpt}
\fi
%
% Check if we have selected 11 points
\def\@tempa{11}\ifx\@ptsize\@tempa
\typeout{-- This is an 11 point document}

\def\@normalsize{\@setsize\normalsize{13.6pt}\xipt\@xipt
\abovedisplayskip 1em plus2pt minus5pt\belowdisplayskip \abovedisplayskip
\abovedisplayshortskip \z@ plus3pt\belowdisplayshortskip .6em plus3pt minus3pt}

\def\small{\@setsize\small{12pt}\xpt\@xpt}
\def\footnotesize{\@setsize\footnotesize{11pt}\ixpt\@ixpt}
\def\scriptsize{\@setsize\scriptsize{9.5pt}\viiipt\@viiipt}
\def\tiny{\@setsize\tiny{7pt}\vipt\@vipt}
\def\large{\@setsize\large{14pt}\xiipt\@xiipt}
\def\Large{\@setsize\Large{18pt}\xivpt\@xivpt}
\def\LARGE{\@setsize\LARGE{22pt}\xviipt\@xviipt}
\def\huge{\@setsize\huge{25pt}\xxpt\@xxpt}
\def\Huge{\@setsize\Huge{30pt}\xxvpt\@xxvpt}
\fi
%
% Check if we have selected 12 points
\def\@tempa{12}\ifx\@ptsize\@tempa
\typeout{-- This is a 12 point document}

\def\@normalsize{\@setsize\normalsize{14pt}\xiipt\@xiipt
\abovedisplayskip 1em plus3pt minus6pt\belowdisplayskip \abovedisplayskip
\abovedisplayshortskip \z@ plus3pt\belowdisplayshortskip .6em plus4pt minus4pt}

\def\small{\@setsize\small{11.4pt}\xpt\@xpt}
\def\footnotesize{\@setsize\footnotesize{10pt}\ixpt\@ixpt}
\def\scriptsize{\@setsize\scriptsize{9pt}\viiipt\@viiipt}
\def\tiny{\@setsize\tiny{8pt}\vipt\@vipt}
\def\large{\@setsize\large{18pt}\xivpt\@xivpt}
\def\Large{\@setsize\Large{22pt}\xviipt\@xviipt}
\def\LARGE{\@setsize\LARGE{25pt}\xxpt\@xxpt}
\def\huge{\@setsize\huge{30pt}\xxvpt\@xxvpt}
\let\Huge=\huge
\fi

% define \normalsize to use what we set @normalsize to above
\renewcommand{\normalsize}{\@normalsize}
% now default to the normal size
\normalsize


% IEEE uses Times font, so we'll default to times.
% These three commands are the entire times.sty package.
\renewcommand{\sfdefault}{phv}
\renewcommand{\rmdefault}{ptm}
\renewcommand{\ttdefault}{pcr}


% set sizes and margins
% old values, metric is not popular in USA book printing.
% Heck, they don't even seem to use inches. It is a world
% where point and pica (12pt) reign supreme. IEEE textwidth 
% is 21 pica. They have a colsep of 1 pica.
%\topmargin      -16.0mm
%\oddsidemargin  -11.0mm
%\evensidemargin -11.0mm
%\textheight     243.5mm
%\textwidth      183.0mm
%\columnsep        4.1mm
\topmargin        -49.0pt
\textheight        58pc
\textwidth         43pc   % 2 x 21pc + 1pc = 43pc
\columnsep          1pc


% IEEE MARGIN INFO and new \overrideIEEEmargins command
% The IEEE margins may look strange because they are.
% After binding, IEEE physically cuts off (I think!) about 0.16in 
% from the OUTSIDE margin of the pages, yielding a finished book
% that is about 11in long and 8.34in wide.
% So, the wider margin goes on the OUTSIDE edge of the page.
% But, this doesn't explain everything. Looking at a reprint
% of a paper of mine, (circa 1997), I note that IEEE puts the
% "narrower" margin on the LEFT side ALWAYS- even when two side
% (duplex) printed! They even do this in the PDF files of
% the Journal Collection CDs I have. So, with the reprints and CDs,
% the margins on the front and back of the pages DO NOT LINE UP. 
% Now, I understand that a paper could begin on an even page, and 
% they have to make the title page the first page of the reprints, but
% this would be easy to fix by swapping the even and odd side margins.
% However, in the actual Journal, they get it right and the margins
% on the front and back of the pages line up! I guess the fellow
% that runs the printing press also sets the margins!
% 
% However, this is really crappy if YOU want to print and bind a copy
% of work made with this class. So, here is a new command 
% \overrideIEEEmargins
% which when executed in the beginning of the document or preamble, 
% will ensure that the wider margin is always on the edge where the page
% is bound and that the margins of the front and back of the pages
% (twoside) will line up- like article.cls does.

\newdimen\@IEEEmarginA
\newdimen\@IEEEmarginB

\@IEEEmarginA -30.0pt % this is the normal IEEE left margin
% (after you add 1in) for odd and EVEN pages
% The OTHER (wider) margin = \paperwidth -2in -\textwidth - \@IEEEmarginA
% the 2in is due to the LaTeX margin offset 
\setlength{\@IEEEmarginB}{\paperwidth}
\addtolength{\@IEEEmarginB}{-2in}
\addtolength{\@IEEEmarginB}{-1\textwidth}
\addtolength{\@IEEEmarginB}{-1\@IEEEmarginA}

\oddsidemargin    \@IEEEmarginA
\evensidemargin   \@IEEEmarginA

% execute \overrideIEEEmargins in your preamble and
% your document will have a "more normal" margins and will
% be much more agreeable to being bound and/or printed by
% a duplex printer (The margins on both sides of the page
% will line up.) 
\def\overrideIEEEmargins{\if@draftclsmode\relax\else %
 \if@twoside
 \oddsidemargin\@IEEEmarginB
 \evensidemargin\@IEEEmarginA
 \else
 \oddsidemargin\@IEEEmarginB
 \evensidemargin\@IEEEmarginB
 \fi
\fi}


\parindent        1.0em
\headsep          6.3mm
\headheight        12pt
\lineskip           1pt
\normallineskip     1pt
\def\baselinestretch{1}

% normally zero, should be relative to font height.
% put in a little rubber to help stop bad breaks
\parskip 0ex plus 0.2ex minus 0.2ex

\if@draftclsmode 
\topmargin       -6.0mm
\oddsidemargin       0mm
\evensidemargin      0mm
\textheight     223.5mm
\textwidth      161.0mm
\def\baselinestretch{1.5} % controls line spacing for draft version
\fi                       % some people may like 1.7 or greater
                          % so that there will be even more space
                          % for hand written comments 

% margin note stuff
\marginparsep      10pt
\marginparwidth    20pt
\marginparpush     25pt


% SPACING CONTROLS

% equations
\abovedisplayskip       0pt plus2pt minus5pt%
\belowdisplayskip       \abovedisplayskip
\abovedisplayshortskip  0pt plus3pt%
\belowdisplayshortskip  0pt plus3pt minus3pt%


% Controls the amount of EXTRA spacing
% above and below \trivlist 
% Both \list and IED lists override this.
% However, \trivlist will use this as will most
% things built from \trivlist like the \center
% environment.
\topsep           1.3ex

% Controls the additional spacing around lists preceded
% or followed by blank lines. IEEE does not increase
% spacing before or after paragraphs so it is set to zero.
% \z@ is the same as zero, but faster.
\partopsep          \z@

% Controls the spacing between paragraphs in lists. 
% IEEE does not increase spacing before or after paragraphs
% so this is also zero. 
% With IEEEtran.cls, global changes to
% this value DO affect lists (but not IED lists).
\parsep             \z@

% Controls the extra spacing between list items. 
% IEEE does not put extra spacing between items.
% With IEEEtran.cls, global changes to this value DO affect
% lists (but not IED lists).
\itemsep            \z@

% \itemindent is the amount to indent the FIRST line of a list
% item. It is auto set to zero within the \list environment. To alter
% it, you have to do so when you call the \list.
% However, IEEE uses this for the theorem environment
% There is an alternative value for this near \leftmargini below
\itemindent         -1em

% \leftmargin, the spacing from the left margin of the main text to
% the left of the main body of a list item is set by \list.
% Hence this statement does nothing for lists.
% But, quote and verse do use it for indention.
\leftmargin         2em

% we retain this stuff from the older IEEEtran.cls so that \list
% will work the same way as before. However, itemize, enumerate and
% description (IED) could care less about what these are as they
% all are overridden.
\leftmargini        2em
%\itemindent         2em  % Alternative values: sometimes used.
%\leftmargini        0em
\leftmarginii       1em
\leftmarginiii    1.5em
\leftmarginiv     1.5em
\leftmarginv      1.0em
\leftmarginvi     1.0em
\labelsep           5pt  % probably should be based on font size (Xem)
\labelwidth         \z@


% The old IEEEtran.cls behavior of \list is retained.
% However, the new V1.3 IED list environments override all the
% @list stuff (\@listX is called within \list for the
% appropriate level just before the user's list_decl is called). 
% \topsep is now 0.3ex as IEEE puts this little extra space around
% lists
% Note that \parsep and \itemsep are not redefined as in 
% the sizexx.clo \@listX (which article.cls uses) so global changes
% of these values DO affect \list
% 
\def\@listI{\leftmargin\leftmargini}
\def\@listi{\leftmargin\leftmargini \topsep 0.3ex plus 1pt minus 1pt}
\let\@listI\@listi
\def\@listii{\leftmargin\leftmarginii\labelwidth\leftmarginii
    \advance\labelwidth-\labelsep \topsep 0.3ex}
\def\@listiii{\leftmargin\leftmarginiii\labelwidth\leftmarginiii
    \advance\labelwidth-\labelsep \topsep 0.3ex}
\def\@listiv{\leftmargin\leftmarginiv\labelwidth\leftmarginiv
    \advance\labelwidth-\labelsep \topsep 0.3ex}
\def\@listv{\leftmargin\leftmarginv\labelwidth\leftmarginv
    \advance\labelwidth-\labelsep \topsep 0.3ex}
\def\@listvi{\leftmargin\leftmarginvi\labelwidth\leftmarginvi
    \advance\labelwidth-\labelsep \topsep 0.3ex}


% IEEE uses 5) not 5.
\def\labelenumi{\theenumi)}     \def\theenumi{\arabic{enumi}}

% IEEE uses a) not (a)
\def\labelenumii{\theenumii)}  \def\theenumii{\alph{enumii}}

% IEEE uses iii) not iii.
\def\labelenumiii{\theenumiii)} \def\theenumiii{\roman{enumiii}}

% IEEE uses A) not A.
\def\labelenumiv{\theenumiv)}   \def\theenumiv{\Alph{enumiv}}

% exactly the same as in article.cls
\def\p@enumii{\theenumi}
\def\p@enumiii{\theenumi(\theenumii)}
\def\p@enumiv{\p@enumiii\theenumiii}

% itemized list label styles
\def\labelitemi{$\scriptstyle\bullet$}
\def\labelitemii{\textbf{--}}
\def\labelitemiii{$\ast$}
\def\labelitemiv{$\cdot$}


% IEEEtran.cls version numbers, provided as of V1.3
% These values serve as a way a .tex file can
% determine if the new features are provided.
% The version number of this IEEEtrans.cls can be obtained from 
% these values. i.e., V1.4
% KEEP THESE AS INTEGERS! i.e., NO {4a} or anything like that-
% (no need to enumerate "a" minor changes here)
\def\IEEEtransversionmajor{1}
\def\IEEEtransversionminor{5}


% **** V1.3 ENHANCEMENTS ****
% Itemize, Enumerate and Description (IED) List Controls
% ***************************
% 
% 
% IEEE seems to use at least two different values by
% which ITEMIZED list labels are indented to the right
% For The Journal of Lightwave Technology (JLT) and The Journal
% on Selected Areas in Communications (JSAC), they tend to use
% an indention equal to \parindent. For Transactions on Communications
% they tend to indent ITEMIZED lists a little more--- 1.3\parindent.
% We'll provide both values here for you so that you can choose 
% which one you like in your document using a command such as:
% setlength{\IEEEilabelindent}{\IEEEilabelindentB}
\newdimen\IEEEilabelindentA
\IEEEilabelindentA \parindent

\newdimen\IEEEilabelindentB
\IEEEilabelindentB 1.3\parindent
% However, we'll default to using \parindent
% which makes more sense to me
\newdimen\IEEEilabelindent
\IEEEilabelindent \IEEEilabelindentA


% This controls the default amount the enumerated list labels
% are indented to the right.
% Normally, this is the same as the paragraph indention
\newdimen\IEEEelabelindent
\IEEEelabelindent \parindent

% This controls the default amount the description list labels
% are indented to the right.
% Normally, this is the same as the paragraph indention
\newdimen\IEEEdlabelindent
\IEEEdlabelindent \parindent

% This is the value actually used within the IED lists.
% The IED environments automatically set its value to
% one of the three values above, so global changes do 
% not have any effect
\newdimen\labelindent
\labelindent \parindent

% The actual amount labels will be indented is
% \labelindent multiplied by the factor below
% corresponding to the level of nesting depth
% This provides a means by which the user can
% alter the effective \labelindent for deeper
% levels
% There may not be such a thing as correct "standard IEEE"
% values. What IEEE actually does may depend on the specific
% circumstances.
% The first list level almost always has full indention.
% The second levels I've seen have only 75% of the normal indentation
% Three level or greater nestings are very rare. I am guessing
% that they don't use any indentation.
\def\IEEElabelindentfactori{1.0}   % almost always one
\def\IEEElabelindentfactorii{0.75} % 0.0 or 1.0 may be used in some cases
\def\IEEElabelindentfactoriii{0.0} % 0.75? 0.5? 0.0?
\def\IEEElabelindentfactoriv{0.0}
\def\IEEElabelindentfactorv{0.0}
\def\IEEElabelindentfactorvi{0.0}

% value actually used within IED lists, it is auto
% set to one of the 6 values above
% global changes here have no effect
\def\labelindentfactor{1.0}

% This controls the default spacing between the end of the IED
% list labels and the list text, when normal text is used for
% the labels.
\newdimen\IEEEiednormlabelsep
\IEEEiednormlabelsep 0.6em

% This controls the default spacing between the end of the IED
% list labels and the list text, when math symbols are used for
% the labels (nomenclature lists). IEEE usually increases the 
% spacing in these cases
\newdimen\IEEEiedmathlabelsep
\IEEEiedmathlabelsep 1.2em


% This controls the extra vertical separation put above and
% below each IED list. IEEE usually puts a little extra spacing
% around each list. However, this spacing is barely noticeable.
\newdimen\IEEEiedtopsep
\IEEEiedtopsep 0.3ex


% This command is executed within each IED list environment
% at the beginning of the list. You can use this to set the 
% parameters for some/all your IED list(s) without disturbing 
% global parameters that affect things other than lists.
% i.e., renewcommand{\iedlistdecl}{\setlength{\labelsep}{5em}}
% will alter the \labelsep for the next list(s) until 
% \iedlistdecl is redefined. 
\def\iedlistdecl{\relax}

% This command provides an easy way to set \leftmargin based
% on the \labelwidth, \labelsep and the argument \labelindent
% Usage: \calcleftmargin{width-to-indent-the-label}
% output is in the \leftmargin variable, i.e., effectively:
% \leftmargin = argument + \labelwidth + \labelsep
\def\calcleftmargin#1{\setlength{\leftmargin}{#1}
\addtolength{\leftmargin}{\labelwidth}
\addtolength{\leftmargin}{\labelsep}}

% This command provides an easy way to set \labelwidth to the
% width of the given text. It is the same as
% \settowidth{\labelwidth}{label-text}
% and useful as a shorter alternative.
% Typically used to set \labelwidth to be the width
% of the longest label in the list
\def\setlabelwidth#1{\settowidth{\labelwidth}{#1}}

% When this command is executed, IED lists will use the 
% IEEEiedmathlabelsep label separation rather than the normal
% spacing. To have an effect, this command must be executed via
% the \iedlistdecl or within the option of the IED list
% environments.
\def\usemathlabelsep{\setlength{\labelsep}{\IEEEiedmathlabelsep}}

% A flag which controls whether the IED lists automatically
% calculate \leftmargin from \labelindent, \labelwidth and \labelsep
% Useful if you want to specify your own \leftmargin
% This flag must be set (\nocalcleftmargintrue or \nocalcleftmarginfalse) 
% via the \iedlistdecl or within the option of the IED list
% environments to have an effect.
\newif\ifnocalcleftmargin
\nocalcleftmarginfalse

% A flag which controls whether \labelindent is multiplied by
% the \labelindentfactor for each list level.
% This flag must be set via the \iedlistdecl or within the option 
% of the IED list environments to have an effect.
\newif\ifnolabelindentfactor
\nolabelindentfactorfalse


% internal variable to indicate type of IED label
% justification
% 0 - left; 1 - center; 2 - right
\newcount\@iedjustify \@iedjustify=0


% commands to allow the user to control IED
% label justifications. Use these commands within
% the IED environment option or in the \iedlistdecl
% Note that changing the normal list justifications
% is nonstandard and IEEE may not like it if you do so!
% I include these commands as they may be helpful to
% those who are using these enhanced list controls for
% other non-IEEE related LaTeX work.
% itemize and enumerate automatically default to right
% justification, description defaults to left.
\def\iedlabeljustifyl{\@iedjustify=0} %left
\def\iedlabeljustifyc{\@iedjustify=1} %center
\def\iedlabeljustifyr{\@iedjustify=2} %right

% create temporary copies of all list parameters 
\newdimen\@iedtopsep
\newdimen\@iedlabelwidth
\newdimen\@iedlabelsep
\newdimen\@iedleftmargin
\newdimen\@iedpartopsep
\newdimen\@iedparsep
\newdimen\@ieditemsep
\newdimen\@iedrightmargin
\newdimen\@iedlistparindent
\newdimen\@ieditemindent

% commands to save to and restore from the list parameter copies
% this allows us to set all the list parameters within
% the list_decl and prevent \list (and its \@list) 
% from overriding any of our parameters
\def\@savelistparams{\@iedtopsep\topsep
\@iedlabelwidth\labelwidth
\@iedlabelsep\labelsep
\@iedleftmargin\leftmargin
\@iedpartopsep\partopsep
\@iedparsep\parsep
\@ieditemsep\itemsep
\@iedrightmargin\rightmargin
\@iedlistparindent\listparindent
\@ieditemindent\itemindent}

\def\@restorelistparams{\topsep\@iedtopsep
\labelwidth\@iedlabelwidth
\labelsep\@iedlabelsep
\leftmargin\@iedleftmargin
\partopsep\@iedpartopsep
\parsep\@iedparsep
\itemsep\@ieditemsep
\rightmargin\@iedrightmargin
\listparindent\@iedlistparindent
\itemindent\@ieditemindent}

% provide all these commands
% we will then override them later 
% we have to do things this way since the base LaTeX
% (latex.ltx) defines itemize and enumerate,
% but not description and we want to use \renewenvironment
\def\itemize{\relax}
\let\enditemize =\relax
\def\enumerate{\relax}
\let\endenumerate =\relax
\def\description{\relax}
\let\enddescription =\relax

% DO NOT ALLOW BLANK LINES TO BE IN THESE IED ENVIRONMENTS
% AS THIS WILL FORCE NEW PARAGRAPHS AFTER THE IED LISTS
% new itemized list MDS 1/2001
\renewenvironment{itemize}[1][\relax]
               {\ifnum \@itemdepth >3 \@toodeep\else
                \ifnum \@listdepth >5 \@toodeep\else
                \advance\@itemdepth\@ne 
                \edef\@itemitem{labelitem\romannumeral\the\@itemdepth}%
                % get the labelindentfactor for this level
                \advance\@listdepth\@ne % we need to know what the level WILL be
                \edef\labelindentfactor{\csname IEEElabelindentfactor\romannumeral\the\@listdepth\endcsname}
                \advance\@listdepth-\@ne % undo our increment
                \@iedjustify=2 % right justified labels are default
                % set other defaults
                \nocalcleftmarginfalse
                \nolabelindentfactorfalse
                \topsep \IEEEiedtopsep
                \labelindent\IEEEilabelindent
                \labelsep\IEEEiednormlabelsep
                \partopsep 0ex
                \parsep 0ex
                \itemsep 0ex
                \rightmargin 0em
                \listparindent 0em
                \itemindent 0em
                % calculate the label width
                % the user can override this later if
                % they specified a \labelwidth
                \settowidth{\labelwidth}%
                {\csname labelitem\romannumeral\the\@itemdepth\endcsname}
                \@savelistparams % save our list parameters
                \list{\csname\@itemitem\endcsname}{%
                \@restorelistparams % override any list{} changes
                                    % to our globals
                \iedlistdecl % let user alter parameters
                #1
                % If the user has requested not to use the
                % labelindent factor, don't revise \labelindent
                \ifnolabelindentfactor \relax
                \else \labelindent =\labelindentfactor\labelindent
                \fi
                % Unless the user has requested otherwise,
                % calculate our left margin based
                % on \labelindent, \labelwidth and
                % \labelsep
                \ifnocalcleftmargin \relax
                \else \calcleftmargin{\labelindent} 
                \fi
                % justify the label appropriately
                \let\makelabel\@iedmakelabelright % right if don't know
                                                  % what else to do 
                \ifnum \@iedjustify =0 \let\makelabel\@iedmakelabelleft \fi
                \ifnum \@iedjustify =1 \let\makelabel\@iedmakelabelcenter \fi
}\fi\fi}%
{\endlist}

% DO NOT ALLOW BLANK LINES TO BE IN THESE IED ENVIRONMENTS
% AS THIS WILL FORCE NEW PARAGRAPHS AFTER THE IED LISTS
% new enumerate list MDS 1/2001
\renewenvironment{enumerate}[1][\relax]
               {\ifnum \@enumdepth >3 \@toodeep\else
                \ifnum \@listdepth >5 \@toodeep\else
                \advance\@enumdepth\@ne
                \edef\@enumctr{enum\romannumeral\the\@enumdepth}%
                % get the labelindentfactor for this level
                \advance\@listdepth\@ne % we need to know what the level WILL be
                \edef\labelindentfactor{\csname IEEElabelindentfactor\romannumeral\the\@listdepth\endcsname}
                \advance\@listdepth-\@ne % undo our increment
                \@iedjustify=2 % right justified labels are default
                % set other defaults
                \nocalcleftmarginfalse
                \nolabelindentfactorfalse
                \topsep \IEEEiedtopsep
                \labelindent\IEEEelabelindent
                \labelsep\IEEEiednormlabelsep
                \partopsep 0ex
                \parsep 0ex
                \itemsep 0ex
                \rightmargin 0em
                \listparindent 0em
                \itemindent 0em
                % calculate the label width
                % We'll set it to the width suitable for all labels using
                % normalfont 1) to 9)
                % The user can override this later
                \settowidth{\labelwidth}{9)}
                \@savelistparams % save our list parameters
                \list{\csname label\@enumctr\endcsname}{%
                \usecounter{\@enumctr}
                \@restorelistparams % override any list{} changes
                                    % to our globals
                \iedlistdecl % let user alter parameters 
                #1
                % If the user has requested not to use the
                % labelindent factor, don't revise \labelindent
                \ifnolabelindentfactor \relax
                \else \labelindent =\labelindentfactor\labelindent
                \fi
                % Unless the user has requested otherwise,
                % calculate our left margin based
                % on \labelindent, \labelwidth and
                % \labelsep
                \ifnocalcleftmargin \relax
                \else \calcleftmargin{\labelindent} 
                \fi          
                % justify the label appropriately
                \let\makelabel\@iedmakelabelright % right if don't know
                                                  % what else to do 
                \ifnum \@iedjustify =0 \let\makelabel\@iedmakelabelleft \fi
                \ifnum \@iedjustify =1 \let\makelabel\@iedmakelabelcenter \fi
}\fi\fi}%
{\endlist}

% DO NOT ALLOW BLANK LINES TO BE IN THESE IED ENVIRONMENTS
% AS THIS WILL FORCE NEW PARAGRAPHS AFTER THE IED LISTS
% new description list MDS 1/2001
\renewenvironment{description}[1][\relax]
               {\ifnum \@listdepth >5 \@toodeep\else
                % get the labelindentfactor for this level
                \advance\@listdepth\@ne % we need to know what the level WILL be
                \edef\labelindentfactor{\csname IEEElabelindentfactor\romannumeral\the\@listdepth\endcsname}
                \advance\@listdepth-\@ne % undo our increment
                \@iedjustify =0 % left justified labels are default
                % set other defaults
                \nocalcleftmarginfalse
                \nolabelindentfactorfalse
                \topsep \IEEEiedtopsep 
                \labelindent\IEEEdlabelindent
                % assume normal labelsep
                \labelsep\IEEEiednormlabelsep
                \partopsep 0ex
                \parsep 0ex
                \itemsep 0ex
                \rightmargin 0em
                \listparindent 0em
                \itemindent 0em
                % Bogus label width in case the user forgets
                % to set it.
                % TIP: If you want to see what a variable's width is you
                % can use the TeX command \showthe\width-variable to 
                % display it on the screen during compilation 
                % (This might be helpful to know when you need to find out
                % which label is the widest)
                \settowidth{\labelwidth}{Hello}
                \@savelistparams  % save our list parameters
                \list{}{\@restorelistparams % override any list{} changes
                                            % to our globals
                \iedlistdecl % let user alter parameters 
                #1 
                % If the user has requested not to use the
                % labelindent factor, don't revise \labelindent
                \ifnolabelindentfactor \relax
                \else \labelindent =\labelindentfactor\labelindent
                \fi
                % Unless the user has requested otherwise,
                % calculate our left margin based
                % on \labelindent, \labelwidth and
                % \labelsep
                \ifnocalcleftmargin \relax
                \else \calcleftmargin{\labelindent} 
                \fi
                % justify the label appropriately
                \let\makelabel\@iedmakelabelleft   % left if don't know
                                                   % what else to do 
                \ifnum \@iedjustify =1 \let\makelabel\@iedmakelabelcenter \fi
                \ifnum \@iedjustify =2 \let\makelabel\@iedmakelabelright \fi}
\fi}%
{\endlist}

\newcommand*\@iedmakelabelleft[1]{\makebox[\labelwidth][l]{\normalfont #1}}
\newcommand*\@iedmakelabelright[1]{\makebox[\labelwidth][r]{\normalfont #1}}
\newcommand*\@iedmakelabelcenter[1]{\makebox[\labelwidth][c]{\normalfont #1}}


% VERSE and QUOTE
\def\verse{\let\\=\@centercr
    \list{}{\itemsep\z@ \itemindent -1.5em \listparindent \itemindent
    \rightmargin\leftmargin\advance\leftmargin 1.5em}\item[]}
\let\endverse\endlist
\def\quotation{\list{}{\listparindent 1.5em \itemindent\listparindent
    \rightmargin\leftmargin \parsep 0pt plus 1pt}\item[]}
\let\endquotation=\endlist
\def\quote{\list{}{\rightmargin\leftmargin}\item[]}
\let\endquote=\endlist


% Titlepage
\newif\if@restonecol
\def\titlepage{\@restonecolfalse\if@twocolumn\@restonecoltrue\onecolumn
    \else \newpage \fi \thispagestyle{empty}\c@page\z@}
\def\endtitlepage{\if@restonecol\twocolumn \else \newpage \fi}

\arraycolsep     5pt
\tabcolsep       6pt
\arrayrulewidth .4pt
\doublerulesep   2pt
\tabbingsep\labelsep

%% FOOTNOTES
%
\skip\footins 10pt plus 4pt minus 2pt
\footnotesep 7pt
\footskip 10mm
\skip\@mpfootins = \skip\footins
\fboxsep = 3pt \fboxrule = .4pt
\long\def\@makefntext#1{\parindent .8em\indent$^{\@thefnmark}$#1}
\def\footnoterule{}

\if@technote
   \setcounter{secnumdepth}{3}
\else
   \setcounter{secnumdepth}{4}
\fi


\newcounter{section}
\newcounter{subsection}[section]
\newcounter{subsubsection}[subsection]
\newcounter{paragraph}[subsubsection]


% as shown in table of contents and when called by user from \ref, \label
\def\theequation{\arabic{equation}}                          % 1
\def\thesection{\arabic{section}}                             % 1.
\def\thesubsection{\thesection.\arabic{subsection}}            % 1.1
\def\thesubsubsection{\thesubsection.\arabic{subsubsection}} % 1.1.1
\def\theparagraph{\thesubsubsection.\alph{paragraph}}        % I.1.1.a

% Main text forms (how shown in main text headings)
\def\thesectiondis{\arabic{section}.}               % 1.
\def\thesubsectiondis{\thesection.\arabic{subsection}}          % 1.1
\def\thesubsubsectiondis{\thesubsection.\arabic{subsubsection}}  % 1.1.1
\def\theparagraphdis{\alph{paragraph})}            % d)


% LIST OF FIGURES AND TABLES AND TABLE OF CONTENTS
%
\def\@pnumwidth{1.55em}
\def\@tocrmarg{2.55em}
\def\@dotsep{4.5}
\setcounter{tocdepth}{3}

% adjusted some spacings here so that section numbers will not easily 
% collide with the section titles. 
% VIII; VIII-A; and VIII-A.1 are usually the worst offenders.
% MDS 1/2001
\def\tableofcontents{\section*{Contents}\@starttoc{toc}}
\def\l@section#1#2{\addpenalty{\@secpenalty} \addvspace{1.0em plus 1pt}
    \@tempdima 2.75em \begingroup \parindent \z@ \rightskip \@pnumwidth
    \parfillskip-\@pnumwidth {\bfseries\leavevmode #1}\hfil\hbox to\@pnumwidth{\hss #2}
    \par \endgroup}
% argument format #1:level, #2:labelindent,#3:labelsep
\def\l@subsection{\@dottedtocline{2}{2.75em}{3.75em}}
\def\l@subsubsection{\@dottedtocline{3}{6.5em}{4.5em}}
% must provide \l@ defs for ALL sublevels EVEN if tocdepth
% is such as they will not appear in the table of contents
% these defs are how TOC knows what level these things are!
\def\l@paragraph{\@dottedtocline{4}{6.5em}{5.5em}}
\def\l@subparagraph{\@dottedtocline{5}{6.5em}{6.5em}}
\def\listoffigures{\section*{List of Figures}\@starttoc{lof}}
\def\l@figure{\@dottedtocline{1}{0em}{2.75em}}
\def\listoftables{\section*{List of Tables}\@starttoc{lot}}
\let\l@table\l@figure

\newdimen\@maxsep
\newdimen\@dblmaxsep
%% Definitions for floats
%%
%% Normal Floats                      %% Double Column Floats
\floatsep 12pt plus 2pt minus 2pt     \dblfloatsep 12pt plus 2pt minus 2pt
\textfloatsep 20pt plus 2pt minus 4pt \dbltextfloatsep 20pt plus 2pt minus 4pt
\@maxsep 20pt                         \@dblmaxsep 20pt
\@fptop 0pt plus 1fil                 \@dblfptop 0pt plus 1fil
\@fpsep 8pt plus 2fil                 \@dblfpsep 8pt plus 2fil
\@fpbot 0pt plus 1fil                 \@dblfpbot 0pt plus 1fil
\def\topfraction{1.0}                 \def\dbltopfraction{1.0}
\def\floatpagefraction{0.8}           \def\dblfloatpagefraction{0.8}
\setcounter{topnumber}{2}             \setcounter{dbltopnumber}{4}
%
\intextsep 12pt plus 2pt minus 2pt
\setcounter{bottomnumber}{2}
\def\bottomfraction{.4}
\setcounter{totalnumber}{4}
\def\textfraction{.2}


%% redefine CAPTION
% V1.4 add user control for short figure caption justification
\newif\ifcenterfigcaptions
\centerfigcaptionsfalse

\long\def\@caption#1[#2]#3{\par\addcontentsline{\csname ext@#1\endcsname}{#1}{%
   \protect\numberline{\csname the#1\endcsname}{\ignorespaces #2}}
   \begingroup \@parboxrestore \normalsize
    \@makecaption{\csname fnum@#1\endcsname}{\ignorespaces #3}{#1}\par
   \endgroup}

\def\tablestring{table}
\def\figurestring{figure}

\newlength{\captionindent}
\long\def\@makecaption#1#2#3{
%\ifx\figurestring#3
% \ifx tests if is a for a figure
    \vskip 5pt
    % 3/2001 use footnotesize, not small; use two nonbreaking spaces, not one
    \setbox\@tempboxa\hbox{\footnotesize #1.~~ #2}
    \ifdim \wd\@tempboxa >\hsize
       % 3/2001 use footnotesize, not small
       \setbox\@tempboxa\hbox{\footnotesize #1.~~ }
       \setlength\captionindent{\wd\@tempboxa} \divide\captionindent by 2
       \captionindent 0em % IEEE does not indent captions MDS 1/2001
       % 3/2001 use footnotesize, not small
       \parbox[t]{\hsize}{\footnotesize \hangindent \captionindent \hangafter=1%
       \unhbox\@tempboxa#2}
      %\else \hbox to\hsize{\small\hfil\box\@tempboxa\hfil}
      % 3/2001 use footnotesize, not small
      % allow user to control short figure caption justification (left or center)
      \else
        \ifcenterfigcaptions \hbox to\hsize{\footnotesize\hfil\box\@tempboxa\hfil}
        \else \hbox to\hsize{\footnotesize\box\@tempboxa\hfil}
        \fi
    \fi
%\else
% or a table
% 3/2001 use \footnotesize, instead of \small for table caption
%    \begin{center}{\small #1}\\{\footnotesize\scshape #2}\end{center}
%    \vskip 8pt
%\fi
}

\newcounter{figure}
\def\thefigure{\@arabic\c@figure}
\def\fps@figure{tbp}
\def\ftype@figure{1}
\def\ext@figure{lof}
\def\fnum@figure{Fig.~\thefigure}
\def\figure{\@float{figure}}
\let\endfigure\end@float
\@namedef{figure*}{\@dblfloat{figure}}
\@namedef{endfigure*}{\end@dblfloat}
\newcounter{table}
\def\thetable{\@Roman\c@table}
\def\fps@table{tbp}
\def\ftype@table{2}
\def\ext@table{lot}
\def\fnum@table{TABLE~\thetable}
\def\table{\@float{table}}
\let\endtable\end@float
\@namedef{table*}{\@dblfloat{table}}
\@namedef{endtable*}{\end@dblfloat}

\if@twoside
 \if@technote
   \def\ps@headings{
       \def\@oddhead{\hbox{}\scriptsize\leftmark \hfil \thepage}
       \def\@evenhead{\scriptsize\thepage \hfil \leftmark\hbox{}}
       \if@draftclsmode
            \def\@oddfoot{\scriptsize\@date\hfil DRAFT}
            \def\@evenfoot{\scriptsize DRAFT\hfil\@date}
       \else
            \def\@oddfoot{}\def\@evenfoot{}
       \fi}
 \else % not a technote
 \def\ps@headings{
       \def\@oddhead{\hbox{}\scriptsize\rightmark \hfil \thepage}
       \def\@evenhead{\scriptsize\thepage \hfil \leftmark\hbox{}}
       \if@draftclsmode
            \def\@oddfoot{\scriptsize\@date\hfil DRAFT}
            \def\@evenfoot{\scriptsize DRAFT\hfil\@date}
       \else
            \def\@oddfoot{}\def\@evenfoot{}
       \fi}
 \fi
\else % single side
\def\ps@headings{
    \def\@oddhead{\hbox{}\scriptsize\leftmark \hfil \thepage}
    \def\@evenhead{}
    \if@draftclsmode
         \def\@oddfoot{\scriptsize \@date \hfil DRAFT}
    \else
         \def\@oddfoot{}
    \fi
    \def\@evenfoot{}}
\fi
\def\ps@titlepagestyle{
    \def\@oddhead{\hbox{}\scriptsize\leftmark \hfil \thepage}
    \def\@evenhead{\scriptsize\thepage \hfil \leftmark\hbox{}}
    \if@draftclsmode
         \def\@oddfoot{\scriptsize \@date\hfil DRAFT}
         \def\@evenfoot{\scriptsize DRAFT\hfil \@date}
    \else
         \def\@oddfoot{}\def\@evenfoot{}
    \fi}
\def\rightmark{}\def\leftmark{}


%% Defines the command for putting the header. footernote{TEXT} is the same
%% as markboth{TEXT}{TEXT}. Here for compatibility with other style files.
\def\markboth#1#2{\def\leftmark{\uppercase{#1}}\def\rightmark{\uppercase{#2}}}
\def\footernote#1{\markboth{#1}{#1}}

\def\today{\ifcase\month\or
    January\or February\or March\or April\or May\or June\or
    July\or August\or September\or October\or November\or December\fi
    \space\number\day, \number\year}

%% BIBLIOGRAPHY
%
%% separate citations with "], ["
\def\@citex[#1]#2{\if@filesw\immediate\write\@auxout{\string\citation{#2}}\fi
\def\@citea{}\@cite{\@for\@citeb:=#2\do {\@citea\def\@citea{],
[}\@ifundefined {b@\@citeb}{{\bfseries ?}\@warning {Citation `\@citeb' on
page \thepage \space undefined}}%
{\csname b@\@citeb\endcsname}}}{#1}}

\def\@cite#1#2{[{#1\if@tempswa, #2\fi}]}

%% Allow short (name-less) citations, when used in conjunction with a
%% bibliography style that creates labels like \citename{<names>, }<year>
\let\@internalcite\cite
\def\cite{\def\citename##1{##1}\@internalcite}
\def\shortcite{\def\citename##1{}\@internalcite}
\def\@biblabel#1{\def\citename##1{##1}[#1]\hfill}

\def\thebibliography#1{\section*{References}%
    \addcontentsline{toc}{section}{References}
    \footnotesize\list
    {[\arabic{enumi}]}{\settowidth\labelwidth{[#1]}\leftmargin\labelwidth
    \advance\leftmargin\labelsep \itemsep 0pt plus .5pt
    \usecounter{enumi}}
    \def\newblock{\hskip .11em plus .33em minus .07em}
% originally:
%   \sloppy\clubpenalty4000\widowpenalty4000%
% by adding the \interlinepenalty here, we make it much more
% difficult, but not impossible, for LaTeX to break within a reference.
% IEEE almost never breaks a reference (but they do it more often with
% technotes). You may get an underfull vbox warning around the bibliography, 
% but the final result will be much more like what IEEE will publish. 
% You can remove the \interlinepenalty if you want references to be split.
% MDS 11/2000
\if@technote\sloppy\clubpenalty4000\widowpenalty4000\interlinepenalty100%
\else\sloppy\clubpenalty4000\widowpenalty4000\interlinepenalty500\fi%
    \sfcode`\.=1000\relax}
\let\endthebibliography=\endlist



\def\maketitle{\par
  \begingroup
  \def\thefootnote{} % the \thanks{} mark type is empty
  \if@twocolumn
     \if@technote
        \newpage\global\@topnum\z@ \@maketitle
     \else\twocolumn[\@maketitle]\fi
  \else\newpage\global\@topnum\z@ \@maketitle \fi
  \thispagestyle{titlepagestyle}\@thanks
  \endgroup
  \setcounter{footnote}{0}\let\maketitle\relax\let\@maketitle\relax
  \gdef\@thanks{}\gdef\@author{}\gdef\@title{}
  \let\thanks\relax
  \if@draftclsmode \vfil\mbox{}\newpage\fi}

\def\@maketitle{\newpage\if@draftclsmode\mbox{}\vfil\fi\begin{center}
    \if@technote
      % V1.4a
      %{\textbf{\large\@title}\par}\vskip 1.3em{\lineskip .5em\@author\@specialpapernotice\par}
      {\textbf{\large\@title}\par}\vskip 1.3em{\lineskip .5em\begin{tabular}[t]{c}\@author\end{tabular}\@specialpapernotice\par}
    \else
      \vskip0.2em{\Huge\@title\par}\vskip1.0em%
      \if@draftclsmode\vfil\fi% 
      %{\lineskip.5em\large\@author\par}
      % V1.4a
      % {\lineskip.5em\large\@author\@specialpapernotice\par}
      {\lineskip.5em\large\begin{tabular}[t]{c}\@author\end{tabular}\@specialpapernotice\par}
    \fi
%    \end{center}\par\vskip 1.4em}
    \end{center}
    \if@technote
    \par\normalfont\normalsize\vskip 1\baselineskip plus 2ex minus 1ex
    \else
    \par\normalfont\normalsize\vskip 2.5\baselineskip plus 2ex minus 1ex
    \fi}

\def\abstract{%
    \if@twocolumn
     % IEEE puts heading in bold
     % \small\textit{Abstract}\bfseries---$\!$%
      \small\bfseries\textit{Abstract}---$\!$%
    \else
      \begin{center}\vspace{-0.8em}\small\textbf{Abstract}\end{center}\quotation\small
    \fi}
\def\endabstract{\vspace{0.6em}\par\if@twocolumn\else\endquotation\fi
    \normalfont\normalsize}

\def\keywords{%\vspace{-.3em}
    % IEEE uses the term (in bold italics) "Index Terms" now. 
    \if@twocolumn
      % \small\textit{Keywords}\bfseries---$\!$%
      \small\bfseries\textit{Index Terms}---$\!$%
    \else
      % \begin{center}\small\bfseries Keywords\end{center}\quotation\small
      \begin{center}\small\bfseries Index Terms\end{center}\quotation\small
    \fi}
\def\endkeywords{\if@technote\vspace{0.6em}\else\vspace{0.3em}\fi
    \par\if@twocolumn\else\endquotation\fi
    \normalsize\normalfont}


% titling of sections
\gdef\@punct{:\ \,}  % Punctuation after run-in section heading  (headings which are
                     % part of the paragraphs), need little bit more than a space
%\gdef\@punct{.\ \ } % old way, they needed two spaces because one got eaten,
                     % but puts too much space in the old nonstar section form.
\def\@sect#1#2#3#4#5#6[#7]#8{%
  \ifnum #2>\c@secnumdepth
     \def\@svsec{}
  \else
     \refstepcounter{#1}\edef\@svsec{%
     % load section label into \@svsec
     % A. section title
     \ifnum #2=1{\thesectiondis}\fi\ifnum #2=2{\thesubsectiondis}\fi
     \ifnum #2>2{\csname the#1dis\endcsname}\fi \hskip .5em}
  \fi
  \@tempskipa #5\relax
  \ifdim \@tempskipa>\z@   % tempskipa determines whether is treated as a high
     \begingroup #6\relax  % or low level heading
      \noindent % subsections are NOT indented
       % print top level headings. \@svsec is label, #8 is heading title
       %\@hangfrom{\hskip #3\relax\@svsec}{\interlinepenalty \@M #8\par}
       % IEEE does not block indent the section title text, it flows like normal
       \relax{\hskip #3\relax\@svsec}{\interlinepenalty \@M #8\par}
     \endgroup
     % got rid of sectionmark stuff
     % \csname #1mark\endcsname{#7}
     \addcontentsline{toc}{#1}{\ifnum #2>\c@secnumdepth\else
               \protect\numberline{\csname the#1\endcsname}\fi#7}
  \else % printout low level headings
     % svsechd seems to swallow the trailing space, protect it with \mbox{}
     % got rid of sectionmark stuff
     \def\@svsechd{#6\hskip #3\@svsec #8\@punct\mbox{} %\csname #1mark\endcsname{#7}
     \addcontentsline{toc}{#1}{\ifnum #2>\c@secnumdepth \else
               \protect\numberline{\csname the#1\endcsname}\fi#7}}
  \fi %skip down
  \@xsect{#5}}
  
% section* handler
\def\@ssect#1#2#3#4#5{\@tempskipa #3\relax
  \ifdim \@tempskipa>\z@
     %\begingroup #4\@hangfrom{\hskip #1}{\interlinepenalty \@M #5\par}\endgroup
     % IEEE does not block indent the section title text, it flows like normal
     \begingroup \noindent #4\relax{\hskip #1}{\interlinepenalty \@M #5\par}\endgroup
  % svsechd seems to swallow the trailing space, protect it with \mbox{}
  \else \def\@svsechd{#4\hskip #1\relax #5\@punct\mbox{}}\fi
  \@xsect{#3}}


%% SECTION heading spacing and font
%%
% arguments are: #1 - sectiontype name
% (for \@sect)   #2 - section level
%                #3 - indent
%                #4 - top separation
%                #5 - tempskipa, bottom separation
%                #6 - font control
%\def\section{\@startsection{section}{1}{\z@}{2.8ex plus 4ex
% IEEE sometimes REALLY stretches the area before a section
% heading by up to about 0.5in. However, it may not be a good
% idea to let LaTeX have quite this much rubber.
% You've got to have \normalfont\normalsize in the font spec to prevent
% trouble when you do something like:
% \section{Note}{\ttfamily TT-TEXT} is known to ... 
%\def\section{\@startsection{section}{1}{\z@}{2.8ex plus 2ex minus 1.4ex}%
\def\section{\@startsection{section}{1}{\z@}{3.5ex plus 1.5ex minus 1.5ex}%
{0.7ex plus 1ex minus 0ex}{\normalfont\normalsize\centering\scshape}}
%\def\subsection{\@startsection{subsection}{2}{\z@}{2.5ex plus 1.5ex minus 1ex}%
\def\subsection{\@startsection{subsection}{2}{\z@}{3.5ex plus 1.5ex minus 1.5ex}%
{0.7ex plus .5ex minus 0ex}{\normalfont\normalsize\itshape}}
% decided to put in a little rubber above the section, might help somebody
\def\subsubsection{\@startsection{subsubsection}{3}{\parindent}{0ex plus 0.1ex minus 0.1ex}%
{0ex}{\normalfont\normalsize\itshape}}
\def\paragraph{\@startsection{paragraph}{4}{2\parindent}{0ex plus 0.1ex minus 0.1ex}%
{0ex}{\normalfont\normalsize\itshape}}


%% ENVIRONMENTS
%\itemindent is set to \z@ by list, so define new temporary variable
\newdimen\tmpitemindent

%\def\QED{\mbox{\rule[0pt]{1.5ex}{1.5ex}}}
%\def\proof{\noindent\hspace{2em}{\itshape Proof: }}
%\def\endproof{\hspace*{\fill}~\QED\par\endtrivlist\unskip}
%\def\@begintheorem#1#2{\tmpitemindent\itemindent\topsep 0pt\rmfamily\trivlist
%    \item[\hskip \labelsep{\indent\itshape #1\ #2:}]\itemindent\tmpitemindent}
%\def\@opargbegintheorem#1#2#3{\tmpitemindent\itemindent\topsep 0pt\rmfamily \trivlist


% IEEE uses a long dash between the theorem number and name and does not
% put () around the name
%    \item[\hskip\labelsep{\indent\itshape #1\ #2\ \rmfamily(#3)}]\itemindent\tmpitemindent}
%    \item[\hskip\labelsep{\indent\itshape #1\ #2\rmfamily---#3:}]\itemindent\tmpitemindent}
%\def\@endtheorem{\endtrivlist\unskip}

%% sets up default PAGESTYLE
\ps@headings
\pagenumbering{arabic}

\if@twoside\else\raggedbottom\fi
\if@tmptwocolumn \twocolumn \sloppy \flushbottom \fi



% \APPENDIX and \APPENDICES definitions

% This is the \@ifmtarg command from the LaTeX ifmtarg package
% by Peter Wilson (CUA) and Donald Arseneau
% \@ifmtarg is used to determine if an argument to a command
% is present or not.
% For instance:
% \@ifmtarg{#1}{\typeout{empty}}{\typeout{has something}}
% \@ifmtarg is used with our redefined \section command if
% \appendices is invoked.
% The command \section will behave slightly differently depending
% on whether the user specifies a title: 
% \section{My appendix title}
% or not:
% \section{}
% This way, we can eliminate the blank lines where the title
% would be, and the unneeded : after Appendix in the table of
% contents 
\begingroup
\catcode`\Q=3
\long\gdef\@ifmtarg#1{\@xifmtarg#1QQ\@secondoftwo\@firstoftwo\@nil}
\long\gdef\@xifmtarg#1#2Q#3#4#5\@nil{#4}
\endgroup
% end of \@ifmtarg defs

% save the "original" meaning of \section so we can redefine
% \section after a call to \appendix or \appendices 
\let\@ppendixsavesection\section

% neat trick to grab and process the argument from \section{argument}
% we process differently if the user invoked \section{} with no
% argument (title)
% note we reroute the call to the old \section*
\def\@processthesectionargument#1{%
\@ifmtarg{#1}{
\@ppendixsavesection*{Appendix \thesectiondis}
\addcontentsline{toc}{section}{Appendix \thesection}}{%
\@ppendixsavesection*{Appendix \thesectiondis \\* #1}
\addcontentsline{toc}{section}{Appendix \thesection: #1}}}

% we use this if the user calls \section{} after
% \appendix-- which has no meaning. So, we ignore the
% command and its argument. Then, warn the user.
\def\@destroythesectionargument#1{\typeout{WARNING: Ignoring useless
\protect\section {\noexpand#1} in Appendix.}}


% remember \thesection forms will be displayed in \ref calls
% and in the Table of Contents.
% The \sectiondis form is used in the actual heading itself

% appendix command for one single appendix
% normally has no heading. However, if you want a 
% heading, you can do so via the optional argument:
% \appendix[Optional Heading]
\def\appendix{\relax}
\renewcommand{\appendix}[1][]{\par
    \setcounter{section}{0}
    \setcounter{subsection}{0}
    \setcounter{subsubsection}{0}
    \setcounter{paragraph}{0}
    \def\thesection{}
    \def\thesectiondis{}  
    \def\thesubsection{\Alph{subsection}}
    \refstepcounter{section} % update the \ref counter
    \@ifmtarg{#1}{\@ppendixsavesection*{Appendix}
                  \addcontentsline{toc}{section}{Appendix}}{%
             \@ppendixsavesection*{Appendix \\* #1}
             \addcontentsline{toc}{section}{Appendix: #1}}
    % redefine \section command for appendix
    % leave \section* as is
    \def\section{\@ifstar{\@ppendixsavesection*}{%
                    \@destroythesectionargument}}% throw out the argument
                                                 % of the normal form
}



% provides the user a way to choose between
% Appendix A
%    and
% Appendix I
% notation
% defaults to Roman.
\newif\ifuseRomanappendices
\useRomanappendicestrue


% appendices command for multiple appendices
% user then calls \section with an argument (possibly empty) to
% declare the individual appendices
\def\appendices{\par
    \setcounter{section}{-1} % we want \refstepcounter to use section 0
    \setcounter{subsection}{0}
    \setcounter{subsubsection}{0}
    \setcounter{paragraph}{0}
    \def\thesection{\Alph{section}}
    \def\thesectiondis{\Alph{section}}
    \refstepcounter{section} % update the \ref counter
    \setcounter{section}{0} % NEXT \section will be the FIRST appendix
    % redefine \section command for appendices
    % leave \section* as is
    \def\section{\@ifstar{\@ppendixsavesection*}{% process the *-form
                    \refstepcounter{section} % or is a new section so,
                    \@processthesectionargument}}% process the argument 
                                                 % of the normal form
}



%% 
%% Definition for Big letter at the beginning of a paragraph

% for folks that have trouble with Times fonts, here is the old PARstart
% that uses the Computer Modern fonts.
\def\CMPARstart#1#2{\begingroup\def\par{\endgraf\endgroup\lineskiplimit=0pt}
    \setbox2=\hbox{\uppercase{#2} }\newdimen\tmpht \tmpht \ht2
    \advance\tmpht by \baselineskip\font\hhuge=cmr10 at \tmpht
    \setbox1=\hbox{{\hhuge #1}}
    \count7=\tmpht \count8=\ht1\divide\count8 by 1000 \divide\count7 by\count8
%   \tmpht=.001\tmpht\multiply\tmpht by \count7\font\hhuge=cmr10 at \tmpht
%   first letter of parstart is BOLD, use cmbx10
    \tmpht=.001\tmpht\multiply\tmpht by \count7\font\hhuge=cmbx10 at \tmpht
    \setbox1=\hbox{{\hhuge #1}} \noindent \hangindent1.05\wd1
    \hangafter=-2 {\hskip-\hangindent \lower1\ht1\hbox{\raise1.0\ht2\copy1}%
    \kern-0\wd1}\copy2\lineskiplimit=-1000pt}

% The new default PARstart uses Times font
\def\PARstart#1#2{\begingroup\def\par{\endgraf\endgroup\lineskiplimit=0pt}
    \setbox2=\hbox{\uppercase{#2} }\newdimen\tmpht \tmpht \ht2
%   by making it 1.1\baselineskip, we get a first letter that drops a
%   little below the baseline. The result is extremely close to what
%   IEEE uses.
    \advance\tmpht by 1.1\baselineskip\font\hhuge=ptmr at \tmpht
    \setbox1=\hbox{{\hhuge #1}}
    \count7=\tmpht \count8=\ht1\divide\count8 by 1000 \divide\count7 by\count8
    \tmpht=.001\tmpht\multiply\tmpht by \count7\font\hhuge=ptmb at \tmpht
    \setbox1=\hbox{{\hhuge #1}} \noindent \hangindent1.05\wd1
    \hangafter=-2 {\hskip-\hangindent \lower1\ht1\hbox{\raise1.0\ht2\copy1}%
    \kern-0\wd1}\copy2\lineskiplimit=-1000pt}

\if@technote\def\PARstart#1#2{#1#2}\def\CMPARstart#1#2{#1#2}\fi     % if technical note, disable it
\if@draftclsmode\def\PARstart#1#2{#1#2}\def\CMPARstart#1#2{#1#2}\fi % if draft, disable it

%% Allows user to enter BIOGRAPHY leaving place for picture (adapts to font size)
%% As of V1.5, a new optional argument allows you to have a real graphic!
%% V1.5 and later also fixes the "colliding biographies" which could happen when a 
%% biography's text was shorter than the space for the photo.
%% MDS 7/2001
\newenvironment{biography}[2][]{%
\addcontentsline{toc}{section}{Biography}
\footnotesize\unitlength 1in\bigskip\parskip=0pt\par%
\rule{0pt}{1.54in}\vspace{-1.54in}\par%   guarantees correct page breaking
\noindent
% the default box for where the photo goes
\def\@tempbiographybox{\hbox{\framebox(1,1.25){\parbox[][\height][c]{0.9in}{\centering PLACE\\ PHOTO\\ HERE}}}}%
% detect if the optional argument was supplied, this requires the
% \@ifmtarg command as defined in the appendix section above
\@ifmtarg{#1}{\relax}{%
%   % The user supplied the optional argument
%   % override the default box with what they want
\def\@tempbiographybox{\hbox{%
\begin{minipage}[b][1.25in][c]{1in}%
\centering
#1%
\end{minipage}
}}%
}% end if optional argument supplied
\setbox0\@tempbiographybox%
\ht0=1.45in\count10=\ht0\divide\count10 by\baselineskip%  calculates lines
\global\hangindent1.14in\global\hangafter-\count10%
\hskip-1.12in\setbox0\hbox to 1.12in {\raise-1.2in\box0\hss}%
\dp0=0in\ht0=0in\box0\noindent{\bfseries#2}\rmfamily}{\par%
% 7/2001 V1.5 detect when the biography text is shorter than the photo area
% and pad the unused area - preventing a collision from the next biography entry
% MDS
\ifnum \prevgraf <\count10 % detect when the biography text is shorter than the photo
    \count11=\count10
    \advance\count11 by -\prevgraf % calculate how many lines we need to pad
    \dimen10=\baselineskip % calculate the length of the padding
    \multiply\dimen10 by \count11
    \noindent\rule{0pt}{\dimen10} % insert an invisible support strut
\fi
\par\normalfont}


% provide the user with some old font commands
% got this from article.cls
\DeclareOldFontCommand{\rm}{\normalfont\rmfamily}{\mathrm}
\DeclareOldFontCommand{\sf}{\normalfont\sffamily}{\mathsf}
\DeclareOldFontCommand{\tt}{\normalfont\ttfamily}{\mathtt}
\DeclareOldFontCommand{\bf}{\normalfont\bfseries}{\mathbf}
\DeclareOldFontCommand{\it}{\normalfont\itshape}{\mathit}
\DeclareOldFontCommand{\sl}{\normalfont\slshape}{\@nomath\sl}
\DeclareOldFontCommand{\sc}{\normalfont\scshape}{\@nomath\sc}
\DeclareRobustCommand*\cal{\@fontswitch\relax\mathcal}
\DeclareRobustCommand*\mit{\@fontswitch\relax\mathnormal}


% SPECIAL PAPER NOTICE COMMANDS
% 
% holds the special notice text
\def\@specialpapernotice{\relax}
 
% for special papers, like invited papers, the user can do:
% \specialpapernotice{(Invited Paper)} before \maketitle
\def\specialpapernotice#1{\def\@specialpapernotice{%
\\*[1.5ex]\textit{#1}\vspace*{-2ex}}}



% PUBLISHER ID COMMANDS
% save the normalsize baselineskip
% so that we can reduce the textheight by exactly two normal
% lines when we put in the publisher's ID footer
\let\@normalbaselineskipsave\baselineskip

% holds the ID text
\def\@pubid{\relax}

% issue this command in the page to have the ID at the bottom
\def\pubid#1{\def\@pubid{#1} \thispagestyle{pubidtitlepagestyle}
\enlargethispage{-1.3\@normalbaselineskipsave}}

% command which will pull up (shorten) the column it is executed in
% to make room for the publisher ID
% Is smart enough not to do anything when in single column text.
% currently needed in for the second column of a page with the
% publisher ID. If not needed in future releases, please provide this
% command and define it as \relax for backward compatibility 
\def\pubidadjcol{\if@twocolumn\enlargethispage{-1.3\@normalbaselineskipsave}\fi}


% new pagestyle for pages that have a publisher's ID
% we bring the footer ID text up into the last line of the
% main text
% publisher's ID will not be printed when in DRAFT mode.
\def\ps@pubidtitlepagestyle{
    \def\@oddhead{\hbox{}\scriptsize\leftmark \hfil \thepage}
    \def\@evenhead{\scriptsize\thepage \hfil \leftmark\hbox{}}
    \if@draftclsmode
         \def\@oddfoot{\scriptsize \@date\hfil DRAFT}
         \def\@evenfoot{\scriptsize DRAFT\hfil \@date}
    \else
         \footskip 0pt
         \def\@oddfoot{\hss\normalfont\footnotesize\raisebox{1.5ex}[1.5ex]{\@pubid}\hss}
         \def\@evenfoot{\hss\normalfont\footnotesize\raisebox{1.5ex}[1.5ex]{\@pubid}\hss}
    \fi}


\endinput

%%%%%%%%%%%%%%%%%%%%%%%%%%%%% End of IEEEtran.cls  %%%%%%%%%%%%%%%%%%%%%%%%%%%%


