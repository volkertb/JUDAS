
;
; basic DPMI functions for C/C++ by Piotr Ulaszewski (2000 - 2002)
; modified and improved for === THE NEXT STEP RELEASE 0.10 ===
;

global _DPMI_DOSmalloc              ; function 100h
global _DPMI_DOSfree                ; function 101h
global _DPMI_GetRMVector            ; function 200h
global _DPMI_SetRMVector            ; function 201h
global _DPMI_GetPMVector            ; function 204h
global _DPMI_SetPMVector            ; function 205h
global _DPMI_SimulateRMI            ; function 300h
global _DPMI_Malloc                 ; function 501h
global _DPMI_Free                   ; function 502h
global _DPMI_Lock                   ; function 600h
global _DPMI_Unlock                 ; function 601h
global _DPMI_MapMemory              ; function 800h
global _DPMI_UnmapMemory            ; function 801h


segment _TEXT public align=256 class=CODE use32

; bool DPMI_DOSmalloc (unsigned long size, unsigned short *segment, unsigned short *selector)
_DPMI_DOSmalloc:
        push ebx
        push edx
        mov eax,0100h
        mov ebx,[esp+4+8]              ; size in bytes
        add ebx,0fh
        shr ebx,4                      ; convert to para
        int 31h
        jnc .malloc_ok
        xor eax,eax
        jmp short .exit
.malloc_ok:
        mov ebx,[esp+8+8]              ; ebx = pointer to segment of allocated block
        mov [ebx],ax
        mov ebx,[esp+12+8]             ; ebx = pointer to selector of allocated block
        mov [ebx],dx
        mov eax,1
.exit:
        pop edx
        pop ebx
        ret


; void DPMI_DOSfree (unsigned short *selector);
_DPMI_DOSfree:
        push eax
        push edx
        mov eax,[esp+4+8]              ; eax = pointer to selector of freed block
        mov dx,[eax]
        mov eax,0101h
        int 31h
        pop edx
        pop eax
        ret


; void DPMI_GetRMVector (unsigned char IntNum, unsigned short *segment, unsigned short *offset);
_DPMI_GetRMVector:
        push ebp
        mov ebp,esp
        push eax
        push ebx
        push ecx
        push edx
        mov eax,0200h
        movzx ebx,byte [ebp+8]         ; IntNum
        int 31h
        mov ebx,[ebp+12]               ; pointer to segment of RM interrupt vector
        mov [ebx],cx
        mov ebx,[ebp+16]               ; pointer to offset of RM interrupt vector
        mov [ebx],dx
        pop edx
        pop ecx
        pop ebx
        pop eax
        ret


; void DPMI_SetRMVector (unsigned char IntNum, unsigned short *segment, unsigned short *offset);
_DPMI_SetRMVector:
        push ebp
        mov ebp,esp
        push eax
        push ebx
        push ecx
        push edx
        mov ebx,[ebp+12]               ; pointer to segment of RM interrupt vector
        mov cx,[ebx]                   ; segment
        mov ebx,[ebp+16]               ; pointer to offset of RM interrupt vector
        mov dx,[ebx]                   ; offset
        movzx ebx,byte [ebp+8]         ; IntNum
        mov eax,0201h
        int 31h
        pop edx
        pop ecx
        pop ebx
        pop eax
        ret


; void DPMI_GetPMVector (unsigned char IntNum, unsigned short *selector, unsigned long *offset);
_DPMI_GetPMVector:
        push ebp
        mov ebp,esp
        push eax
        push ebx
        push ecx
        push edx
        mov eax,0204h
        movzx ebx,byte [ebp+8]         ; IntNum
        int 31h
        mov ebx,[ebp+12]               ; pointer to selector of interrupt vector
        mov [ebx],cx
        mov ebx,[ebp+16]               ; pointer to offset of interrupt vector
        mov [ebx],edx
        pop edx
        pop ecx
        pop ebx
        pop eax
        ret


; void DPMI_SetPMVector (unsigned char IntNum, unsigned short *selector, unsigned long *offset);
_DPMI_SetPMVector:
        push ebp
        mov ebp,esp
        push eax
        push ebx
        push ecx
        push edx
        mov ebx,[ebp+12]               ; pointer to selector of interrupt vector
        mov cx,[ebx]                   ; selector
        mov ebx,[ebp+16]               ; pointer to offset of interrupt vector
        mov edx,[ebx]                  ; offset
        movzx ebx,byte [ebp+8]         ; IntNum
        mov eax,0205h
        int 31h
        pop edx
        pop ecx
        pop ebx
        pop eax
        ret


; bool DPMI_SimulateRMI (unsigned char IntNum, DPMIREGS *regs);
_DPMI_SimulateRMI:
        push ebp
        mov ebp,esp
        pushad
        push es
        push ds
        pop es
        mov edi,[ebp+12]               ; ES:EDI = DPMIREGS pointer
        sub ecx,ecx
        sub ebx,ebx
        mov bl,[ebp+8]                 ; IntNum
        mov eax,300h
        int 31h
        jc .error
        pop es
        popad
        pop ebp
        mov eax,1                      ; success
        ret
.error:
        pop es
        popad
        pop ebp
        sub eax,eax                    ; failure
        ret


; void *DPMI_Malloc (unsigned long size, unsigned long *handle)
_DPMI_Malloc:
        push ebp
        mov ebp,esp
        push ebx
        push ecx
        push edx
        push esi
        push edi

        mov eax,501h
        mov ebx,[ebp+8]                ; size
        shr ebx,16
        mov ecx,[ebp+8]                ; size
        and ecx,0ffffh                 ; BX:CX = size
        int 31h
        jnc .malloc_ok
        xor eax,eax
        jmp short .exit
.malloc_ok:
        shl ebx,16
        mov bx,cx
        mov eax,ebx
        mov ebx,[ebp+12]               ; pointer to handle
        shl esi,16
        mov si,di
        mov [ebx],esi                  ; handle
.exit:
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop ebp
        ret


; void DPMI_Free (unsigned long *handle)
_DPMI_Free:
        push eax
        push esi
        push edi
        mov eax,[esp+4+12]             ; pointer to handle
        mov esi,[eax]
        mov di,si
        shr esi,16
        mov eax,502h
        int 31h
        pop edi
        pop esi
        pop eax
        ret


; bool DPMI_Lock (unsigned long *address, unsigned long size);
_DPMI_Lock:
        push ebp
        mov ebp,esp
        pushad
        mov eax,[ebp+8]                ; pointer to address
        mov ebx,[eax]                  ; starting address
        mov cx,bx
        shr ebx,16                     ; BX:CX = starting address
        mov esi,[ebp+12]               ; size in bytes
        mov di,si
        shr esi,16                     ; SI:DI = size in bytes
        mov eax,600h                   ; lock linear region
        int 31h
        jc .error
        popad
        pop ebp
        mov eax,1                      ; success
        ret
.error:
        popad
        pop ebp
        sub eax,eax                    ; failure
        ret


; bool DPMI_Unlock (unsigned long *address, unsigned long size);
_DPMI_Unlock:
        push ebp
        mov ebp,esp
        pushad
        mov eax,[ebp+8]                ; pointer to address
        mov ebx,[eax]                  ; starting address
        mov cx,bx
        shr ebx,16                     ; BX:CX = starting address
        mov esi,[ebp+12]               ; size in bytes
        mov di,si
        shr esi,16                     ; SI:DI = size in bytes
        mov eax,601h                   ; unlock linear region
        int 31h
        jc .error
        popad
        pop ebp
        mov eax,1                      ; success
        ret
.error:
        popad
        pop ebp
        sub eax,eax                    ; failure
        ret


; bool DPMI_MapMemory (unsigned long *physaddress, unsigned long *linaddress, unsigned long size);
_DPMI_MapMemory:
        push ebp
        mov ebp,esp
        pushad
        mov eax,[ebp+8]                ; pointer to physical address
        mov ebx,[eax]                  ; physical address
        mov cx,bx
        shr ebx,16                     ; BX:CX = physical address
        mov esi,[ebp+16]               ; size in bytes
        mov di,si
        shr esi,16                     ; SI:DI = size in bytes
        mov eax,800h                   ; physical address mapping
        int 31h
        jc .error
        shl ebx,16
        mov bx,cx
        mov eax,[ebp+12]               ; pointer to linear address
        mov [eax],ebx                  ; linaddress = BX:CX
        popad
        pop ebp
        mov eax,1                      ; success
        ret
.error:
        mov eax,[ebp+12]               ; pointer to linear address
        mov dword [eax],0              ; linaddress = NULL on error
        popad
        pop ebp
        sub eax,eax                    ; failure
        ret


; bool DPMI_UnmapMemory (unsigned long *linaddress);
_DPMI_UnmapMemory:
        push ebp
        mov ebp,esp
        pushad
        mov eax,[ebp+8]                ; pointer to linear address
        mov ebx,[eax]                  ; linear address
        mov cx,bx
        shr ebx,16                     ; BX:CX = linear address
        mov eax,801h                   ; free physical address mapping
        int 31h
        jc .error
        popad
        pop ebp
        mov eax,1                      ; success
        ret
.error:
        popad
        pop ebp
        sub eax,eax                    ; failure
        ret


