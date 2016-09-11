# JUDAS PLAYER makefile

jp.exe: jp.obj timer.obj timerasm.obj
#        wlink F jp,timer,timerasm N jp.exe L judas.lib SYS dos4g
        wcl386 -d2 -ldos4g jp.c timer.c timerasm.obj judas.lib

jp.obj: jp.c
        wcc386 -d2 -w3 -zp4 jp.c
timer.obj: timer.c
        wcc386 -d2 -w3 -zp4 timer.c
timerasm.obj: timerasm.asm
        nasm -dwatcom -fobj timerasm.asm
