@echo off
set blaster=
rem set DJGPP=E:\DJGPP\DJGPP.ENV
rem set PATH=E:\DJGPP\BIN;%PATH%
make.exe -f makefile.dj
make.exe -f jp.dj
make.exe -f anal.dj
strip -s anal.exe
strip -s jp.exe
upx --best anal.exe
upx --best jp.exe
