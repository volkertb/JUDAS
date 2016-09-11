nasm dos32\input.asm -g -fobj -Einput.err
nasm dos32\timer.asm -g -fobj -Etimer.err
nasm dos32\dpmi.asm -g -fobj -Edpmi.err
wcl386 /d2 /l=dos4g /bt=dos ichinit.c pci.c codec.c dos32\dpmi.obj dos32\timer.obj dos32\input.obj
del ichinit.obj
rem stubit ichinit.exe
del *.err
