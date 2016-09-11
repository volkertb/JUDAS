rem rebuild judas library
del *.lib
wmake /f makefile.wat
del *.obj

rem rebuild example files
wmake /f jp.wat
wmake /f anal.wat
del *.obj
