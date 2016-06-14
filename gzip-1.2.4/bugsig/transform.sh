#!/bin/bash

# sed -i bak -E -e "s/REG[0-9]{1,2}//g" compress42.c # remove all REG2 kinds of macros, save back up file with extension .bak

echo "OSTYPE: " $OSTYPE
echo "Transforming ..."
SED=sed
if [[ "$OSTYPE" == "linux-gnu" ]]; then
    SED=sed
elif [[ "$OSTYPE" == "darwin"* ]]; then
    SED=gsed
fi


cp ../src/gzip.c .

# add @HeliumStmt markup before the line
# CAUTION I hard coded the line number!
$SED -E -e "828 i\\
//@HeliumStmt" gzip.c -i # | head -893 | tail -5

# remove a conditional compilation
$SED -E -e "707 s/^(.*)$/\/\/ \1/" gzip.c -i
$SED -E -e "709,711 s/^(.*)$/\/\/ \1/" gzip.c -i

cp ../src/trees.c .

# remove a conditional compilation
$SED -E -e "896 s/^(.*)$/\/\/ \1/" trees.c -i
$SED -E -e "898,900 s/^(.*)$/\/\/ \1/" trees.c -i

rm -rf getopt.c
rm -rf getopt.h
# # remove all REG2 kinds of macros
# $SED -E -e "460,$ s/REG[0-9]{1,2}//g" compress42.c.orig > compress42.bugsig.c


# # remove a callsite of comprexx
# $SED -E -e "1311 s/^(.*)$/\/\/ \1/" compress42.bugsig.c -i

echo "Done"
