#!/bin/bash

for f in `find src \( -name "*.h" -or -name "*.cpp" \) -not -path "*lcmtypes*"`
do
   echo "`cat LICENSE $f`" > $f
done
