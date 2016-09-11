Build notes by RayeR
********************

This source package was accomodated to use with latest GCC 4.6.x, DJGPP 2.04
Due to GCC optimalization issues since ver. 4.0.1 I modified global
optimization flag from -O3 to -Os (-O1 and higher produced crashing code).
It would be better to isolate problem function or file and modify the
optimization locally.
To build the package just run !DJGPP.BAT under plain DOS or Windows DOS box.
When using intel HDA be sure to not have set BLASTER environment variable
otherwise it will try initialize SB first.