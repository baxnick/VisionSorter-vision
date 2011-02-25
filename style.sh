#!/bin/bash
astyle --style=ansi -s3 -n \
   `find src \( -name "*.h" -or -name "*.cpp" \) -not -path "*lcmtypes*"`
