
;******************************************************************************
;*
;* INPUT.ASM module for ULTIMATE_FLIP library
;* by Piotr Ulaszewski <BlackSpider>.
;* piotrkn22@poczta.onet.pl
;* http://www.republika.pl/piotrkn22/
;* Keyboard interrupt handler for protected mode.
;* Some info in Judas 2.06y source code about extended keys processing
;* was very usefull to develop this handler.
;* Includes DPMI lock and extended keys.
;* This keyboard handler may sometimes behave strangely under DOS4GW,
;* please use WDOSX or PMODEW instead (especially on slow systems).
;*
;* This is an extended version for C/C++ support.
;* developemnent version 0.44 (special without mouse handler)
;*
;******************************************************************************

%include "segments.inc"

; for keyboard input
global _flip2_KeyTable                          ; pointer to key table (locked)
global _flip2_KeyInit                           ; proc
global _flip2_KeyDeinit                         ; proc
global _flip2_GetKey                            ; proc
global _flip2_GetLastKey                        ; proc
global _flip2_GetKeyAscii                       ; proc
global _flip2_ScanToAscii                       ; proc
global _flip2_KeyErrorcode                      ; dword


segment _DATA
; data values for keyboard handler
_flip2_KeyTable dd 0                            ; address of key_tab
old_key_handler_offset dd 0
old_key_handler_selector dd 0
_flip2_KeyErrorcode dd 0                        ; error code for key functions


;*********   A S C I I   C O N V E R S I O N   T A B L E   1   ****************
; Ascii conversion table (SHIFT key depressed)
ScanToAsciiTable1 db 0,27,49,50,51,52,53,54,55,56,57,48,45,61,8,9
; nt esc 1 2 3 4 5 6 7 8 9 0 - = bs ht

db 113,119,101,114,116,121,117,105,111,112,91,93,13,0,97,115
; q w e r t y u i o p [ ] cr (ctrll) a s

db 100,102,103,104,106,107,108,59,39,96,0,92,122,120,99,118
; d f g h j k l ; ' ` (shl) \ z x c v

db 98,110,109,44,46,47,0,42,0,32,0,0,0,0,0,0
; b n m , . / (shr) * (altl) spc (caps) (f1) (f2) (f3) (f4) (f5)

db 0,0,0,0,0,0,0,55,56,57,45,52,53,54,43,49
; (f6) (f7) (f8) (f9) (f10) (nl) (sl) kp7 kp8 kp9 g- kp4 kp5 kp6 g+ kp1

db 50,51,48,46,0,0,0,0,0,0,0,0,0,0,0,0
; kp2 kp3 kp0 kppt nt nt nt (f11) (f12) nt nt nt nt nt nt nt

times 160 db 0
; all extended keys zeroed


;*********   A S C I I   C O N V E R S I O N   T A B L E   2   ****************
; Ascii conversion table (SHIFT key pressed)
ScanToAsciiTable2 db 0,27,33,64,35,36,37,94,38,42,40,41,95,43,8,9
; nt esc ! @ # $ % ^ & * ( ) _ + bs ht

db 81,87,69,82,84,89,85,73,79,80,123,125,13,0,65,83
; Q W E R T Y U I O P { } cr (ctrll) A S

db 68,70,71,72,74,75,76,58,34,126,0,124,90,88,67,86
; D F G H J K L : " ~ (shl) | Z X C V

db 66,78,77,60,62,63,0,42,0,32,0,0,0,0,0,0
; B N M < > ? (shr) * (altl) spc (caps) (f1) (f2) (f3) (f4) (f5)

db 0,0,0,0,0,0,0,55,56,57,45,52,53,54,43,49
; (f6) (f7) (f8) (f9) (f10) (nl) (sl) kp7 kp8 kp9 g- kp4 kp5 kp6 g+ kp1

db 50,51,48,46,0,0,0,0,0,0,0,0,0,0,0,0
; kp2 kp3 kp0 kppt nt nt nt (f11) (f12) nt nt nt nt nt nt nt

times 160 db 0
; all extended keys zeroed

;******************************************************************************


;******************************************************************************
;*
;* Keyboard handler functions.
;* Multiple calls to these functions will not hang the machine.
;* Warning - this will only work with 4GB flat mode zero based segments
;*           where the base and limit of CS is equal to those of DS.
;*
;* enable    :      call _flip2_KeyInit - returns TRUE or FALSE
;*
;* disable   :      call _flip2_KeyDeinit - returns nothing
;*
;*       out :      eax = TRUE(1) on success or FALSE(0) on error
;*                        _flip2_KeyErrorcode holds the function exec status
;*                                = 0 - success
;*                                = 1 - DPMI lock error
;*                                = 2 - set interrupt vector error
;*
;******************************************************************************
segment _TEXT
_flip2_KeyInit:
      cmp dword [old_key_handler_offset],0
      jne near .exit_2                  ; key handler already running -> exit
      push ebx
      push ecx
      push edx
      push esi
      push edi
      push es
      push ds
      pop es
      cld
      cli

      mov eax,600h                      ; lock linear region
      mov ebx,irq9                      ; offset to key handler
      mov cx,bx
      shr ebx,16                        ; BX:CX = address
      sub esi,esi
      mov edi,end_of_key_lock - irq9    ; length of handler + table
      int 31h
      jnc .dpmi_lock_ok
      sub eax,eax
      mov dword [_flip2_KeyErrorcode],1 ; DPMI lock error
      jmp short .exit

.dpmi_lock_ok:
      mov eax,key_tab                   ; clear key table
      mov [_flip2_KeyTable],eax         ; address of key_tab
      mov edi,eax
      mov ecx,64
      sub eax,eax
      rep stosd

      mov bl,9                          ; get old key vector
      mov eax,204h
      int 31h
      mov [old_key_handler_offset],edx
      mov [old_key_handler_selector],cx

      mov ax,ds
      mov [key_flat_mode_data_selector],ax

      mov edx,irq9                      ; set new key vector
      mov cx,cs
      mov bl,9
      mov eax,205h
      int 31h
      jnc .set_vector_ok
      sub eax,eax
      mov dword [_flip2_KeyErrorcode],2 ; set interrupt vector error
      jmp short .exit

.set_vector_ok:
      mov eax,1
      mov dword [_flip2_KeyErrorcode],0 ; status = success

.exit:
      sti
      pop es
      pop edi
      pop esi
      pop edx
      pop ecx
      pop ebx
.exit_2:
      ret



;******************************************************************************
_flip2_KeyDeinit:
      cmp dword [old_key_handler_offset],0
      je .exit                          ; no key handler running -> exit
      pushad
      cli

      mov edx,[old_key_handler_offset]  ; restore old key handler
      mov cx,[old_key_handler_selector]
      mov bl,9
      mov eax,205h
      int 31h

      mov eax,601h                      ; unlock linear region
      mov ebx,irq9                      ; offset to key handler
      mov cx,bx
      shr ebx,16
      sub esi,esi
      mov edi,end_of_key_lock - irq9    ; length of handler + table
      int 31h

      mov dword [old_key_handler_offset],0
      mov word [old_key_handler_selector],0
      mov dword [_flip2_KeyErrorcode],0 ; status = success
      sti
      popad
.exit:
      ret



;******************************************************************************
      align 32

irq9:                                     ; KEYBOARD HANDLER
;      pushfd
      pushad
      push ds
      mov ds,[cs:key_flat_mode_data_selector] ; needed for IRQ

      sub eax,eax
      in al,60h                           ; get key code
      cmp byte [pause_key],0
      je .@@no_pause
      dec byte [pause_key]
      jmp .@@end
.@@no_pause:
      cmp al,0e0h                         ; will the next be extended
      jne .@@no_extended
      mov byte [extended_key],1           ; set flag
      jmp .@@end
.@@no_extended:
      cmp al,0e1h                         ; is it pause
      jne .@@no_pause2
      xor byte [key_tab + 255],1          ; set/clear pause state
      mov byte [pause_key],5              ; 5 codes coming to discard
      mov byte [was_key_pressed],1        ; for GetKey function
      mov byte [last_key_scancode],255    ; pause key scancode
      jmp short .@@end
.@@no_pause2:
      sub ebx,ebx                         ; reset to normal key offset
      cmp byte [extended_key],1
      jne .@@no_extended2
      mov byte [extended_key],0           ; reset flag
      cmp al,2ah                          ; skip system request
      je  .@@end
      mov bl,80h                          ; extended key offset
.@@no_extended2:
      cmp al,080h                         ; press or release
      jae .@@clr
      mov edi,key_tab                     ; offset of key table
      add edi,eax
      add edi,ebx                         ; normal or extended
      mov byte [edi],1                    ; press
      mov byte [was_key_pressed],1        ; for GetKey function
      mov byte [last_key_scancode],bl     ; extended key ?
      add byte [last_key_scancode],al     ; key scancode
      jmp short .@@end
.@@clr:
      mov edi,key_tab                     ; offset of key table
      sub eax,080h
      add edi,eax
      add edi,ebx                         ; normal or extended
      mov byte [edi],0                    ; release
.@@end:
      mov al,020h                         ; send EOI
      out 20h,al
      pop ds
      popad
;      popfd
      iretd

      align 32

key_tab resb 256                          ; a 256 byte buffer to hold key info
key_flat_mode_data_selector dd 0          ; needed for IRQ
extended_key db 0
pause_key db 0
was_key_pressed db 0                      ; for GetKey function
last_key_scancode db 0
end_of_key_lock:



;******************************************************************************
;*
;* flip2_GetKey - Waits until a key is pressed and retrieves it's scan code.
;*                Look at scans.h or scans.inc for info on key naming.
;*
;*        in :    call _flip2_GetKey
;*
;*       out :    eax = pressed key scancode
;*
;******************************************************************************
_flip2_GetKey:
.wait:
        cmp byte [was_key_pressed],1
        jne .wait
        sub eax,eax
        mov al,[last_key_scancode]
        mov byte [was_key_pressed],0
        ret



;******************************************************************************
;*
;* flip2_GetLastKey - returns immediately with last pressed key scan code
;*                    or NULL if no key pressed.
;*                    Look at scans.h or scans.inc for info on key naming.
;*
;*          in :      call _flip2_GetLastKey
;*
;*         out :      eax = pressed key scancode
;*
;******************************************************************************
_flip2_GetLastKey:
        sub eax,eax
        mov al,[last_key_scancode]
        mov byte [last_key_scancode],0
        mov byte [was_key_pressed],0
        ret



;******************************************************************************
;*
;* flip2_GetKeyAscii
;*
;*       Waits until a key is pressed and retrieves it's ascii code.
;*       NumLock is considered as active (even if it is not)
;*       CapsLock is considered as inactive (even if it is active)
;*       ScrollLock is considered as inactive (even if it is active)
;*       Special keys (Alt, Ctrl, Shift, ...) will not be returned.
;*       Backspace-8, Tab-9, Enter-13, Escape-27 will be returned.
;*
;*        in :      call _flip2_GetKeyAscii
;*
;*       out :      eax = pressed key ascii code
;*
;******************************************************************************
_flip2_GetKeyAscii:
        push ebx
.wait:
        cmp byte [was_key_pressed],1
        jne .wait
        sub eax,eax
        cmp byte [key_tab + 42],1         ; is left shift pressed ?
        je .second_ascii_table
        cmp byte [key_tab + 54],1         ; is right shift pressed ?
        je .second_ascii_table
        mov ebx,ScanToAsciiTable1         ; translation table 1
        jmp short .retrieve_ascii_code
.second_ascii_table:
        mov ebx,ScanToAsciiTable2         ; translation table 2
.retrieve_ascii_code:
        mov al,[last_key_scancode]
        xlatb
        mov byte [was_key_pressed],0
        test al,al                        ; do not return special keys
        jz .wait
        pop ebx
        ret



;******************************************************************************
;*
;* flip2_ScanToAscii
;*       Converts a key scan code to it's ascii code.
;*       Special keys (Alt, Ctrl, Shift, ...) will return NULL.
;*       Backspace-8, Tab-9, Enter-13, Escape-27 will be returned.
;*       This function will use the ASCII conversion table 1 to
;*       perform the translation therefore only chars from that
;*       table will be returend (eg. only small letters).
;*
;*                  push dword scan_key
;*        in :      call _flip2_ScanToAscii
;*
;*       out :      eax = pressed key ascii code
;*
;******************************************************************************
_flip2_ScanToAscii:
        push ebx
        sub eax,eax
        mov ebx,ScanToAsciiTable1         ; translation table 1
        mov al,[esp + 8]                  ; scan_key
        xlatb
        pop ebx
        ret
