#!/bin/tcsh
set file = $1
set figures = $2
if ($# > 2) then
    set xmargin = $3
else
    set xmargin = 1
endif
if ($# > 3 ) then
    set ymargin = $4
else
    set ymargin = 1
endif
set name = `basename $file .prn`
set oldbox = `grep "%%BoundingBox" $file`
set size = `grep -n $name $figures| cut -d ' ' -f2,3`
set oldpagebox = `grep "%%PageBoundingBox" $file | head -1`

set boundingCommand = \$width"="\$ARGV\[0\]\;\$height"="\$ARGV\[1\]\;\$xmargin"="${xmargin}\;\$ymargin"="${ymargin}\;\$pageheight"="11\;\$LLX"="\$xmargin"*"72\;\$LLY"="\(\$pageheight\-\$ymargin\-\$height\)"*"72\;\$ULX"="\(\$xmargin"+"\$width\)"*"72\;\$ULY"="\(\$pageheight\-\$ymargin\)"*"72\;print\"\$LLX\ \$LLY\ \$ULX\ \$ULY\"

set bounding = "/usr/bin/perl -e '$boundingCommand'"
set newpagebox = "%%PageBoundingBox: `$bounding $size`"
set newbox = "%%BoundingBox: `$bounding $size`"

if (`grep -c PageBoundingBox $file` > 0) then
    sed -e "s/$oldbox/$newbox/g" $file | sed -e "s/$oldpagebox/$newpagebox/g" - > $name.eps
else
    sed -e "s/$oldbox/$newbox/g" $file > $name.eps
endif
