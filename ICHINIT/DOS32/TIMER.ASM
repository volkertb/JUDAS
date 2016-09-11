
;******************************************************************************
;*
;* TIMER.ASM module for ULTIMATE_FLIP library
;* by Piotr Ulaszewski <BlackSpider> - basing on timer examples from USMP
;* piotrkn22@poczta.onet.pl
;* http://www.republika.pl/piotrkn22/
;* Multiple timers interface (defullt 16).
;* The structure and behaviour of the multiple timers was based on the timer
;* interface of the USMP 1.04 music player, although DPMI lock was added for
;* both, the main INT 8 handler and for all the user's procedures, direct
;* INT 31h functions were used, the timer structure was simplified and
;* some code parts have been rearanged (mainly StartTimer and the interrupt
;* routine). However most credits should go to Freddy Vételé (FreddyV/Useless)
;* for releasing his source code, so that others can learn from it.
;* 
;* This is an extended version for C/C++ support.
;* developemnent version 0.35
;*
;******************************************************************************

MaxTimers equ 16                          ; this value can be increased
TimerSpeed equ 1193181                    ; PIC Frequency
TimerStrucSize equ 16

struc Timer                               ; structure size = 16 bytes
        tspeed resd 1                     ; speed of this timer
        tcount resd 1                     ; ticks of this timer
        tproc resd 1                      ; procedure to call for this timer
        tactive resb 1                    ; 1=timer on   0=timer off
        treserved resb 3                  ; reserved to stay aligned
endstruc


%include "segments.inc"

global _flip2_StartTimer                  ; proc
global _flip2_StopTimer                   ; proc
global _flip2_TimerErrorcode              ; dword

segment _DATA
_flip2_TimerErrorcode dd 0                ; error code for timer functions


;******************************************************************************
;*
;* StartTimer: Starts a new timer procedure.
;*             User sub timers can be slower than 18.2Hz and must terminate
;*             with a RET instruction instead of an IRETD !!!
;*             Other interrupts will be allowed while executing
;*             a user timer procedure (so the CLI instruction may be used) !!!
;*             User timer procedures should be smaller than 4KB in virtual
;*             environments such as Windows because only the first 4KB
;*             of a timer procedure will be locked !!!
;*             Warning - this will only work with 4GB flat mode zero based
;*             segments where the base and limit of CS is equal to those of DS
;*
;*
;*        In:  push TimerSpeed (1193181/freq)
;*             push TimerProcedure
;*             call StartTimer
;*
;*        Out: eax = TRUE(1) on success and FALSE(0) on error
;*                   _flip2_TimerErrorcode holds execution status :
;*                         00 = success (CF clear)
;*                         01 = could not lock linear region
;*                         02 = set protected mode interrupt error
;*                         03 = no more timers available
;*                         04 = not used
;*
;******************************************************************************
segment _TEXT
_flip2_StartTimer:
        push ebp
        mov ebp,esp
        push ebx
        push ecx
        push edx
        push esi
        push edi
        cli

        mov ecx,MaxTimers
        mov esi,timersdata              ; offset to timers data
.timer_add_loop:
        cmp byte [esi+tactive],0
        je .timer_found
        add esi,TimerStrucSize
        dec ecx
        jnz .timer_add_loop
        sub eax,eax
        mov dword [_flip2_TimerErrorcode],3  ; 3 - no free timer available
        stc
        jmp .exit

.timer_found:
        push esi
        mov eax,600h                    ; lock linear region (for user timer)
        mov ebx,[ebp+8]                 ; offset to new timer handler
        mov cx,bx
        shr ebx,16                      ; BX:CX = address
        sub esi,esi
        mov edi,4096                    ; assume 4096 bytes to lock
        int 31h
        jnc .user_timer_lock_ok
        pop esi
        sub eax,eax
        mov dword [_flip2_TimerErrorcode],1  ; 1 - colud not lock linear region
        stc
        jmp .exit

.user_timer_lock_ok:
        pop esi
        mov eax,[ebp+12]                ; timer speed
        mov [esi+tspeed],eax            ; set new timer speed
        mov dword [esi+tcount],0
        mov eax,[ebp+8]                 ; timer procedure
        mov [esi+tproc],eax             ; set new timer procedure
        mov byte [esi+tactive],1        ; activate the new timer
        cmp dword [timerspeed],0        ; global timer speed = 18.2Hz ???
        jne near .do_not_hook_timer
        mov word [flat_ds_selector],ds

; get old and set the new IRQ 0 (timer) main procedure
        mov eax,0204h                   ; get PM interrupt handler
        mov bl,8                        ; timer interrupt
        int 31h
        mov [oldtimeroffset],edx
        mov [oldtimerselector],cx       ; CX:EDX = old PM handler

        mov eax,600h                    ; lock linear region (global handler)
        mov ebx,new_timer_procedure     ; offset to new timer handler
        mov cx,bx
        shr ebx,16                      ; BX:CX = address
        sub esi,esi
        mov edi,new_timer_procedure_end - new_timer_procedure
        int 31h
        jnc .global_timer_lock_ok
        push es                         ; clear all timers data
        push ds
        pop es
        cld
        mov edi,timersdata              ; offset to timers data
        mov ecx,MaxTimers*TimerStrucSize
        sub al,al
        rep stosb
        pop es
        sub eax,eax
        mov dword [_flip2_TimerErrorcode],1  ; 1 - colud not lock linear region
        stc
        jmp short .exit

.global_timer_lock_ok:
        mov eax,0205h                   ; set PM interrupt handler
        mov bl,8
        mov cx,cs
        mov edx,new_timer_procedure     ; CX:EDX = new PM handler
        int 31h
        jnc .do_not_hook_timer          ; jump if set PM IRQ ok

        mov eax,601h                    ; must unlock linear region on error
        mov ebx,new_timer_procedure     ; offset to PM handler
        mov cx,bx
        shr ebx,16                      ; BX:CX = address
        sub esi,esi
        mov edi,new_timer_procedure_end - new_timer_procedure
        int 31h

        sub eax,eax
        mov dword [_flip2_TimerErrorcode],2  ; 2 - error in set PM interrupt call
        stc
        jmp short .exit

.do_not_hook_timer:
        call GetMaxSpeed                ; out eax=speed of the fastest timer
        call SetTimerSpeed              ; in  eax=new timer speed value
        mov eax,1
        mov dword [_flip2_TimerErrorcode],0  ; 0 - success
        clc
.exit:
        sti
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop ebp
        ret



;******************************************************************************
;*
;* StopTimer:  Stops a timer procedure.
;*
;*        In:  push TimerProcedure
;*             call StartTimer
;*
;*        Out: eax = TRUE(1) on success and FALSE(0) on error
;*                   _flip2_TimerErrorcode holds execution status :
;*                         00 = success (CF clear)
;*                         01 = not used
;*                         02 = not used
;*                         03 = not used
;*                         04 = timer not found
;*
;******************************************************************************
_flip2_StopTimer:
        push ebp
        mov ebp,esp
        push ebx
        push ecx
        push edx
        push esi
        push edi
        cli

        mov ecx,MaxTimers
        mov esi,timersdata              ; offset to timers data
.timer_stop_loop:
        cmp byte [esi+tactive],0
        je .timer_not_active
        mov eax,[ebp+8]                 ; timer procedure to stop offset
        cmp [esi+tproc],eax
        je .timer_found
.timer_not_active:
        add esi,TimerStrucSize
        dec ecx
        jnz .timer_stop_loop
        sub eax,eax
        mov dword [_flip2_TimerErrorcode],4  ; 4 - timer not found error
        stc
        jmp .exit

.timer_found:
        mov byte [esi+tactive],0        ; stop timer
        mov eax,601h                    ; unlock his linear region
        mov ebx,[esi+tproc]             ; offset to this timer handler
        mov cx,bx
        shr ebx,16                      ; BX:CX = address
        sub esi,esi
        mov edi,4096                    ; assume 4096 bytes to unlock
        int 31h
        call GetMaxSpeed                ; out eax = speed of the fastest timer
        call SetTimerSpeed              ; in  eax = new global timer value
                                        ; set speed to the fastest timer speed
        mov ecx,MaxTimers
        mov esi,timersdata              ; offset to timers data
.timer_stop_loop_2:
        cmp byte [esi+tactive],1
        je .timer_stop_end              ; is timer is active?
        add esi,TimerStrucSize
        dec ecx
        jnz .timer_stop_loop_2

        mov eax,0205h                   ; no user timer is active so
        mov bl,8                        ; restore previous BIOS timer
        mov cx,[oldtimerselector]
        mov edx,[oldtimeroffset]
        int 31h

        mov eax,601h                    ; unlock linear region
        mov ebx,new_timer_procedure     ; offset to PM handler
        mov cx,bx
        shr ebx,16                      ; BX:CX = address
        sub esi,esi
        mov edi,new_timer_procedure_end - new_timer_procedure
        int 31h

        mov dword [timerspeed],0        ; all user timers and main timer off

.timer_stop_end:
        mov eax,1
        mov dword [_flip2_TimerErrorcode],0  ; 0 - success
        clc
.exit:
        sti
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop ebp
        ret



;******************************************************************************
;*
;* new_timer_procedure:  Main timer interrupt handler.
;*                       Called at the speed of the fastest user timer.
;*                       Warning : Stack must be 4KB or more (suggested 8KB).
;*
;******************************************************************************
align 32
new_timer_procedure:
        pushfd
        pushad
        push ds
        push es
        mov ax,[cs:flat_ds_selector]
        mov ds,ax
        mov es,ax

        mov ecx,MaxTimers
        mov esi,timersdata              ; offset to timers data
.timer_call_loop:
        cmp byte [esi+tactive],0
        je .timer_not_active
        mov eax,[esi+tcount]            ; timer count of this user timer
        add eax,[timerspeed]            ; add main (global) timer speed
        mov [esi+tcount],eax
        cmp eax,[esi+tspeed]            ; is it time to call this user timer?
        jb .timer_not_active
        sub eax,[esi+tspeed]            ; ticks that exceeded tspeed
        mov [esi+tcount],eax            ; tcount = ticks that exceeded tspeed
        push ecx
        push esi
        call dword [esi+tproc]          ; call this user timer procedure
        pop esi
        pop ecx
.timer_not_active:
        add esi,TimerStrucSize
        dec ecx
        jnz .timer_call_loop

        mov eax,[timerspeed]            ; is it time to call the old BIOS timer
        add [oldtimercount],eax
        cmp dword [oldtimercount],10000h
        jae .call_old_timer
        mov al,20h                      ; send EOI (end of interrupt)
        out 20h,al
        pop es
        pop ds
        popad
        popfd
        iretd                           ; irq end

.call_old_timer:
        sub dword [oldtimercount],10000h
        pop es
        pop ds
        popad
        popfd
        jmp dword far [cs:oldtimeroffset] ; chain to old BIOS timer


        align 32
timersdata times (MaxTimers*TimerStrucSize) db 0
timerspeed dd 0                           ; main (global) timer speed
oldtimercount dd 0                        ; old BIOS timer count
flat_ds_selector dd 0                     ; for the interrupt handler
oldtimeroffset dd 0                       ; old BIOS timer address
oldtimerselector dd 0

new_timer_procedure_end:



;******************************************************************************
;*
;* GetMaxSpeed:  Returns the speed of the fastest timer.
;*               (internal to timer.asm).
;*
;*        Out: eax = speed of the fastest timer
;*
;******************************************************************************
GetMaxSpeed:
        push ecx
        push esi
        mov eax,10000h                  ; old BIOS timer speed
        mov ecx,MaxTimers
        mov esi,timersdata              ; offset to timers data
.timer_get_speed_loop:
        cmp byte [esi+tactive],0
        je .timer_not_active
        cmp [esi+tspeed],eax            ; slowest value for the fastest timer
        jae .timer_not_active           ; is the old BIOS timer speed.
        mov eax,[esi+tspeed]
.timer_not_active:
        add esi,TimerStrucSize
        dec ecx
        jnz .timer_get_speed_loop
        pop esi
        pop ecx
        ret



;******************************************************************************
;*
;* SetTimerSpeed:  Sets the new main (global) timer speed.
;*                 (internal to timer.asm).
;*
;*        In: eax = timer value (frequency = 1193181/eax)
;*
;******************************************************************************
SetTimerSpeed:
        cmp [timerspeed],eax
        je .same_speed
        push eax
        push ebx
        mov ebx,eax
        cli                             ; set the PIT channel 0 frequency
        mov al,00110110b                ; 34h - 54 - load 16bit value (mode 2)
        out 43h,al                      ; control word
        mov eax,ebx                     ; eax = speed
        out 40h,al                      ; LSB (least significant byte)
        mov al,ah
        out 40h,al                      ; MSB (most significant byte)
        sti
        pop ebx
        pop eax
        mov [timerspeed],eax            ; set new main (global) timer speed
.same_speed:
        ret


