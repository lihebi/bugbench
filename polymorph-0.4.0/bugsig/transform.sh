#!/bin/bash

echo "OSTYPE: " $OSTYPE
echo "Transforming ..."
SED=sed
if [[ "$OSTYPE" == "linux-gnu" ]]; then
    SED=sed
elif [[ "$OSTYPE" == "darwin"* ]]; then
    SED=gsed
fi

cp ../src/polymorph.c .

# add @HeliumStmt markup before the line
# CAUTION I hard coded the line number!
$SED -E -e "120 i\\
//@HeliumStmt" polymorph.c -i # | head -893 | tail -5

echo "Done"
