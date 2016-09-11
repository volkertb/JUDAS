# JUDAS library makefile
#
# NOTE: modules which are or might be called from within an interrupt have the
# -zu option, to tell that SS != DS. It'll crash otherwise.
#
# note : nasm compiles for obj output :
# nasm judasasm.asm -dwatcom -fobj -E!nasm.err
# rem nasm judasasm.asm -ddjgpp -fcoff -E!nasm.err
#

judas.lib: judas.obj judassmp.obj judasraw.obj judaswav.obj judastbl.obj judasxm.obj judasmod.obj judass3m.obj judasio.obj judasasm.obj judasdma.obj judasmem.obj
        wlib -n -c judas.lib @judas.cmd
        
judas.obj: judas.c
        wcl386 -c -d2 -w3 -zp4 judas.c
judassmp.obj: judassmp.c
        wcl386 -c -d2 -w3 -zp4 judassmp.c
judasraw.obj: judasraw.c
        wcl386 -c -d2 -w3 -zp4 judasraw.c
judaswav.obj: judaswav.c
        wcl386 -c -d2 -w3 -zp4 judaswav.c
judasio.obj: judasio.c
        wcl386 -c -d2 -w3 -zp4 judasio.c
judasmem.obj: judasmem.c
        wcl386 -c -d2 -w3 -zp4 judasmem.c
judastbl.obj: judastbl.c
        wcl386 -c -d2 -w3 -zp4 judastbl.c
judasxm.obj: judasxm.c
        wcl386 -c -d2 -zu -w3 -zp4 judasxm.c
judasmod.obj: judasmod.c
        wcl386 -c -d2 -zu -w3 -zp4 judasmod.c
judass3m.obj: judass3m.c
        wcl386 -c -d2 -zu -w3 -zp4 judass3m.c
judasdma.obj: judasdma.c
        wcl386 -c -d2 -zu -w3 -zp4 judasdma.c
judasasm.obj: judasasm.asm
        nasm -dwatcom -fobj judasasm.asm

