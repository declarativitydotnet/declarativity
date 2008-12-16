#!/bin/bash

dir=`dirname $OVERLOG`/../../lang/olg

echo Erasing the dataflow files in $dir
rm $dir/*.df
