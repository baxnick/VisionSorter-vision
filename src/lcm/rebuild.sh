#!/bin/bash
rm -R ../lcmtypes
mkdir ../lcmtypes
lcm-gen --c --java *.lcm
mv *.c *.h ../lcmtypes
rm lcmtypes.jar
javac -cp /usr/local/share/java/lcm.jar lcmtypes/*.java
jar -cf lcmtypes.jar lcmtypes/
