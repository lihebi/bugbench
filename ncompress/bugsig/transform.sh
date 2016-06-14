#!/bin/bash

# sed -i bak -E -e "s/REG[0-9]{1,2}//g" compress42.c # remove all REG2 kinds of macros, save back up file with extension .bak


# remove all REG2 kinds of macros
sed -E -e "460,$ s/REG[0-9]{1,2}//g" compress42.c.orig > compress42.bugsig.c

# add @HeliumStmt markup before the line
# CAUTION I hard coded the line number!
sed -E -e "892 i\\
//@HeliumStmt" compress42.bugsig.c -i # | head -893 | tail -5

# remove a conditional compilation
sed -E -e "1268,1270 s/^(.*)$/\/\/ \1/" compress42.bugsig.c -i
sed -E -e "1272 s/^(.*)$/\/\/ \1/" compress42.bugsig.c -i
# remove a callsite of comprexx
sed -E -e "1311 s/^(.*)$/\/\/ \1/" compress42.bugsig.c -i
