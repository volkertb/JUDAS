; Timer interrupt handler
; Modified by BSpider for NASM on 5th Sep 2006

                                %include "segments.inc"
                %ifdef djgpp
                section .text
                %else
                segment _TEXT
                %endif

                global  _timer_code_lock_start
                global  _timer_code_lock_end
                global  _timer_oldvect
                global  _timer_count
                global  _timer_systemcount
                global  _timer_frequency
                global  _timer_ds
                global  _timer_function
                ; expose both calling conventions
                global  timer_handler_
                global  _timer_handler

_timer_code_lock_start:

                align   4

_timer_ds       dw      0
_timer_oldvect  dd      0, 0
_timer_count    dd      0
_timer_systemcount dw   0
_timer_frequency dw     0
_timer_function dd      0

                align   4


timer_handler_:
_timer_handler:
                pushad
                push    DS
                push    ES
                mov     DS, [CS:_timer_ds]
                mov     ES, [CS:_timer_ds]
                sti
                mov     AL, 20h
                out     20h, AL
                inc     dword [_timer_count]
                call    dword [_timer_function]
                mov     AX, [_timer_frequency]
                add     [_timer_systemcount], AX
                jc      timer_callold
                pop     ES
                pop     DS
                popad
                iretd
timer_callold:  pop     ES
                pop     DS
                popad
                jmp     dword far [CS:_timer_oldvect]

_timer_code_lock_end:

;                end
