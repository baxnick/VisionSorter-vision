#!/bin/bash
lcm-gen --c --java *.lcm
mv *.c *.h ../
rm lcmtypes.jar
javac -cp /usr/local/share/java/lcm.jar lcmtypes/*.java
jar -cf lcmtypes.jar lcmtypes/
