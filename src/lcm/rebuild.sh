#!/bin/bash
lcm-gen --c --java *.lcm
mv *.c *.h ../
rm lcmtypes.jar
jar -cf lcmtypes.jar lcmtypes/
