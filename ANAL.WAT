# ANAL INVADERS makefile

anal.exe: anal.obj timer.obj timerasm.obj kbd.obj kbdasm.obj
#        wlink F anal,timer,timerasm,kbd,kbdasm N anal.exe L judas.lib SYS dos4g
        wcl386 -d2 -ldos4g anal.c timer.c timerasm.obj kbd.c kbdasm.obj judas.lib
        
anal.obj: anal.c
        wcc386 -d2 -w3 -zp4 anal.c
timer.obj: timer.c
        wcc386 -d2 -w3 -zp4 timer.c
kbd.obj: kbd.c
        wcc386 -d2 -w3 -zp4 kbd.c
timerasm.obj: timerasm.asm
        nasm -dwatcom -fobj timerasm.asm
kbdasm.obj: kbdasm.asm
        nasm -dwatcom -fobj kbdasm.asm
