; This module contains soundcard IRQ handlers, DMA routines, mixing routines
; as well as some time-critical GUS routines and AC97 access code.
;
; N„m„ ei mit„„n (kovin) optimisoituja rutiineja ole, PERKELE!
;
; Modified by BSpider for NASM on 5th Sep 2006

                %idefine offset
                %include "segments.inc"
                %include "judascfg.inc"
                %include "judasgus.inc"
                %include "judasac.inc"

%define         MONO            0
%define         EIGHTBIT        0
%define         STEREO          1
%define         SIXTEENBIT      2

%define         VM_OFF          0
%define         VM_ON           1
%define         VM_LOOP         2
%define         VM_16BIT        4

%define         DEV_NOSOUND     0
%define         DEV_SB          1
%define         DEV_SBPRO       2
%define         DEV_SB16        3
%define         DEV_GUS         4
%define         DEV_AC97        5
%define         DEV_HDA         6
%define         DEV_FILE        7

%define         CACHESLOTS      16

%define         IPMINUS1        -1
%define         IP0              0
%define         IP1              1
%define         IP2              2

; predefined to 0
struc           CACHESLOT
                GDC_Pos         resd 1
                GDC_Length      resd 1
endstruc

; predefined to 0
struc           DMACHANNEL
                DMA_PagePort    resw 1
                DMA_AdrPort     resw 1
                DMA_LenPort     resw 1
                DMA_MaskPort    resw 1
                DMA_ModePort    resw 1
                DMA_ClearPort   resw 1
                DMA_Mask        resb 1
                DMA_UnMask      resb 1
                DMA_Unused      resw 1
endstruc

; predefined to 0
struc           CHANNEL
                Chn_Pos          resd 1
                Chn_Repeat       resd 1
                Chn_End          resd 1
                Chn_Sample       resd 1
                Chn_Freq         resd 1
                Chn_FractPos     resw 1
                Chn_MasterVol    resb 1
                Chn_Panning      resb 1
                Chn_Vol          resw 1
                Chn_VoiceMode    resb 1
                Chn_PrevVM       resb 1
                Chn_PrevPos      resd 1
                Chn_LastValL     resd 1
                Chn_LastValR     resd 1
                Chn_SmoothVolL   resd 1
                Chn_SmoothVolR   resd 1
endstruc



; not predefined
struc           AUDIO_PCI_DEV
                .vender_id              resw 1
                .device_id              resw 1
                .sub_vender_id          resw 1
                .sub_device_id          resw 1
                .device_bus_number      resw 1
                .irq                    resb 1
                .pin                    resb 1
                .command                resw 1
                .base0                  resd 1
                .base1                  resd 1
                .base2                  resd 1
                .base3                  resd 1
                .base4                  resd 1
                .base5                  resd 1
                .device_type            resd 1

                .mem_mode               resd 1
                .hda_mode               resd 1

; memory allocated for BDL and PCM buffers
                .bdl_buffer             resd 1
                .pcmout_buffer0         resd 1
                .pcmout_buffer1         resd 1
                .hda_buffer             resd 1
                .pcmout_bufsize         resd 1
                .pcmout_bdl_entries     resd 1
                .pcmout_bdl_size        resd 1
                .pcmout_dmasize         resd 1
                .pcmout_dma_lastgoodpos resd 1
                .pcmout_dma_pos_ptr     resd 1

; AC97 only properties
                .ac97_vra_supported     resd 1

; HDA modified structure will be placed here.
                .codec_mask             resd 1
                .codec_index            resd 1

                .afg_root_nodenum       resw 1
                .afg_num_nodes          resd 1
                .afg_nodes              resd 1
                .def_amp_out_caps       resd 1
                .def_amp_in_caps        resd 1

                .dac_node               resd 1
                .out_pin_node           resd 1
                .adc_node               resd 1
                .in_pin_node            resd 1
                .input_items            resd 1
                .pcm_num_vols           resd 1
                .pcm_vols               resd 1

                .format_val             resd 1
                .dacout_num_bits        resd 1
                .dacout_num_channels    resd 1
                .stream_tag             resd 1
                .supported_formats      resd 1
                .supported_max_freq     resd 1
                .supported_max_bits     resd 1

                .freq_card              resd 1
                .chan_card              resd 1
                .bits_card              resd 1

                .codec_id1              resw 1
                .codec_id2              resw 1
                .device_name            resb 128
                .codec_name             resb 128
endstruc

%define         DEVICE_INTEL            0    ; AC97 device Intel ICH compatible
%define         DEVICE_SIS              1    ; AC97 device SIS compatible
%define         DEVICE_INTEL_ICH4       2    ; AC97 device Intel ICH4 compatible
%define         DEVICE_NFORCE           3    ; AC97 device nForce compatible
%define         DEVICE_HDA_INTEL        4    ; HDA audio device for Intel  and others
%define         DEVICE_HDA_ATI          5
%define         DEVICE_HDA_ATIHDMI      6
%define         DEVICE_HDA_NVIDIA       7
%define         DEVICE_HDA_SIS          8
%define         DEVICE_HDA_ULI          9
%define         DEVICE_HDA_VIA          10

                ; register calling convention for WATCOM C++
                global  judas_code_lock_start_
                global  judas_code_lock_end_
                global  judas_update_
                global  judas_get_ds_
                global  sb_handler_
                global  sb_aihandler_
                global  sb16_handler_
                global  gus_handler_
                global  gus_peek_
                global  gus_poke_
                global  gus_dmawait_
                global  gus_dmainit_
                global  gus_dmaprogram_
                global  gus_startchannels_
                global  fmixer_
                global  qmixer_
                global  safemixer_
                global  normalmix_
                global  ipmix_
                global  qmix_linear_
                global  qmix_cubic_
                global  dma_program_
                ; stack calling convention for anything else
                global  _judas_code_lock_start
                global  _judas_code_lock_end
                global  _judas_update
                global  _judas_get_ds
                global  _sb_handler
                global  _sb_aihandler
                global  _sb16_handler
                global  _gus_handler
                global  _gus_peek
                global  _gus_poke
                global  _gus_dmawait
                global  _gus_dmainit
                global  _gus_dmaprogram
                global  _gus_startchannels
                global  _fmixer
                global  _qmixer
                global  _safemixer
                global  _normalmix
                global  _ipmix
                global  _qmix_linear
                global  _qmix_cubic
                global  _dma_program

                extern   _judas_ds;word
                extern   _judas_initialized;byte
                extern   _judas_mixmode;byte
                extern   _judas_samplesize;byte
                extern   _judas_clipbuffer;dword
                extern   _judas_zladdbuffer;dword
                extern   _judas_zerolevell;dword
                extern   _judas_zerolevelr;dword
                extern   _judas_cliptable;dword
                extern   _judas_volumetable;dword
                extern   _judas_mixrate;dword
                extern   _judas_channel;dword
                extern   _judas_mixroutine;dword
                extern   _judas_mixersys;dword
                extern   _judas_device;dword
                extern   _judas_port;dword
                extern   _judas_irq;dword
                extern   _judas_dma;dword
                extern   _judas_irqcount;dword
                extern   _judas_bufferlength;dword
                extern   _judas_buffermask;dword
                extern   _judas_bpmcount;dword
                extern   _judas_bpmtempo;byte
                extern   _judas_player;dword
                extern   _judas_mixpos;dword
                extern   _dma_address;dword
                extern   _judas_clipped;byte
                extern   _audio_pci;AUDIO_PCI_DEV
                extern   _hda_civ ; dword
                extern   _hda_lpib ; dword

%ifdef djgpp
section .text
%else
segment _TEXT
%endif

judas_get_ds_:
_judas_get_ds:
                mov     AX, DS
                mov     [_judas_ds], AX
                ret

judas_code_lock_start_:
_judas_code_lock_start:
; this code is constant - TASM declaration of .const

                align   4

DMAChannels:
                istruc DMACHANNEL
                    at DMA_PagePort,  dw 87h
                                        at DMA_AdrPort,   dw 0h
                                        at DMA_LenPort,   dw 1h
                                        at DMA_MaskPort,  dw 0ah
                                        at DMA_ModePort,  dw 0bh
                                        at DMA_ClearPort, dw 0ch
                                        at DMA_Mask,      db 4h
                                        at DMA_UnMask,    db 0h
                                        at DMA_Unused,    dw 0h
                iend

                istruc DMACHANNEL
                    at DMA_PagePort,  dw 83h
                                        at DMA_AdrPort,   dw 2h
                                        at DMA_LenPort,   dw 3h
                                        at DMA_MaskPort,  dw 0ah
                                        at DMA_ModePort,  dw 0bh
                                        at DMA_ClearPort, dw 0ch
                                        at DMA_Mask,      db 5h
                                        at DMA_UnMask,    db 1h
                                        at DMA_Unused,    dw 0h
                iend

                                istruc DMACHANNEL
                    at DMA_PagePort,  dw 81h
                                        at DMA_AdrPort,   dw 4h
                                        at DMA_LenPort,   dw 5h
                                        at DMA_MaskPort,  dw 0ah
                                        at DMA_ModePort,  dw 0bh
                                        at DMA_ClearPort, dw 0ch
                                        at DMA_Mask,      db 6h
                                        at DMA_UnMask,    db 2h
                                        at DMA_Unused,    dw 0h
                iend

                                istruc DMACHANNEL
                    at DMA_PagePort,  dw 82h
                                        at DMA_AdrPort,   dw 6h
                                        at DMA_LenPort,   dw 7h
                                        at DMA_MaskPort,  dw 0ah
                                        at DMA_ModePort,  dw 0bh
                                        at DMA_ClearPort, dw 0ch
                                        at DMA_Mask,      db 7h
                                        at DMA_UnMask,    db 3h
                                        at DMA_Unused,    dw 0h
                iend

                istruc DMACHANNEL
                    at DMA_PagePort,  dw 8fh
                                        at DMA_AdrPort,   dw 0c0h
                                        at DMA_LenPort,   dw 0c2h
                                        at DMA_MaskPort,  dw 0d4h
                                        at DMA_ModePort,  dw 0d6h
                                        at DMA_ClearPort, dw 0d8h
                                        at DMA_Mask,      db 4h
                                        at DMA_UnMask,    db 0h
                                        at DMA_Unused,    dw 0h
                iend

                istruc DMACHANNEL
                    at DMA_PagePort,  dw 8bh
                                        at DMA_AdrPort,   dw 0c4h
                                        at DMA_LenPort,   dw 0c6h
                                        at DMA_MaskPort,  dw 0d4h
                                        at DMA_ModePort,  dw 0d6h
                                        at DMA_ClearPort, dw 0d8h
                                        at DMA_Mask,      db 5h
                                        at DMA_UnMask,    db 1h
                                        at DMA_Unused,    dw 0h
                iend

                istruc DMACHANNEL
                    at DMA_PagePort,  dw 89h
                                        at DMA_AdrPort,   dw 0c8h
                                        at DMA_LenPort,   dw 0cah
                                        at DMA_MaskPort,  dw 0d4h
                                        at DMA_ModePort,  dw 0d6h
                                        at DMA_ClearPort, dw 0d8h
                                        at DMA_Mask,      db 6h
                                        at DMA_UnMask,    db 2h
                                        at DMA_Unused,    dw 0h
                iend

                                istruc DMACHANNEL
                    at DMA_PagePort,  dw 8ah
                                        at DMA_AdrPort,   dw 0cch
                                        at DMA_LenPort,   dw 0ceh
                                        at DMA_MaskPort,  dw 0d4h
                                        at DMA_ModePort,  dw 0d6h
                                        at DMA_ClearPort, dw 0d8h
                                        at DMA_Mask,      db 7h
                                        at DMA_UnMask,    db 3h
                                        at DMA_Unused,    dw 0h
                iend

                align   4

shittable       dd      0, 60, 56, 52, 48, 44, 40, 36
                dd      32, 28, 24, 20, 16, 12, 8, 4

%ifdef djgpp
section .data
%else
segment _DATA
%endif

                align   4

gdc:
                %rep CACHESLOTS
                istruc CACHESLOT
                                    at GDC_Pos,       dd 0
                                        at GDC_Length,    dd 0
                iend
                                %endrep

                align   4

loopcount       dd      0
fractadd        dd      0
integeradd      dd      0
smpend          dd      0
smpsubtract     dd      0
samples         dd      0
totalwork       dd      0
postproc        dd      0
cptr            dd      0
dptr            dd      0
fptr            dd      0
ipminus1        dd      0
ip0             dd      0
ip1             dd      0
ip2             dd      0
leftvol         dd      0
rightvol        dd      0
SmoothVolL      dd      0
SmoothVolR      dd      0
saved_reg       dd      0

mix_exec        db      0
gus_dmainprogress db    0
ac97_buffer0_set  db    0
ac97_buffer1_set  db    0

%ifdef djgpp
section .text
%else
segment _TEXT
%endif

                align   4

        ;DMA functions. DMA polling is really fucked up: if reading the
        ;position too often (> 100 Hz) one may get bogus values. This is
        ;compensated by reading two values, and if their offset is too big or
        ;they're outside the buffer, the position is read again.
        ;
        ;Actually GUS fucks up just in the same way when reading the channel
        ;position. Shit, what is wrong with the hardware?!
        ;
        ;Previously I though that EMM386 causes these fuckups, but no, it
        ;wasn't so. However, under DPMI there's no fuckups!
        ;
        ;It would be really nice & simple to just update one bufferhalf at a
        ;time in the soundcard interrupt, but I think it's important to give
        ;the user full control of the sound updating, even at the expense of
        ;PAIN!!!

dma_program_:
_dma_program:
                push    ESI
                push    EDI
                push    ECX
                mov     ECX, EAX                        ;ECX = mode
                mov     EDI, EDX                        ;EDI = offset
                mov     ESI, [_judas_dma]               ;Get channel num
                cmp     ESI, 4
                jae     dma16_program
                shl     ESI, 4                          ;16 = dma struc len
                add     ESI, offset DMAChannels         ;Ptr now ready
                mov     DX, [ESI + DMA_MaskPort]
                mov     AL, [ESI + DMA_Mask]
                out     DX, AL                          ;Mask the DMA channel
                xor     AL, AL
                mov     DX, [ESI + DMA_ClearPort]
                out     DX, AL                          ;Clear byte ptr.
                mov     DX, [ESI + DMA_ModePort]
                mov     AL, CL                          ;Get mode
                or      AL, [ESI + DMA_UnMask]          ;Or with channel num
                out     DX, AL                          ;Set DMA mode
                mov     DX, [ESI + DMA_LenPort]
                dec     EBX                             ;EBX = length
                mov     AL, BL
                out     DX, AL                          ;Set length low and
                mov     AL, BH                          ;high bytes
                out     DX, AL
                mov     DX, [ESI + DMA_AdrPort]
                mov     EBX, [_dma_address]             ;Get DMA buffer address
                add     EBX, EDI                        ;Add offset
                mov     AL, BL
                out     DX, AL                          ;Set offset
                mov     AL, BH
                out     DX, AL
                mov     DX, [ESI + DMA_PagePort]
                shr     EBX, 16
                mov     AL, BL
                out     DX, AL                          ;Set page
                mov     DX, [ESI + DMA_MaskPort]
                mov     AL, [ESI + DMA_UnMask]
                out     DX, AL                          ;Unmask the DMA channel
                pop     ECX
                pop     EDI
                pop     ESI
                ret
dma16_program:  shl     ESI, 4                          ;16 = dma struc len
                add     ESI, offset DMAChannels         ;Ptr now ready
                mov     DX, [ESI + DMA_MaskPort]
                mov     AL, [ESI + DMA_Mask]
                out     DX, AL                          ;Mask the DMA channel
                xor     AL, AL
                mov     DX, [ESI + DMA_ClearPort]
                out     DX, AL                          ;Clear byte ptr.
                mov     DX, [ESI + DMA_ModePort]
                mov     AL, CL                          ;Get mode
                or      AL, [ESI + DMA_UnMask]          ;Or with channel num
                out     DX, AL                          ;Set DMA mode
                mov     DX, [ESI + DMA_LenPort]
                shr     EBX, 1
                dec     EBX
                mov     AL, BL
                out     DX, AL                          ;Set length low and
                mov     AL, BH                          ;high bytes
                out     DX, AL
                mov     DX, [ESI + DMA_AdrPort]
                mov     EBX, [_dma_address]             ;Get DMA buffer address
                add     EBX, EDI                        ;Add offset
                shr     EBX, 1                          ;Because of 16-bitness
                mov     AL, BL
                out     DX, AL                          ;Set offset
                mov     AL, BH
                out     DX, AL
                mov     DX, [ESI + DMA_PagePort]
                shr     EBX, 15
                mov     AL, BL
                out     DX, AL                          ;Set page
                mov     DX, [ESI + DMA_MaskPort]
                mov     AL, [ESI + DMA_UnMask]
                out     DX, AL                          ;Unmask the DMA channel
                pop     ECX
                pop     EDI
                pop     ESI
                ret

dma_query_:     cli
                push    EBX
                push    ECX
                push    EDX
                push    ESI
                mov     ESI, [_judas_dma]
                cmp     ESI, 4
                jae     dma16_query
                shl     ESI, 4                          ;16 = dma struc len
                add     ESI, offset DMAChannels         ;Ptr now ready
                xor     EAX, EAX
                mov     DX, [ESI + DMA_ClearPort]       ;Clear flip-flop
                out     DX, AL
                mov     DX, [ESI + DMA_AdrPort]
dqloop1:        xor     EAX, EAX
                in      AL, DX
                xchg    AL, AH
                in      AL, DX
                xchg    AL, AH
                sub     AX, word [_dma_address]         ;Subtract page offset
                mov     EBX, EAX                        ;EBX = position 1
                in      AL, DX
                xchg    AL, AH
                in      AL, DX
                xchg    AL, AH
                sub     AX, word [_dma_address]         ;Subtract page offset
                mov     ECX, EAX                        ;ECX = position 2
                cmp     EBX, [_judas_bufferlength]      ;Outside buffer?
                jae     dqloop1
                mov     EAX, EBX
                sub     EAX, ECX
                cmp     EAX, 64
                jg      dqloop1
                cmp     EAX, -64
                jl      dqloop1
                mov     EAX, EBX
                pop     ESI
                pop     EDX
                pop     ECX
                pop     EBX
                sti
                ret
dma16_query:    shl     ESI, 4                          ;16 = dma struc len
                add     ESI, offset DMAChannels         ;Ptr now ready
                mov     DX, [ESI + DMA_ClearPort]       ;Clear flip-flop
                xor     EAX, EAX
                out     DX, AL
                mov     DX, [ESI + DMA_AdrPort]
                mov     ESI, [_dma_address]
                and     ESI, 1ffffh
dqloop2:        xor     EAX, EAX
                in      AL, DX
                xchg    AL, AH
                in      AL, DX
                xchg    AL, AH
                shl     EAX, 1
                sub     EAX, ESI                        ;Subtract page offset
                mov     EBX, EAX                        ;EBX = position 1
                xor     EAX, EAX
                in      AL, DX
                xchg    AL, AH
                in      AL, DX
                xchg    AL, AH
                shl     EAX, 1
                sub     EAX, ESI                        ;Subtract page offset
                mov     ECX, EAX                        ;ECX = position 2
                cmp     EBX, [_judas_bufferlength]      ;Outside buffer?
                jae     dqloop2
                mov     EAX, EBX
                sub     EAX, ECX
                cmp     EAX, 64
                jg      dqloop2
                cmp     EAX, -64
                jl      dqloop2
                mov     EAX, EBX
                pop     ESI
                pop     EDX
                pop     ECX
                pop     EBX
                sti
                ret

        ;Generic send-EOI routine.

send_eoi:       inc     dword [_judas_irqcount]
                cmp     dword [_judas_irq], 8
                jae     highirq
                mov     AL, 20h
                out     20h, AL
                ret
highirq:        mov     AL, 20h
                out     0a0h, AL
                mov     AL, 00001011b
                out     0a0h, AL
                in      AL, 0a0h
                or      AL, AL
                jnz     sb_noeoi
                mov     AL, 20h
                out     20h, AL
sb_noeoi:       ret

        ;Soundblaster IRQ handlers, one for singlecycle, one for 8bit autoinit
        ;and one for 16bit autoinit.

sb_handler_:
_sb_handler:
                pushad
                push    DS
                mov     AX, [CS:_judas_ds]
                mov     DS, AX
                mov     EDX, [_judas_port]
                add     EDX, 0eh
                in      AL, DX
                sub     EDX, 2h
sb_wait1:       in      AL, DX
                or      AL, AL
                js      sb_wait1
                mov     AL, 14h
                out     DX, AL
sb_wait2:       in      AL, DX
                or      AL, AL
                js      sb_wait2
                mov     AX, 0fff0h
                out     DX, AL
sb_wait3:       in      AL, DX
                or      AL, AL
                js      sb_wait3
                mov     AL, AH
                out     DX, AL
                sti
                call    send_eoi
                pop     DS
                popad
                iretd

sb_aihandler_:
_sb_aihandler:
                pushad
                push    DS
                mov     AX, [CS:_judas_ds]
                mov     DS, AX
                mov     EDX, [_judas_port]
                add     EDX, 0eh
                in      AL, DX
                sti
                call    send_eoi
                pop     DS
                popad
                iretd

sb16_handler_:
_sb16_handler:
                pushad
                push    DS
                mov     AX, [CS:_judas_ds]
                mov     DS, AX
                mov     EDX, [_judas_port]
                add     EDX, 0fh
                in      AL, DX
                sti
                call    send_eoi
                pop     DS
                popad
                iretd

        ;GUS IRQ handler

gus_handler_:
_gus_handler:
                pushad
                push    DS
                mov     AX, [CS:_judas_ds]
                mov     DS, AX
gus_irqloop:    mov     EDX, [_judas_port]
                add     EDX, GF1_IRQ_STAT
                in      AL, DX
                test    AL, DMA_TC_IRQ
                jz      near gus_irqdone
                mov     EDX, [_judas_port]              ;Acknowledge the DMA
                add     EDX, GF1_REG_SELECT             ;interrupt
                mov     AL, DMA_CONTROL
                out     DX, AL
                mov     EDX, [_judas_port]
                add     EDX, GF1_DATA_HI
                in      AL, DX
                dec     byte [gus_dmainprogress]
                mov     ESI, offset gdc
                mov     ECX, CACHESLOTS
gusirq_seekslot:cmp     dword [ESI + GDC_Length], 0
                jnz     gusirq_slotfound
                add     ESI, CACHESLOT_size             ;type CACHESLOT in TASM
                dec     ECX
                jnz     gusirq_seekslot
                jmp     gus_irqloop
gusirq_slotfound:
                mov     EBX, [ESI + GDC_Pos]            ;DMA offset
                shr     EBX, 4
                mov     CL, DMA_ENABLE | DMA_R0 | DMA_TWOS_COMP | DMA_IRQ_ENABLE
                test    byte [_judas_mixmode], SIXTEENBIT
                jz      gus_dma_eight2
                mov     CL, DMA_ENABLE | DMA_R0 | DMA_DATA_16 | DMA_IRQ_ENABLE
gus_dma_eight2: cmp     dword [_judas_dma], 4
                jb      gus_nohighdma2
                or      CL, DMA_WIDTH_16
                shr     EBX, 1
gus_nohighdma2: mov     EDX, [_judas_port]
                add     EDX, GF1_REG_SELECT
                mov     AL, SET_DMA_ADDRESS
                out     DX, AL
                mov     EDX, [_judas_port]
                add     EDX, GF1_DATA_LOW
                mov     AX, BX
                out     DX, AX
                mov     EDX, [_judas_port]
                add     EDX, GF1_REG_SELECT
                mov     AL, DMA_CONTROL
                out     DX, AL
                mov     EDX, [_judas_port]
                add     EDX, GF1_DATA_HI
                mov     AL, CL
                out     DX, AL
                mov     EBX, [ESI + GDC_Length]
                mov     dword [ESI + GDC_Length], 0
                mov     EDX, [ESI + GDC_Pos]            ;DMA offset
                mov     EAX, 48h                        ;DMA mode
                call    dma_program_                    ;Program it!
                jmp     gus_irqloop
gus_irqdone:    sti
                call    send_eoi
                pop     DS
                popad
                iretd

        ;Various GUS functions

gus_peek_:
_gus_peek:
                push    EBX
                mov     EBX, EAX
                mov     AL, SET_DRAM_LOW
                mov     EDX, [_judas_port]
                add     EDX, GF1_REG_SELECT
                out     DX, AL
                mov     AX, BX
                mov     EDX, [_judas_port]
                add     EDX, GF1_DATA_LOW
                out     DX, AX
                mov     AL, SET_DRAM_HIGH
                mov     EDX, [_judas_port]
                add     EDX, GF1_REG_SELECT
                out     DX, AL
                shr     EBX, 16
                mov     AL, BL
                mov     EDX, [_judas_port]
                add     EDX, GF1_DATA_HI
                out     DX, AL
                mov     EDX, [_judas_port]
                add     EDX, GF1_DRAM
                in      AL, DX
                pop     EBX
                ret

gus_poke_:
_gus_poke:
                push    EBX
                push    EDX
                mov     EBX, EAX
                mov     AL, SET_DRAM_LOW
                mov     EDX, [_judas_port]
                add     EDX, GF1_REG_SELECT
                out     DX, AL
                mov     AX, BX
                mov     EDX, [_judas_port]
                add     EDX, GF1_DATA_LOW
                out     DX, AX
                mov     AL, SET_DRAM_HIGH
                mov     EDX, [_judas_port]
                add     EDX, GF1_REG_SELECT
                out     DX, AL
                shr     EBX, 16
                mov     AL, BL
                mov     EDX, [_judas_port]
                add     EDX, GF1_DATA_HI
                out     DX, AL
                mov     EDX, [_judas_port]
                add     EDX, GF1_DRAM
                pop     EAX
                out     DX, AL
                pop     EBX
                ret

gus_startchannels_:
_gus_startchannels:
                push    EBX                             ;This routine starts
                push    ECX                             ;the two channels
                push    EDX                             ;as quickly as possible.
                mov     EBX, [_judas_port]
                add     EBX, GF1_PAGE
                mov     ECX, [_judas_port]
                add     ECX, GF1_DATA_HI
                mov     EDX, [_judas_port]
                add     EDX, GF1_REG_SELECT
                mov     AL, SET_CONTROL
                out     DX, AL
                test    byte [_judas_mixmode], SIXTEENBIT
                jz      gus_start8
                mov     EDX, EBX
                mov     AL, 0
                out     DX, AL
                mov     EDX, ECX
                mov     AL, VC_LOOP_ENABLE | VC_DATA_TYPE
                out     DX, AL
                mov     EDX, EBX
                mov     AL, 1
                out     DX, AL
                mov     EDX, ECX
                mov     AL, VC_LOOP_ENABLE | VC_DATA_TYPE
                out     DX, AL
                pop     EDX
                pop     ECX
                pop     EBX
                ret
gus_start8:     mov     EDX, EBX
                xor     AL, AL
                out     DX, AL
                mov     EDX, ECX
                mov     AL, VC_LOOP_ENABLE
                out     DX, AL
                mov     EDX, EBX
                mov     AL, 1
                out     DX, AL
                mov     EDX, ECX
                mov     AL, VC_LOOP_ENABLE
                out     DX, AL
                pop     EDX
                pop     ECX
                pop     EBX
                ret

gus_dmaprogram_:
_gus_dmaprogram:
                or      EDX, EDX                        ;Zero length fucks up!
                jz      gus_skipdma
                pushad
                cli
                cmp     byte [gus_dmainprogress], 0     ;Do we have to cache the
                je      gus_dontcache                   ;block?
                mov     EBX, offset gdc
                mov     ECX, CACHESLOTS
gus_seekslot:   cmp     dword [EBX + GDC_Length], 0
                je      gus_slotfound
                add     EBX, CACHESLOT_size             ;type CACHESLOT in TASM
                dec     ECX
                jnz     gus_seekslot
                sti
                popad
gus_skipdma:    ret
gus_slotfound:  mov     [EBX + GDC_Pos], EAX
                mov     [EBX + GDC_Length], EDX
                inc     byte [gus_dmainprogress]
                sti
                popad
                ret
gus_dontcache:  sti
                inc     byte [gus_dmainprogress]
                mov     ESI, EAX
                mov     EDI, EDX
                mov     EBX, ESI                        ;DMA offset
                shr     EBX, 4
                mov     CL, DMA_ENABLE | DMA_R0 | DMA_TWOS_COMP | DMA_IRQ_ENABLE
                test    byte [_judas_mixmode], SIXTEENBIT
                jz      gus_dma_eight
                mov     CL, DMA_ENABLE | DMA_R0 | DMA_DATA_16 | DMA_IRQ_ENABLE
gus_dma_eight:  cmp     dword [_judas_dma], 4
                jb      gus_nohighdma
                or      CL, DMA_WIDTH_16
                shr     EBX, 1
gus_nohighdma:  mov     EDX, [_judas_port]
                add     EDX, GF1_REG_SELECT
                mov     AL, SET_DMA_ADDRESS
                out     DX, AL
                mov     EDX, [_judas_port]
                add     EDX, GF1_DATA_LOW
                mov     EAX, EBX
                out     DX, AX
                mov     EDX, [_judas_port]
                add     EDX, GF1_REG_SELECT
                mov     AL, DMA_CONTROL
                out     DX, AL
                mov     EDX, [_judas_port]
                add     EDX, GF1_DATA_HI
                mov     AL, CL
                out     DX, AL
                mov     EBX, EDI                        ;DMA length
                mov     EDX, ESI                        ;DMA offset
                mov     EAX, 48h                        ;DMA mode
                call    dma_program_                    ;Program it!
                popad
                ret

gus_dmainit_:
_gus_dmainit:
                cli
                mov     byte [gus_dmainprogress], 0
                push    EAX
                push    EDX
                mov     EDX, [_judas_port]              ;Acknowledge the DMA
                add     EDX, GF1_REG_SELECT             ;interrupt
                mov     AL, DMA_CONTROL
                out     DX, AL
                mov     EDX, [_judas_port]
                add     EDX, GF1_DATA_HI
                in      AL, DX
                mov     EAX, offset gdc
diloop:         mov     dword [EAX + GDC_Pos], 0
                mov     dword [EAX + GDC_Length], 0
                add     EAX, CACHESLOT_size                           ;type CACHESLOT
                cmp     EAX, offset gdc + CACHESLOTS * CACHESLOT_size ;type CACHESLOT in TASM
                jne     diloop
                pop     EDX
                pop     EAX
                sti
                ret

gus_dmawait_:
_gus_dmawait:
                mov     EAX, 200000h                    ;Timeout counter
gus_dmawaitloop:cmp     byte [gus_dmainprogress], 0     ;(might time out if
                je      gus_dmadone                     ;there is a DMA
                dec     EAX                             ;conflict.) This routine
                jnz     gus_dmawaitloop                 ;is used just for click
gus_dmadone:    ret                                     ;removal!

gus_getpos:     push    EBX
                push    EDX
                mov     EDX, [_judas_port]              ;Get the channel
                add     EDX, GF1_PAGE                   ;playing position to
                xor     AL, AL                          ;know where we'll mix
                out     DX, AL
                mov     EDX, [_judas_port]
                add     EDX, GF1_REG_SELECT
                mov     AL, GET_ACC_HIGH
                out     DX, AL
                mov     EDX, [_judas_port]
                add     EDX, GF1_DATA_LOW
                in      AX, DX
                and     EAX, 8191
                shl     EAX, 7
                mov     EBX, EAX
                mov     EDX, [_judas_port]
                add     EDX, GF1_REG_SELECT
                mov     AL, GET_ACC_LOW
                out     DX, AL
                mov     EDX, [_judas_port]
                add     EDX, GF1_DATA_LOW
                in      AX, DX
                shr     AX, 9
                or      EAX, EBX
                test    byte [_judas_mixmode], SIXTEENBIT
                jz      ggp_not16
                shl     EAX, 1
ggp_not16:      pop     EDX
                pop     EBX
                ret


;*****************************************************************************
;               Intel ICH AC97 stuff
;*****************************************************************************
; When CIV == LVI, set LVI <> CIV to never run out of buffers to play.
ac97_updateLVI:
                push    eax
                push    edx
                cmp     dword [_audio_pci + AUDIO_PCI_DEV.mem_mode], 0   ; memory mapped IO?
                jne     ac97_updateLVI_mem

                mov     edx, [_judas_port]
                add     edx, PO_CIV_REG                                ; PCM OUT Current Index Value
                in      ax, dx                                         ; and Last Valid Index
                and     al, 01fh                                       ; bits 0-5 only (important for SIS)
                and     ah, 01fh                                       ; bits 0-5 only (important for SIS)
                cmp     al, ah                                         ; CIV == LVI?
                jnz     ac97_updateLVI_ok                              ; no, don't change LVI
                call    ac97_setNewIndex                               ; set LVI to something else
                jmp     short ac97_updateLVI_ok
ac97_updateLVI_mem:
                mov     edx, [_audio_pci + AUDIO_PCI_DEV.base3]          ; NABMBAR for memory mapped IO
                add     edx, PO_CIV_REG                                ; PCM OUT Current Index Value
                mov     ax, [edx]                                      ; and Last Valid Index
                and     al, 01fh                                       ; bits 0-5 only (important for SIS)
                and     ah, 01fh                                       ; bits 0-5 only (important for SIS)
                cmp     al, ah                                         ; CIV == LVI?
                jnz     ac97_updateLVI_ok                              ; no, don't change LVI
                call    ac97_setNewIndex                               ; set LVI to something else
ac97_updateLVI_ok:
                pop     edx
                pop     eax
                ret

; Set the Last Valid Index to 1 less than the Current Index Value,
; so that we never run out of buffers.
ac97_setNewIndex:
                push    eax
                call    ac97_getCurrentIndex                           ; get CIV
                dec     al                                             ; make LVI != CIV
                and     al, INDEX_MASK                                 ; make sure new value is 0-31
                call    ac97_setLastValidIndex                         ; write new LVI
                pop     eax
                ret

; return AL = PCM OUT Current Index Value
ac97_getCurrentIndex:
                push    edx
                cmp     dword [_audio_pci + AUDIO_PCI_DEV.mem_mode], 0   ; memory mapped IO?
                jne     ac97_getCurrentIndex_mem

                mov     edx, [_judas_port]
                add     edx, PO_CIV_REG
                in      al, dx
                jmp     short ac97_getCurrentIndex_ok
ac97_getCurrentIndex_mem:
                mov     edx, [_audio_pci + AUDIO_PCI_DEV.base3]          ; NABMBAR for memory mapped IO
                add     edx, PO_CIV_REG                                ; PCM OUT Current Index Value
                mov     ax, [edx]
ac97_getCurrentIndex_ok:
                pop     edx
                ret

; input AL = PCM OUT Last Valid Index (index to stop on)
ac97_setLastValidIndex:
                push    edx
                cmp     dword [_audio_pci + AUDIO_PCI_DEV.mem_mode], 0   ; memory mapped IO?
                jne     ac97_setLastValidIndex_mem

                mov     edx, [_judas_port]
                add     edx, PO_LVI_REG
                out     dx, al
                jmp     short ac97_setLastValidIndex_ok
ac97_setLastValidIndex_mem:
                mov     edx, [_audio_pci + AUDIO_PCI_DEV.base3]          ; NABMBAR for memory mapped IO
                add     edx, PO_LVI_REG
                mov     [edx], al                                      ; and Last Valid Index
ac97_setLastValidIndex_ok:
                pop     edx
                ret


;*****************************************************************************
;               Intel HDA stuff
;*****************************************************************************
hda_get_lpib:

                push    edx

                mov     edx, dword [_audio_pci + AUDIO_PCI_DEV.base0]
                add     edx, HDA_SDO0LPIB
                mov     eax, [edx]

                pop     edx
                ret

hda_dma_start:
                push    edx

                mov     edx, dword [_audio_pci + AUDIO_PCI_DEV.base0]
                add     edx, HDA_SDO0CTL
                mov     eax, [edx]
                or      eax, SD_CTL_DMA_START
                mov     [edx], eax

                pop     edx
                ret


        ;General DMAbuffer update routine (call either from main program or
        ;within your timer interrupt)

judas_update_:
_judas_update:
                cmp     dword [_judas_device], DEV_NOSOUND
                je      near judas_gotohell
                cmp     dword [_judas_device], DEV_FILE
                je      near judas_gotohell
                cmp     byte [mix_exec], 0
                jne     near judas_gotohell
                cmp     byte [_judas_initialized], 0
                je      near judas_gotohell
                inc     byte [mix_exec]
                pushad
                cmp     dword [_judas_device], DEV_GUS
                je      near updategus                  ;This is a different story
                cmp     dword [_judas_device], DEV_AC97
                je      near updateac97                 ; audio_pci update for AC97
                cmp     dword [_judas_device], DEV_HDA
                je      near updatehda                  ; audio_pci update for HDA
                call    dma_query_
                                                        ;Must be aligned on 8
                and     EAX, [_judas_buffermask]        ;samples (unrolling!)
                mov     EBX, [_judas_mixpos]            ;This is the old pos
                cmp     EAX, EBX
                je      judas_donothing
                jb      judas_wrap
judas_normal:   mov     [_judas_mixpos], EAX
                mov     EDX, EAX
                sub     EDX, EBX                        ;EDX = length to mix
                mov     EAX, EBX                        ;EAX = pos. to mix
                add     EAX, [_dma_address]
                call    dword [_judas_mixersys]
judas_donothing:popad
                dec     byte [mix_exec]
judas_gotohell: ret
judas_wrap:     mov     [_judas_mixpos], EAX
                mov     EAX, EBX                        ;Mix to buffer end
                mov     EDX, [_judas_bufferlength]
                sub     EDX, EBX
                add     EAX, [_dma_address]
                call    dword [_judas_mixersys]
                mov     EAX, [_dma_address]             ;Then to start
                mov     EDX, [_judas_mixpos]
                or      EDX, EDX
                jz      judas_donothing
                call    dword [_judas_mixersys]
                jmp     judas_donothing


updateac97:     call    ac97_updateLVI                 ; set CIV != LVI
                call    ac97_getCurrentIndex
update_hda_buffers:
                test    al, 1                          ; check parity
                jz      ac97_playing_buffer0

                ; playing buffer 1 -> refresh buffer 0 (Bus Master DMA)
ac97_playing_buffer1:
                cmp     byte [ac97_buffer0_set], 1     ; is buffer 0
                je      judas_donothing                ; already refreshed
                mov     eax, [_audio_pci + AUDIO_PCI_DEV.pcmout_buffer0]           ; buffer 0 address
                mov     edx, [_judas_bufferlength]     ; buffer 0 size
                call    dword [_judas_mixersys]
                mov     byte [ac97_buffer0_set], 1     ; set buffer 0
                mov     byte [ac97_buffer1_set], 0     ; as refreshed
                jmp     judas_donothing

                ; playing buffer 0 -> refresh buffer 1 (Bus Master DMA)
ac97_playing_buffer0:
                cmp     byte [ac97_buffer1_set], 1     ; is buffer 1
                je      near judas_donothing           ; already refreshed
                mov     eax, [_audio_pci + AUDIO_PCI_DEV.pcmout_buffer1]           ; buffer 1 address
                mov     edx, [_judas_bufferlength]     ; buffer 1 size
                call    dword [_judas_mixersys]
                mov     byte [ac97_buffer1_set], 1     ; set buffer 1
                mov     byte [ac97_buffer0_set], 0     ; as refreshed
                jmp     judas_donothing


updatehda:      mov     eax, [_hda_lpib]
                or      eax, eax
                jnz     hda_update_civ
                mov     eax, [_hda_civ]
                or      eax, eax
                jnz     hda_update_civ
                call    hda_dma_start                  ; 1st time run, start the DMA engine

hda_update_civ:
                call    hda_get_lpib                   ; get LPIB
                cmp     eax, dword [_hda_lpib]         ; compare wih last LPIB position
                jae     hda_skip_civ                   ; if no wrap around don't update CIV

                inc     dword [_hda_civ]
                cmp     dword [_hda_civ], 32
                jne     hda_skip_civ
                mov     dword [_hda_civ], 0

hda_skip_civ:
                mov     [_hda_lpib], eax
                mov     eax, [_hda_civ]
                jmp     update_hda_buffers             ; same as AC97 on next step


updategus:      cmp     byte [gus_dmainprogress], CACHESLOTS - 4 ;Is there too many
                jb      ug_notstuck                    ;DMA's waiting? I mean,
                call    gus_dmainit_                   ;maybe WIN95 has stuck
                                                       ;the card somehow. In
                                                       ;that case, release all
                                                       ;waiting DMAs manually!
ug_notstuck:    cli
                test    byte [_judas_mixmode], STEREO
                jz      near updategus_mono
ipc_s:          xor     EAX, EAX
                call    gus_peek_
                mov     DL, AL
                mov     EAX, [_judas_bufferlength]
                shr     EAX, 1
                call    gus_poke_
                mov     EAX, 1
                call    gus_peek_
                mov     DL, AL
                mov     EAX, [_judas_bufferlength]
                shr     EAX, 1
                inc     EAX
                call    gus_poke_
                mov     EAX, [_judas_bufferlength]
                shr     EAX, 1
                add     EAX, 32
                call    gus_peek_
                mov     DL, AL
                mov     EAX, [_judas_bufferlength]
                add     EAX, 32
                call    gus_poke_
                mov     EAX, [_judas_bufferlength]
                shr     EAX, 1
                add     EAX, 33
                call    gus_peek_
                mov     DL, AL
                mov     EAX, [_judas_bufferlength]
                add     EAX, 33
                call    gus_poke_
                mov     EDX, [_judas_bufferlength]
                shr     EDX, 1
ugs_shitloop:   call    gus_getpos
                mov     EBX, EAX
                call    gus_getpos
                mov     ECX, EAX
                cmp     EBX, EDX
                jae     ugs_shitloop
                mov     EAX, EBX
                sub     EAX, ECX
                cmp     EAX, 64
                jg      ugs_shitloop
                cmp     EAX, -64
                jl      ugs_shitloop
                mov     EAX, EBX
                sti
                and     EAX, [_judas_buffermask]
                mov     EBX, [_judas_mixpos]            ;EBX = old mixpos
                cmp     EAX, EBX
                je      near judas_donothing
                jb      updategus_wrap
                mov     [_judas_mixpos], EAX            ;New "oldpos"
                mov     EDX, EAX
                sub     EDX, EBX                        ;How much to mix
                mov     EAX, EBX                        ;Where to mix
                push    EAX
                push    EDX
                shl     EDX, 1
                add     EAX, [_dma_address]
                call    dword [_judas_mixersys]
                pop     EDX
                pop     EAX
                call    gus_dmaprogram_
                add     EAX, 32
                mov     EBX, [_judas_bufferlength]
                shr     EBX, 1
                add     EAX, EBX
                call    gus_dmaprogram_
                jmp     judas_donothing
updategus_wrap: mov     [_judas_mixpos], EAX            ;Mix first to buffer
                mov     EAX, EBX                        ;end, then to start
                mov     EDX, [_judas_bufferlength]
                shr     EDX, 1
                sub     EDX, EBX
                push    EAX
                push    EDX
                shl     EDX, 1
                add     EAX, [_dma_address]
                call    dword [_judas_mixersys]
                mov     EAX, [_dma_address]
                mov     EDX, [_judas_mixpos]
                shl     EDX, 1
                call    dword [_judas_mixersys]
                pop     EDX
                pop     EAX
                call    gus_dmaprogram_
                add     EAX, 32
                mov     EBX, [_judas_bufferlength]
                shr     EBX, 1
                add     EAX, EBX
                call    gus_dmaprogram_
                xor     EAX, EAX
                mov     EDX, [_judas_mixpos]
                call    gus_dmaprogram_
                add     EAX, 32
                mov     EBX, [_judas_bufferlength]
                shr     EBX, 1
                add     EAX, EBX
                call    gus_dmaprogram_
                jmp     judas_donothing

updategus_mono: xor     EAX, EAX
                call    gus_peek_
                mov     DL, AL
                mov     EAX, [_judas_bufferlength]
                call    gus_poke_
                mov     EAX, 1
                call    gus_peek_
                mov     DL, AL
                mov     EAX, [_judas_bufferlength]
                inc     EAX
                call    gus_poke_
                mov     EDX, [_judas_bufferlength]
ugm_shitloop:   call    gus_getpos
                mov     EBX, EAX
                call    gus_getpos
                mov     ECX, EAX
                cmp     EBX, EDX
                jae     ugm_shitloop
                mov     EAX, EBX
                sub     EAX, ECX
                cmp     EAX, 64
                jg      ugm_shitloop
                cmp     EAX, -64
                jl      ugm_shitloop
                mov     EAX, EBX
                sti
                and     EAX, [_judas_buffermask]
                mov     EBX, [_judas_mixpos]            ;EBX = old mixpos
                cmp     EAX, EBX
                je      near judas_donothing
                jb      updategusm_wrap
                mov     [_judas_mixpos], EAX            ;New "oldpos"
                mov     EDX, EAX
                sub     EDX, EBX                        ;How much to mix
                mov     EAX, EBX                        ;Where to mix
                push    EAX
                push    EDX
                add     EAX, [_dma_address]
                call    dword [_judas_mixersys]
                pop     EDX
                pop     EAX
                call    gus_dmaprogram_
                jmp     judas_donothing
updategusm_wrap:mov     [_judas_mixpos], EAX            ;Mix first to buffer
                mov     EAX, EBX                        ;end
                mov     EDX, [_judas_bufferlength]
                sub     EDX, EBX
                push    EAX
                push    EDX
                add     EAX, [_dma_address]
                call    dword [_judas_mixersys]
                mov     EAX, [_dma_address]
                mov     EDX, [_judas_mixpos]
                call    dword [_judas_mixersys]
                pop     EDX
                pop     EAX
                call    gus_dmaprogram_
                xor     EAX, EAX
                mov     EDX, [_judas_mixpos]
                call    gus_dmaprogram_
                jmp     judas_donothing









;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³                                 Fast Mixer                                 ³
;³                                                                            ³
;³                                 by Cadaver                                 ³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

;First macros...

%macro          getvolume 0
                xor     EAX, EAX
                mov     AX, [ESI + Chn_Vol]
                cmp     EAX, 64*256
                jbe     %%limit
                mov     EAX, 64*256
%%limit:        xor     EBX, EBX
                mov     BL, [ESI + Chn_MasterVol]
                imul    EAX, EBX
                shr     EAX, 6+8
                or      AL, AL
                jz      near zerovolume
                mov     EBX, EAX
%endmacro

%macro          stereoadjust 0
                shl     EBX, 1                          ;Stereo can have 2x vol
                cmp     EBX, 255                        ;Check that volume is
                jbe     %%limit                         ;within volumetable
                mov     EBX, 255                        ;limits though
%%limit:
%endmacro

%macro          smoothpanadjust 0
                xor     EAX, EAX
                mov     AL, [ESI + Chn_Panning]
                mov     ECX, EBX
                imul    ECX, EAX
                shr     ECX, 7
                neg     EAX
                add     EAX, 256
                imul    EBX, EAX
                shr     EBX, 7
                cmp     EBX, 255
                jbe     %%limit1
                mov     EBX, 255
%%limit1:       cmp     ECX, 255
                jbe     %%limit2
                mov     ECX, 255
%%limit2:
%endmacro

%macro          mix8_mono_n 1
                mov     BL, [ESI]
                add     EDX, ECX
                mov     EAX, [EBX * 4]
                adc     ESI, EBP
                add     [EDI + 4 * %1], EAX
%endmacro

%macro          mix8_left_n 1
                mov     BL, [ESI]
                add     EDX, ECX
                mov     EAX, [EBX * 4]
                adc     ESI, EBP
                add     [EDI + 8 * %1], EAX
%endmacro

%macro          mix8_middle_n 1
                mov     BL, [ESI]
                add     EDX, ECX
                mov     EAX, [EBX * 4]
                adc     ESI, EBP
                add     [EDI + 8 * %1], EAX
                add     [EDI + 8 * %1 + 4], EAX
%endmacro

%macro          mix8_right_n 1
                mov     BL, [ESI]
                add     EDX, ECX
                mov     EAX, [EBX * 4]
                adc     ESI, EBP
                add     [EDI + 8 * %1 + 4], EAX
%endmacro

%macro          mix8_smooth_n 1
                mov     BL, [ESI]
                add     EDX, [fractadd]
                mov     EAX, [EBX * 4]
                adc     ESI, EBP
                mov     CL, BL
                add     [EDI + 8 * %1], EAX
                mov     EAX, [ECX * 4]
                add     [EDI + 8 * %1 + 4], EAX
%endmacro

%macro          mix8_mono_i 1
                movsx   EAX, byte [ESI+1]
                movsx   ECX, byte [ESI]
                sub     EAX, ECX
                movzx   ECX, DH
                imul    AX, CX
                mov     BL, AH
                add     BL, [ESI]
                add     DX, word [fractadd + 2]
                mov     EAX, [EBX * 4]
                adc     ESI, EBP
                add     [EDI + 4 * %1], EAX
%endmacro

%macro          mix8_left_i 1
                movsx   EAX, byte [ESI+1]
                movsx   ECX, byte [ESI]
                sub     EAX, ECX
                movzx   ECX, DH
                imul    AX, CX
                mov     BL, AH
                add     BL, [ESI]
                add     DX, word [fractadd + 2]
                mov     EAX, [EBX * 4]
                adc     ESI, EBP
                add     [EDI + 8 * %1], EAX
%endmacro

%macro          mix8_middle_i 1
                movsx   EAX, byte [ESI+1]
                movsx   ECX, byte [ESI]
                sub     EAX, ECX
                movzx   ECX, DH
                imul    AX, CX
                mov     BL, AH
                add     BL, [ESI]
                add     DX, word [fractadd + 2]
                mov     EAX, [EBX * 4]
                adc     ESI, EBP
                add     [EDI + 8 * %1], EAX
                add     [EDI + 8 * %1 + 4], EAX
%endmacro

%macro          mix8_right_i 1
                movsx   EAX, byte [ESI+1]
                movsx   ECX, byte [ESI]
                sub     EAX, ECX
                movzx   ECX, DH
                imul    AX, CX
                mov     BL, AH
                add     BL, [ESI]
                add     DX, word [fractadd + 2]
                mov     EAX, [EBX * 4]
                adc     ESI, EBP
                add     [EDI + 8 * %1 + 4], EAX
%endmacro

%macro          mix8_smooth_i 1
                movsx   EAX, byte [ESI+1]
                movsx   EBP, byte [ESI]
                sub     EAX, EBP
                movzx   EBP, DH
                imul    AX, BP
                mov     BL, AH
                add     BL, [ESI]
                add     DX, word [fractadd + 2]
                mov     EAX, [EBX * 4]
                adc     ESI, [integeradd]
                mov     CL, BL
                add     [EDI + 8 * %1], EAX
                mov     EAX, [ECX * 4]
                add     [EDI + 8 * %1 + 4], EAX
%endmacro

%macro          mix16_mono_n 1
                movsx   EAX, word [ESI * 2]
                imul    EAX, EBX
                sar     EAX, 8 + (16 - SIGNIFICANT_BITS_16)
                add     EDX, ECX
                adc     ESI, EBP
                add     [EDI + 4 * %1], EAX
%endmacro

%macro          mix16_left_n 1
                movsx   EAX, word [ESI * 2]
                imul    EAX, EBX
                sar     EAX, 8 + (16 - SIGNIFICANT_BITS_16)
                add     EDX, ECX
                adc     ESI, EBP
                add     [EDI + 8 * %1], EAX
%endmacro

%macro          mix16_middle_n 1
                movsx   EAX, word [ESI * 2]
                imul    EAX, EBX
                sar     EAX, 8 + (16 - SIGNIFICANT_BITS_16)
                add     EDX, ECX
                adc     ESI, EBP
                add     [EDI + 8 * %1], EAX
                add     [EDI + 8 * %1 + 4], EAX
%endmacro

%macro          mix16_right_n 1
                movsx   EAX, word [ESI * 2]
                imul    EAX, EBX
                sar     EAX, 8 + (16 - SIGNIFICANT_BITS_16)
                add     EDX, ECX
                adc     ESI, EBP
                add     [EDI + 8 * %1 + 4], EAX
%endmacro

%macro          mix16_smooth_n 1
                movsx   EAX, word [ESI * 2]
                imul    EAX, EBX
                sar     EAX, 8 + (16 - SIGNIFICANT_BITS_16)
                add     [EDI + 8 * %1], EAX
                movsx   EAX, word [ESI * 2]
                imul    EAX, ECX
                sar     EAX, 8 + (16 - SIGNIFICANT_BITS_16)
                add     EDX, [fractadd]
                adc     ESI, EBP
                add     [EDI + 8 * %1 + 4], EAX
%endmacro

%macro          mix16_mono_i 1
                movsx   EAX, word [ESI * 2 + 2]
                movsx   ECX, word [ESI * 2]
                sub     EAX, ECX
                movzx   EBX, DH
                imul    EAX, EBX
                sar     EAX, 8
                add     EAX, ECX
                imul    EAX, [leftvol]
                sar     EAX, 8 + (16 - SIGNIFICANT_BITS_16)
                add     DX, word [fractadd + 2]
                adc     ESI, EBP
                add     [EDI + 4 * %1], EAX
%endmacro

%macro          mix16_left_i 1
                movsx   EAX, word [ESI * 2 + 2]
                movsx   ECX, word [ESI * 2]
                sub     EAX, ECX
                movzx   EBX, DH
                imul    EAX, EBX
                sar     EAX, 8
                add     EAX, ECX
                imul    EAX, [leftvol]
                sar     EAX, 8 + (16 - SIGNIFICANT_BITS_16)
                add     DX, word [fractadd + 2]
                adc     ESI, EBP
                add     [EDI + 8 * %1], EAX
%endmacro

%macro          mix16_middle_i 1
                movsx   EAX, word [ESI * 2 + 2]
                movsx   ECX, word [ESI * 2]
                sub     EAX, ECX
                movzx   EBX, DH
                imul    EAX, EBX
                sar     EAX, 8
                add     EAX, ECX
                imul    EAX, [leftvol]
                sar     EAX, 8 + (16 - SIGNIFICANT_BITS_16)
                add     DX, word [fractadd + 2]
                adc     ESI, EBP
                add     [EDI + 8 * %1], EAX
                add     [EDI + 8 * %1 + 4], EAX
%endmacro

%macro          mix16_right_i 1
                movsx   EAX, word [ESI * 2 + 2]
                movsx   ECX, word [ESI * 2]
                sub     EAX, ECX
                movzx   EBX, DH
                imul    EAX, EBX
                sar     EAX, 8
                add     EAX, ECX
                imul    EAX, [leftvol]
                sar     EAX, 8 + (16 - SIGNIFICANT_BITS_16)
                add     DX, word [fractadd + 2]
                adc     ESI, EBP
                add     [EDI + 8 * %1 + 4], EAX
%endmacro

%macro          mix16_smooth_i 1
                movsx   EAX, word [ESI * 2 + 2]
                movsx   ECX, word [ESI * 2]
                sub     EAX, ECX
                movzx   EBX, DH
                imul    EAX, EBX
                sar     EAX, 8
                add     EAX, ECX
                mov     EBX, EAX
                imul    EAX, [leftvol]
                sar     EAX, 8 + (16 - SIGNIFICANT_BITS_16)
                add     [EDI + 8 * %1], EAX
                mov     EAX, EBX
                imul    EAX, [rightvol]
                sar     EAX, 8 + (16 - SIGNIFICANT_BITS_16)
                add     DX, word [fractadd + 2]
                adc     ESI, EBP
                add     [EDI + 8 * %1 + 4], EAX
%endmacro

%macro          mixloop8 3                              ; routine, samplesize, ip
                mov     DX, [ESI + Chn_FractPos]        ;Get fractional pos
                %if (%3 == 0)                           ; if (ip == 0)
                shl     EDX, 16
                %endif
                mov     EAX, [ESI + Chn_End]            ;Get end & endsubtract-
                mov     [smpend], EAX                   ;value
                sub     EAX, [ESI + Chn_Repeat]
                mov     [smpsubtract], EAX
                mov     ESI, [ESI + Chn_Pos]            ;Get sample position
                mov     EDI, [dptr]                     ;Get bufferptr
                mov     EAX, [samples]                  ;Fix loopcount &
                dec     EAX
                shr     EAX, 4                          ;jumpoffset & subtract
                inc     EAX                             ;EDI accordingly
                mov     [loopcount], EAX
                mov     EAX, [samples]
                and     EAX, 15
                mov     EAX, [EAX * 4 + shittable]
                %if (%2 == 4)                           ; if (samplesize == 4)
                sub     EDI, EAX
                %else
                sub     EDI, EAX
                sub     EDI, EAX
                %endif
                add     EAX, offset %%offsettable
                jmp     [EAX]

                align 16

%%offset0:
                %1 0 ;routine 0
%%offset1:
                %1 1 ;routine 1
%%offset2:
                %1 2 ;routine 2
%%offset3:
                %1 3 ;routine 3
%%offset4:
                %1 4 ;routine 4
%%offset5:
                %1 5 ;routine 5
%%offset6:
                %1 6 ;routine 6
%%offset7:
                %1 7 ;routine 7
%%offset8:
                %1 8 ;routine 8
%%offset9:
                %1 9 ;routine 9
%%offseta:
                %1 10 ;routine 10
%%offsetb:
                %1 11 ;routine 11
%%offsetc:
                %1 12 ;routine 12
%%offsetd:
                %1 13 ;routine 13
%%offsete:
                %1 14 ;routine 14
%%offsetf:
                %1 15 ;routine 15
                add     EDI, 16 * %2                    ; samplesize
                cmp     ESI, [smpend]
                jae     %%hitend
                dec     dword [loopcount]
                jnz     near %%offset0
                mov     EAX, [cptr]
                mov     [EAX + Chn_Pos], ESI
                %if (%3 == 0)                           ; if (ip == 0)
                shr     EDX, 16
                %endif
                mov     [EAX + Chn_FractPos], DX
                ret
%%hitend:       mov     EAX, [cptr]
                test    byte [EAX + Chn_VoiceMode], VM_LOOP
                jz      %%oneshot
%%subloop:      sub     ESI, [smpsubtract]
                cmp     ESI, [smpend]
                jae     %%subloop
                dec     dword [loopcount]
                jnz     near %%offset0
                mov     [EAX + Chn_Pos], ESI
                %if (%3 == 0)                           ;if (ip == 0)
                shr     EDX, 16
                %endif
                mov     [EAX + Chn_FractPos], DX
                ret
%%oneshot:      mov     byte [EAX + Chn_VoiceMode], VM_OFF
                ret
                align   4
%%offsettable:  dd      offset %%offset0
                dd      offset %%offset1
                dd      offset %%offset2
                dd      offset %%offset3
                dd      offset %%offset4
                dd      offset %%offset5
                dd      offset %%offset6
                dd      offset %%offset7
                dd      offset %%offset8
                dd      offset %%offset9
                dd      offset %%offseta
                dd      offset %%offsetb
                dd      offset %%offsetc
                dd      offset %%offsetd
                dd      offset %%offsete
                dd      offset %%offsetf
%endmacro

%macro          mixloop16 3                             ; routine, samplesize, ip
                mov     DX, [ESI + Chn_FractPos]        ;Get fractional pos
                %if (%3 == 0)                           ; if (ip == 0)
                shl     EDX, 16
                %endif
                mov     EAX, [ESI + Chn_End]            ;Get end & endsubtract-
                shr     EAX, 1
                mov     [smpend], EAX                   ;value
                mov     EAX, [ESI + Chn_End]
                sub     EAX, [ESI + Chn_Repeat]
                shr     EAX, 1
                mov     [smpsubtract], EAX
                mov     ESI, [ESI + Chn_Pos]            ;Get sample position
                shr     ESI, 1
                mov     EDI, [dptr]                     ;Get bufferptr
                mov     EAX, [samples]                  ;Fix loopcount &
                dec     EAX
                shr     EAX, 4                          ;jumpoffset & subtract
                inc     EAX                             ;EDI accordingly
                mov     [loopcount], EAX
                mov     EAX, [samples]
                and     EAX, 15
                mov     EAX, [EAX * 4 + shittable]
                %if (%2 == 4)                           ; if (samplesize == 4)
                sub     EDI, EAX
                %else
                sub     EDI, EAX
                sub     EDI, EAX
                %endif
                add     EAX, offset %%offsettable
                jmp     [EAX]

                align 16

%%offset0:
                %1 0 ;routine 0
%%offset1:
                %1 1 ;routine 1
%%offset2:
                %1 2 ;routine 2
%%offset3:
                %1 3 ;routine 3
%%offset4:
                %1 4 ;routine 4
%%offset5:
                %1 5 ;routine 5
%%offset6:
                %1 6 ;routine 6
%%offset7:
                %1 7 ;routine 7
%%offset8:
                %1 8 ;routine 8
%%offset9:
                %1 9 ;routine 9
%%offseta:
                %1 10 ;routine 10
%%offsetb:
                %1 11 ;routine 11
%%offsetc:
                %1 12 ;routine 12
%%offsetd:
                %1 13 ;routine 13
%%offsete:
                %1 14 ;routine 14
%%offsetf:
                %1 15 ;routine 15
                add     EDI, 16 * %2                     ; samplesize
                cmp     ESI, [smpend]
                jae     %%hitend
                dec     dword [loopcount]
                jnz     near %%offset0
                mov     EAX, [cptr]
                shl     ESI, 1
                mov     [EAX + Chn_Pos], ESI
                %if (%3 == 0)                            ; if (ip == 0)
                shr     EDX, 16
                %endif
                mov     [EAX + Chn_FractPos], DX
                ret
%%hitend:       mov     EAX, [cptr]
                test    byte [EAX + Chn_VoiceMode], VM_LOOP
                jz      %%oneshot
%%subloop:      sub     ESI, [smpsubtract]
                cmp     ESI, [smpend]
                jae     %%subloop
                dec     dword [loopcount]
                jnz     near %%offset0
                shl     ESI, 1
                mov     [EAX + Chn_Pos], ESI
                %if (%3 == 0)                            ; if (ip == 0)
                shr     EDX, 16
                %endif
                mov     [EAX + Chn_FractPos], DX
                ret
%%oneshot:      mov     byte [EAX + Chn_VoiceMode], VM_OFF
                ret
                align   4
%%offsettable:  dd      offset %%offset0
                dd      offset %%offset1
                dd      offset %%offset2
                dd      offset %%offset3
                dd      offset %%offset4
                dd      offset %%offset5
                dd      offset %%offset6
                dd      offset %%offset7
                dd      offset %%offset8
                dd      offset %%offset9
                dd      offset %%offseta
                dd      offset %%offsetb
                dd      offset %%offsetc
                dd      offset %%offsetd
                dd      offset %%offsete
                dd      offset %%offsetf
%endmacro

        ;16bit fast mixer routines start here!
        ;This is the main mixing routine, which mixes EDX bytes of sound into
        ;address EAX, calling the music player at correct intervals. EDX must
        ;be a multiply of (samplesize * 8) because there is an unrolled
        ;postprocessing loop.
        ;WARNING: This routine destroys every register!

                align   4

fmixer_:
_fmixer:
                or      EDX, EDX                        ;Check zero length
                jz      near mix_quit
                mov     ECX, EDX
                test    byte [_judas_mixmode], STEREO   ;Stereo or mono?
                jz      mix_noshift1
                shr     EDX, 1
mix_noshift1:   test    byte [_judas_mixmode], SIXTEENBIT      ;8- or 16bit?
                jz      mix_noshift2
                shr     EDX, 1
                shr     ECX, 1
mix_noshift2:   mov     [samples], EDX                  ;Save number of samples
                mov     [totalwork], EDX                ;"Total work" counter
                mov     [fptr], EAX                     ;Save final destination
                shr     ECX, 3
                mov     [postproc], ECX                 ;Save clipbuffer size
                mov     EDI, [_judas_clipbuffer]        ;Clear the clipbuffer
                mov     [dptr], EDI
                xor     EAX, EAX
mix_clearloop:  mov     [EDI], EAX
                mov     [EDI + 4], EAX
                mov     [EDI + 8], EAX
                mov     [EDI + 12], EAX
                mov     [EDI + 16], EAX
                mov     [EDI + 20], EAX
                mov     [EDI + 24], EAX
                mov     [EDI + 28], EAX
                add     EDI, 32
                dec     ECX
                jnz     mix_clearloop
                cmp     dword [_judas_player], 0
                jne     mix_hardwayloop
                call    dword [_judas_mixroutine]
                jmp     mix_firstphasedone
mix_hardwayloop:cmp     dword [_judas_bpmcount], 0      ;Time to play?
                jne     mix_skipplaying
                cmp     dword [_judas_player], 0        ;Might change in the
                je      mix_fuckshitup                  ;middle of a loop
                call    dword [_judas_player]
mix_fuckshitup: mov     EAX, [_judas_mixrate]
                mov     EBX, 5
                mul     EBX
                shr     EAX, 1
                xor     EDX, EDX
                movzx   EBX, byte [_judas_bpmtempo]
                div     EBX
                mov     [_judas_bpmcount], EAX
mix_skipplaying:mov     EAX, [totalwork]
                cmp     EAX, [_judas_bpmcount]
                jbe     mix_nolimit
                mov     EAX, [_judas_bpmcount]
mix_nolimit:    mov     [samples], EAX
                call    dword [_judas_mixroutine]
                mov     EAX, [samples]
                sub     [_judas_bpmcount], EAX
                mov     EBX, EAX
                shl     EBX, 2
                test    byte [_judas_mixmode], STEREO
                jz      mix_noshift3
                shl     EBX, 1
mix_noshift3:   add     [dptr], EBX
                sub     [totalwork], EAX
                jnz     near mix_hardwayloop
mix_firstphasedone:
                test    byte [_judas_mixmode], SIXTEENBIT
                jz      near mix_8bit_endphase
mix_16bit_endphase:
                test    byte [_judas_mixmode], STEREO
                jz      mix_nogusshit1
                cmp     dword [_judas_device], DEV_GUS
                je      near mix_gus16_endphase
mix_nogusshit1: mov     EDI, [fptr]
                mov     EBX, [postproc]
                mov     ESI, [_judas_clipbuffer]
                mov     ECX, [_judas_cliptable]
                xor     EAX, EAX
mix_16bit_endphase_loop:
                mov     AX, [ESI]
                mov     AX, [ECX + EAX * 2]
                mov     [EDI], AX
                mov     AX, [ESI + 4]
                mov     AX, [ECX + EAX * 2]
                mov     [EDI + 2], AX
                mov     AX, [ESI + 8]
                mov     AX, [ECX + EAX * 2]
                mov     [EDI + 4], AX
                mov     AX, [ESI + 12]
                mov     AX, [ECX + EAX * 2]
                mov     [EDI + 6], AX
                mov     AX, [ESI + 16]
                mov     AX, [ECX + EAX * 2]
                mov     [EDI + 8], AX
                mov     AX, [ESI + 20]
                mov     AX, [ECX + EAX * 2]
                mov     [EDI + 10], AX
                mov     AX, [ESI + 24]
                mov     AX, [ECX + EAX * 2]
                mov     [EDI + 12], AX
                mov     AX, [ESI + 28]
                mov     AX, [ECX + EAX * 2]
                mov     [EDI + 14], AX
                add     ESI, 32
                add     EDI, 16
                dec     EBX
                jnz     mix_16bit_endphase_loop
mix_quit:       ret
mix_8bit_endphase:
                test    byte [_judas_mixmode], STEREO
                jz      mix_nogusshit2
                cmp     dword [_judas_device], DEV_GUS
                je      near mix_gus8_endphase
mix_nogusshit2: mov     EDI, [fptr]
                mov     EBX, [postproc]
                mov     ESI, [_judas_clipbuffer]
                mov     ECX, [_judas_cliptable]
                xor     EAX, EAX
mix_8bit_endphase_loop:
                mov     AX, [ESI]
                mov     AL, [ECX + EAX]
                mov     [EDI], AL
                mov     AX, [ESI + 4]
                mov     AL, [ECX + EAX]
                mov     [EDI + 1], AL
                mov     AX, [ESI + 8]
                mov     AL, [ECX + EAX]
                mov     [EDI + 2], AL
                mov     AX, [ESI + 12]
                mov     AL, [ECX + EAX]
                mov     [EDI + 3], AL
                mov     AX, [ESI + 16]
                mov     AL, [ECX + EAX]
                mov     [EDI + 4], AL
                mov     AX, [ESI + 20]
                mov     AL, [ECX + EAX]
                mov     [EDI + 5], AL
                mov     AX, [ESI + 24]
                mov     AL, [ECX + EAX]
                mov     [EDI + 6], AL
                mov     AX, [ESI + 28]
                mov     AL, [ECX + EAX]
                mov     [EDI + 7], AL
                add     ESI, 32
                add     EDI, 8
                dec     EBX
                jnz     mix_8bit_endphase_loop
                jmp     mix_quit
mix_gus16_endphase:
                mov     EDI, [fptr]
                mov     EBX, [postproc]
                mov     EDX, [_judas_bufferlength]
                shr     EDX, 1
                add     EDX, EDI
                add     EDX, 32
                mov     ESI, [_judas_clipbuffer]
                mov     ECX, [_judas_cliptable]
                xor     EAX, EAX
mix_gus16_endphase_loop:
                mov     AX, [ESI]
                mov     AX, [ECX + EAX * 2]
                mov     [EDI], AX
                mov     AX, [ESI + 4]
                mov     AX, [ECX + EAX * 2]
                mov     [EDX], AX
                mov     AX, [ESI + 8]
                mov     AX, [ECX + EAX * 2]
                mov     [EDI + 2], AX
                mov     AX, [ESI + 12]
                mov     AX, [ECX + EAX * 2]
                mov     [EDX + 2], AX
                mov     AX, [ESI + 16]
                mov     AX, [ECX + EAX * 2]
                mov     [EDI + 4], AX
                mov     AX, [ESI + 20]
                mov     AX, [ECX + EAX * 2]
                mov     [EDX + 4], AX
                mov     AX, [ESI + 24]
                mov     AX, [ECX + EAX * 2]
                mov     [EDI + 6], AX
                mov     AX, [ESI + 28]
                mov     AX, [ECX + EAX * 2]
                mov     [EDX + 6], AX
                add     ESI, 32
                add     EDI, 8
                add     EDX, 8
                dec     EBX
                jnz     mix_gus16_endphase_loop
                jmp     mix_quit
mix_gus8_endphase:
                mov     EDI, [fptr]
                mov     EBX, [postproc]
                mov     EDX, [_judas_bufferlength]
                shr     EDX, 1
                add     EDX, EDI
                add     EDX, 32
                mov     ESI, [_judas_clipbuffer]
                mov     ECX, [_judas_cliptable]
                xor     EAX, EAX
mix_gus8_endphase_loop:
                mov     AX, [ESI]
                mov     AL, [ECX + EAX]
                mov     [EDI], AX
                mov     AX, [ESI + 4]
                mov     AL, [ECX + EAX]
                mov     [EDX], AX
                mov     AX, [ESI + 8]
                mov     AL, [ECX + EAX]
                mov     [EDI + 1], AX
                mov     AX, [ESI + 12]
                mov     AL, [ECX + EAX]
                mov     [EDX + 1], AX
                mov     AX, [ESI + 16]
                mov     AX, [ECX + EAX]
                mov     [EDI + 2], AX
                mov     AX, [ESI + 20]
                mov     AL, [ECX + EAX]
                mov     [EDX + 2], AX
                mov     AX, [ESI + 24]
                mov     AL, [ECX + EAX]
                mov     [EDI + 3], AX
                mov     AX, [ESI + 28]
                mov     AL, [ECX + EAX]
                mov     [EDX + 3], AX
                add     ESI, 32
                add     EDI, 4
                add     EDX, 4
                dec     EBX
                jnz     mix_gus8_endphase_loop
                jmp     mix_quit

normalmix_:
_normalmix:
                mov     dword [cptr], offset _judas_channel
normalmixloop:  call    mixchannel
                add     dword [cptr], CHANNEL_size                                    ;type CHANNEL in TASM
                cmp     dword [cptr], offset _judas_channel + CHANNELS * CHANNEL_size ;type CHANNEL in TASM
                jne     normalmixloop
                ret

ipmix_:
_ipmix:
                mov     dword [cptr], offset _judas_channel
ipmixloop:      call    ipmixchannel
                add     dword [cptr], CHANNEL_size                                    ;type CHANNEL in TASM
                cmp     dword [cptr], offset _judas_channel + CHANNELS * CHANNEL_size ;type CHANNEL
                jne     ipmixloop
                ret

        ;Mixes [samples] of channel [cptr] to buffer at [dptr]. Destroys
        ;every register.

mixchannel:     mov     ESI, [cptr]
                test    byte [ESI + Chn_VoiceMode], VM_ON
                jz      near mixchannel_quit
                mov     EAX, [ESI + Chn_Freq]           ;Get playing speed here
                cmp     EAX, 535232                     ;Highest linear freq
                jbe     mixchannel_freqok
                mov     EAX, 535232
mixchannel_freqok:
                mov     EDX, EAX                        ;Don't worry: overflow
                shr     EDX, 16                         ;prevented by check
                shl     EAX, 16                         ;above
                div     dword [_judas_mixrate]          ;DIV is always
                mov     word [fractadd + 2], AX         ;frightening!!!
                shr     EAX, 16
                mov     [integeradd], EAX
                test    byte [ESI + Chn_VoiceMode], VM_16BIT   ;16bit takes the branch
                jnz     near mixchannel16               ;because it's unusual
                test    byte [_judas_mixmode], STEREO   ;Mono takes the branch
                jz      near mixchannel_mono            ;because it's faster
mixchannel_stereo:
                getvolume
                cmp     byte [ESI + Chn_Panning], 0     ;Left panning?
                jne     near mc8_notleft
                stereoadjust
                shl     EBX, 8                          ;Convert to volumetable
                add     EBX, [_judas_volumetable]       ;ofs.
                mov     EBP, [integeradd]               ;EBP = integeradd
                mov     ECX, [fractadd]                 ;ECX = fraction add
                mixloop8 mix8_left_n, 8, 0              ;DO IT!
mc8_notleft:    cmp     byte [ESI + Chn_Panning], 128   ;Middle panning?
                jne     near mc8_notmiddle
                shl     EBX, 8                          ;Convert to volumetable
                add     EBX, [_judas_volumetable]       ;ofs.
                mov     EBP, [integeradd]               ;EBP = integeradd
                mov     ECX, [fractadd]                 ;ECX = fraction add
                mixloop8 mix8_middle_n, 8, 0            ;DO IT!
mc8_notmiddle:  cmp     byte [ESI + Chn_Panning], 255   ;Right panning?
                jne     near mc8_notright
                stereoadjust
                shl     EBX, 8                          ;Convert to volumetable
                add     EBX, [_judas_volumetable]       ;ofs.
                mov     EBP, [integeradd]               ;EBP = integeradd
                mov     ECX, [fractadd]                 ;ECX = fraction add
                mixloop8 mix8_right_n, 8, 0             ;DO IT!
mc8_notright:   smoothpanadjust                         ;Oh no, smooth panning!
                shl     EBX, 8                          ;Convert to volumetable
                add     EBX, [_judas_volumetable]       ;ofs.
                shl     ECX, 8
                add     ECX, [_judas_volumetable]
                mov     EBP, [integeradd]               ;ECX not available!
                mixloop8 mix8_smooth_n, 8, 0            ;But yet we must do it..
mixchannel_mono:getvolume
                shl     EBX, 8                          ;Convert to volumetable
                add     EBX, [_judas_volumetable]       ;ofs.
                mov     EBP, [integeradd]               ;EBP = integeradd
                mov     ECX, [fractadd]                 ;ECX = fraction add
                mixloop8 mix8_mono_n, 4, 0              ;DO IT!
mixchannel_quit:ret
mixchannel16:   test    byte [_judas_mixmode], STEREO   ;Mono takes the branch
                jz      near mixchannel16_mono          ;because it's faster
mixchannel16_stereo:
                getvolume
                cmp     byte [ESI + Chn_Panning], 0     ;Left panning?
                jne     near mc16_notleft
                stereoadjust
                mov     ECX, [fractadd]                 ;ECX = fraction add
                mov     EBP, [integeradd]               ;EBP = integeradd
                mixloop16 mix16_left_n, 8, 0            ;DO IT!
mc16_notleft:   cmp     byte [ESI + Chn_Panning], 128   ;Middle panning?
                jne     near mc16_notmiddle
                mov     ECX, [fractadd]                 ;ECX = fraction add
                mov     EBP, [integeradd]               ;EBP = integeradd
                mixloop16 mix16_middle_n, 8, 0          ;DO IT!
mc16_notmiddle: cmp     byte [ESI + Chn_Panning], 255   ;Right panning?
                jne     near mc16_notright
                stereoadjust
                mov     ECX, [fractadd]                 ;ECX = fraction add
                mov     EBP, [integeradd]               ;EBP = integeradd
                mixloop16 mix16_right_n, 8, 0           ;DO IT!
mc16_notright:  smoothpanadjust                         ;Oh no, smooth panning!
                mov     EBP, [integeradd]
                mixloop16 mix16_smooth_n, 8, 0          ;But yet we must do it..
mixchannel16_mono:
                getvolume
                mov     ECX, [fractadd]                 ;ECX = fraction add
                mov     EBP, [integeradd]               ;EBP = integeradd
                mixloop16 mix16_mono_n, 4, 0            ;DO IT!
mixchannel16_quit:
                ret

        ;Mixes [samples] of channel [cptr] to buffer at [dptr] with
        ;interpolation. Destroys every register.

ipmixchannel:   mov     ESI, [cptr]
                test    byte [ESI + Chn_VoiceMode], VM_ON
                jz      near ipmixchannel_quit
                mov     EAX, [ESI + Chn_Freq]           ;Get playing speed here
                cmp     EAX, 535232                     ;Highest linear freq
                jbe     ipmixchannel_freqok
                mov     EAX, 535232
ipmixchannel_freqok:
                mov     EDX, EAX
                shr     EDX, 16
                shl     EAX, 16
                div     dword [_judas_mixrate]
                mov     word [fractadd + 2], AX
                shr     EAX, 16
                mov     [integeradd], EAX
                test    byte [ESI + Chn_VoiceMode], VM_16BIT   ;16bit takes the branch
                jnz     near ipmixchannel16             ;because it's unusual
                test    byte [_judas_mixmode], STEREO   ;Mono takes the branch
                jz      near ipmixchannel_mono          ;because it's faster
ipmixchannel_stereo:
                getvolume
                cmp     byte [ESI + Chn_Panning], 0     ;Left panning?
                jne     near imc8_notleft
                stereoadjust
                shl     EBX, 8                          ;Convert to volumetable
                add     EBX, [_judas_volumetable]       ;ofs.
                mov     EBP, [integeradd]               ;EBP = integeradd
                mixloop8 mix8_left_i, 8, 1              ;DO IT!
imc8_notleft:   cmp     byte [ESI + Chn_Panning], 128   ;Middle panning?
                jne     near imc8_notmiddle
                shl     EBX, 8                          ;Convert to volumetable
                add     EBX, [_judas_volumetable]       ;ofs.
                mov     EBP, [integeradd]               ;EBP = integeradd
                mixloop8 mix8_middle_i, 8, 1            ;DO IT!
imc8_notmiddle: cmp     byte [ESI + Chn_Panning], 255   ;Right panning?
                jne     near imc8_notright
                stereoadjust
                shl     EBX, 8                          ;Convert to volumetable
                add     EBX, [_judas_volumetable]       ;ofs.
                mov     EBP, [integeradd]               ;EBP = integeradd
                mixloop8 mix8_right_i, 8, 1             ;DO IT!
imc8_notright:  smoothpanadjust                         ;Oh no, smooth panning!
                shl     EBX, 8                          ;Convert to volumetable
                add     EBX, [_judas_volumetable]       ;ofs.
                shl     ECX, 8
                add     ECX, [_judas_volumetable]
                mixloop8 mix8_smooth_i, 8, 1
ipmixchannel_mono:getvolume
                shl     EBX, 8                          ;Convert to volumetable
                add     EBX, [_judas_volumetable]       ;ofs.
                mov     EBP, [integeradd]               ;EBP = integeradd
                mixloop8 mix8_mono_i, 4, 1              ;DO IT!
ipmixchannel_quit:ret
ipmixchannel16: test    byte [_judas_mixmode], STEREO   ;Mono takes the branch
                jz      near ipmixchannel16_mono        ;because it's faster
ipmixchannel16_stereo:
                getvolume
                cmp     byte [ESI + Chn_Panning], 0     ;Left panning?
                jne     near imc16_notleft
                stereoadjust
                mov     [leftvol], EBX
                mov     EBP, [integeradd]               ;EBP = integeradd
                mixloop16 mix16_left_i, 8, 1            ;DO IT!
imc16_notleft:  cmp     byte [ESI + Chn_Panning], 128   ;Middle panning?
                jne     near imc16_notmiddle
                mov     [leftvol], EBX
                mov     EBP, [integeradd]               ;EBP = integeradd
                mixloop16 mix16_middle_i, 8, 1          ;DO IT!
imc16_notmiddle:cmp     byte [ESI + Chn_Panning], 255   ;Right panning?
                jne     near imc16_notright
                stereoadjust
                mov     [leftvol], EBX
                mov     EBP, [integeradd]               ;EBP = integeradd
                mixloop16 mix16_right_i, 8, 1           ;DO IT!
imc16_notright: smoothpanadjust                         ;Oh no, smooth panning!
                mov     [leftvol], EBX
                mov     [rightvol], ECX
                mov     EBP, [integeradd]
                mixloop16 mix16_smooth_i, 8, 1          ;But yet we must do it..
ipmixchannel16_mono:
                getvolume
                mov     [leftvol], EBX
                mov     EBP, [integeradd]               ;EBP = integeradd
                mixloop16 mix16_mono_i, 4, 1            ;DO IT!
ipmixchannel16_quit:
                ret

zerovolume:     mov     EBP, [samples]
                mov     EBX, [ESI + Chn_Pos]
                mov     ECX, EBX
                shr     EBX, 16
                shl     ECX, 16
                mov     CX, [ESI + Chn_FractPos]
                mov     EAX, [ESI + Chn_Freq]
                mov     EDX, EAX
                shr     EDX, 16
                shl     EAX, 16
                div     dword [_judas_mixrate]          ;EAX = mixrate
                test    byte [ESI + Chn_VoiceMode], VM_16BIT   ;If 16bit then double
                jz      zerovolume_not16bit             ;the count
                shl     EBP, 1
zerovolume_not16bit:
                mul     EBP                             ;EDX:EAX = pos. add
                add     ECX, EAX                        ;Add low part
                adc     EBX, EDX                        ;Add high part
                mov     [ESI + Chn_FractPos], CX        ;Save fractpos
                shl     EBX, 16                         ;(won't change now)
                shr     ECX, 16                         ;Now shift back: ECX
                or      ECX, EBX                        ;is integer pos
                test    byte [ESI + Chn_VoiceMode], VM_16BIT   ;Final adjust for 16bit
                jz      zerovolume_no16bitadjust
                and     ECX, 0fffffffeh
zerovolume_no16bitadjust:
                test    byte [ESI + Chn_VoiceMode], VM_LOOP    ;Is it looped?
                jnz     zerovolume_looped
                cmp     ECX, [ESI + Chn_End]
                jae     zerovolume_oneshot_end
zerovolume_ready:
                mov     [ESI + Chn_Pos], ECX            ;Store pos
                ret
zerovolume_oneshot_end:
                mov     byte [ESI + Chn_VoiceMode], VM_OFF
                ret
zerovolume_looped:
                mov     EAX, [ESI + Chn_End]
                sub     EAX, [ESI + Chn_Repeat]         ;EAX = subtract value
zerovolume_looped_loop:
                cmp     ECX, [ESI + Chn_End]
                jb      zerovolume_ready
                sub     ECX, EAX
                jmp     zerovolume_looped_loop









;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³                               Quality Mixer                                ³
;³                                                                            ³
;³                                  by Yehar                                  ³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
;
;32 bit mixing with 30 significant bits. 16 bit interpolation for both 8 and 16
;bit samples. Cubic (8192x) and linear (32768x) interpolation aka oversampling.
;14 bit volume. Zero-level reposition & volume change smoothing click removal.
;
;Don't try to read for educational purposes =)))) It's just too shitty & lazy!

        ;32bit quality mixer main routine starts here. This is separated from
        ;the usual mixer because of different postprocessing etc. This is an
        ;alternative main mixing routine, which mixes EDX bytes of sound into
        ;address EAX, calling the music player at correct intervals.
        ;WARNING: This routine destroys every register!

qmixer_:
_qmixer:
                test    EDX, EDX        ;Buffer size is zero -> nothing to do
                jz      near qmixer_quit
                test    byte [_judas_mixmode], SIXTEENBIT      ;16/8bit? 16 means half
                jz      qmixer_noshift2                 ;the number of samples.
                shr     EDX, 1                          ;It is 16 bit!
qmixer_noshift2:
                test    byte [_judas_mixmode], STEREO   ;Stereo/mono? Stereo means half
                jz      qmixer_noshift1                 ;the number of samples.
                shr     EDX, 1                          ;It is stereo!
qmixer_noshift1:
                mov     [fptr], EAX                     ;Save pointer to final buffer.
                mov     [samples], EDX                  ;Save number of samples.
                mov     [totalwork], EDX                ;Save "total samples left" counter.
                mov     [postproc], EDX                 ;Number of samples to postprocess
                mov     EDI, [_judas_clipbuffer]
                mov     [dptr], EDI                     ;Dptr says to mixroutine: "Mix to clipbuffer!"
                mov     EBX, [_judas_zladdbuffer]
                xor     EAX, EAX
                mov     ECX, EDX
                shl     ECX, 1
qmixer_clearloop:                                       ;Clear clipbuffer & zeroleveladdbuffer
                mov     [EDI], EAX
                mov     [EBX], EAX
                add     EDI, 4
                add     EBX, 4
                dec     ECX
                jnz     qmixer_clearloop

                ;Player calling shit...

                cmp     dword [_judas_player], 0        ;Is there a player?
                jne     qmixer_hardwayloop
                call    dword [_judas_mixroutine]       ;No, just mix the sfx and
                jmp     qmixer_firstphasedone           ;postprocess
qmixer_hardwayloop:
                cmp     dword [_judas_bpmcount], 0      ;Yes, check if it has to be
                jne     qmixer_skipplaying              ;called.
                cmp     dword [_judas_player], 0        ;It has to, but player might
                je      qmixer_fuckshitup               ;have disappeared.
                call    dword [_judas_player]           ;Call if it hasn't.
qmixer_fuckshitup:
                mov     EAX, [_judas_mixrate]
                mov     EBP, 5
                mul     EBP
                shr     EAX, 1
                xor     EDX, EDX
                movzx   EBP, byte [_judas_bpmtempo]
                div     EBP
                mov     [_judas_bpmcount], EAX
qmixer_skipplaying:
                mov     EAX, [totalwork]
                cmp     EAX, [_judas_bpmcount]
                jbe     qmixer_nolimit
                mov     EAX, [_judas_bpmcount]
qmixer_nolimit:
                mov     [samples], EAX
                call    dword [_judas_mixroutine]
                mov     EAX, [samples]
                sub     [_judas_bpmcount], EAX
                mov     EBP, EAX
                shl     EBP, 3
                add     [dptr], EBP
                sub     [totalwork], EAX
                jnz     qmixer_hardwayloop

                ;Postprocessing...

qmixer_firstphasedone:
                test    byte [_judas_mixmode], SIXTEENBIT
                jz      near qmixer_8bit_endphase
qmixer_16bit_endphase:
                test    byte [_judas_mixmode], STEREO
                jz      near qmixer_nogusshit1
                cmp     dword [_judas_device], DEV_GUS
                je      near qmixer_gus16s_endphase
                ;stereo
                mov     ESI, [_judas_clipbuffer]
                mov     EBX, [_judas_zladdbuffer]
                mov     EDI, [fptr]
qmixer_s16bit_endphase_loop:
                ;left
                mov     EAX, [ESI]                      ;Get value from clipbuffer
                mov     ECX, [_judas_zerolevell]
                add     ECX, [EBX]                      ;Add from zladdbuffer to zerolevel
                mov     EBP, ECX                        ;zerolevel gets 1/2048 closer to real zero
                sar     EBP, QMIXER_ZEROLEVELDECAY
                sub     ECX, EBP
                mov     [_judas_zerolevell], ECX
                add     EAX, ECX

                sar     EAX, SIGNIFICANT_BITS_32 - 16   ;Shrink to 16 bit
                cmp     EAX, 32767
                jg      qmixer_s16bit_overflowl
                cmp     EAX, -32768
                jl      qmixer_s16bit_underflowl
                mov     [EDI], AX
qmixer_s16bit_cutdonel:

                ;right
                mov     EAX, [ESI+4]                    ;Get value from clipbuffer
                mov     ECX, [_judas_zerolevelr]
                add     ECX, [EBX+4]                    ;Add from zladdbuffer to zerolevel
                mov     EBP, ECX                        ;zerolevel gets 1/2048 closer to real zero
                sar     EBP, QMIXER_ZEROLEVELDECAY
                sub     ECX, EBP
                mov     [_judas_zerolevelr], ECX
                add     EAX, ECX

                sar     EAX, SIGNIFICANT_BITS_32 - 16   ;Shrink to 16 bit
                cmp     EAX, 32767
                jg      qmixer_s16bit_overflowr
                cmp     EAX, -32768
                jl      qmixer_s16bit_underflowr
                mov     [EDI+2], AX
qmixer_s16bit_cutdoner:

                add     ESI, 8
                add     EBX, 8
                add     EDI, 4
                dec     dword [postproc]
                jnz     qmixer_s16bit_endphase_loop
                ret
qmixer_s16bit_overflowl:
                mov     word [EDI], 32767
                shr     EAX, 9
                cmp     [_judas_clipped], AL
                jae     qmixer_s16bit_cutdonel
                mov     [_judas_clipped], AL
                jmp     qmixer_s16bit_cutdonel
qmixer_s16bit_underflowl:
                mov     word [EDI], -32768
                neg     EAX
                shr     EAX, 9
                cmp     [_judas_clipped], AL
                jae     qmixer_s16bit_cutdonel
                mov     [_judas_clipped], AL
                jmp     qmixer_s16bit_cutdonel
qmixer_s16bit_overflowr:
                mov     word [EDI+2], 32767
                shr     EAX, 9
                cmp     [_judas_clipped], AL
                jae     qmixer_s16bit_cutdoner
                mov     [_judas_clipped], AL
                jmp     qmixer_s16bit_cutdoner
qmixer_s16bit_underflowr:
                mov     word [EDI+2], -32768
                neg     EAX
                shr     EAX, 9
                cmp     [_judas_clipped], AL
                jae     qmixer_s16bit_cutdoner
                mov     [_judas_clipped], AL
                jmp     qmixer_s16bit_cutdoner
qmixer_nogusshit1: ;mono 16bit
                mov     ESI, [_judas_clipbuffer]
                mov     EBX, [_judas_zladdbuffer]
                mov     EDI, [fptr]
qmixer_m16bit_endphase_loop:

                mov     EAX, [ESI]                      ;Get value from clipbuffer
                sar     EAX, 1
                mov     ECX, [ESI+4]                    ; + right
                sar     ECX, 1
                add     EAX, ECX

                mov     ECX, [_judas_zerolevell]        ;left zerolevel
                add     ECX, [EBX]                      ;Add from zladdbuffer to zerolevel
                mov     EBP, ECX                        ;zerolevel gets 1/2048 closer to real zero
                sar     EBP, QMIXER_ZEROLEVELDECAY
                sub     ECX, EBP
                mov     [_judas_zerolevell], ECX
                sar     ECX, 1
                add     EAX, ECX
                mov     ECX, [_judas_zerolevelr]        ;right zerolevel
                add     ECX, [EBX+4]                    ;Add from zladdbuffer to zerolevel
                mov     EBP, ECX                        ;zerolevel gets 1/2048 closer to real zero
                sar     EBP, QMIXER_ZEROLEVELDECAY
                sub     ECX, EBP
                mov     [_judas_zerolevelr], ECX
                sar     ECX, 1
                add     EAX, ECX

                sar     EAX, SIGNIFICANT_BITS_32 - 16   ;Shrink to 16 bit
                cmp     EAX, 32767
                jg      qmixer_m16bit_overflow
                cmp     EAX, -32768
                jl      qmixer_m16bit_underflow
                mov     [EDI], AX
qmixer_m16bit_cutdone:

                add     ESI, 8
                add     EBX, 8
                add     EDI, 2
                dec     dword [postproc]
                jnz     qmixer_m16bit_endphase_loop
                ret
qmixer_m16bit_overflow:
                mov     word [EDI], 32767
                shr     EAX, 9
                cmp     [_judas_clipped], AL
                jae     qmixer_m16bit_cutdone
                mov     [_judas_clipped], AL
                jmp     qmixer_m16bit_cutdone
qmixer_m16bit_underflow:
                mov     word [EDI], -32768
                neg     EAX
                shr     EAX, 9
                cmp     [_judas_clipped], AL
                jae     qmixer_m16bit_cutdone
                mov     [_judas_clipped], AL
                jmp     qmixer_m16bit_cutdone

qmixer_8bit_endphase:
                test    byte [_judas_mixmode], STEREO
                jz      near qmixer_nogusshit2
                cmp     dword [_judas_device], DEV_GUS
                je      near qmixer_gus8s_endphase
                ;stereo
                mov     ESI, [_judas_clipbuffer]
                mov     EBX, [_judas_zladdbuffer]
                mov     EDI, [fptr]
qmixer_s8bit_endphase_loop:
                ;left
                mov     EAX, [ESI]                      ;Get value from clipbuffer
                mov     ECX, [_judas_zerolevell]
                add     ECX, [EBX]                      ;Add from zladdbuffer to zerolevel
                mov     EBP, ECX                        ;zerolevel gets 1/2048 closer to real zero
                sar     EBP, QMIXER_ZEROLEVELDECAY
                sub     ECX, EBP
                mov     [_judas_zerolevell], ECX
                add     EAX, ECX

                sar     EAX, SIGNIFICANT_BITS_32 - 8    ;Shrink to 8 bit
                cmp     EAX, 127
                jg      qmixer_s8bit_overflowl
                cmp     EAX, -128
                jl      qmixer_s8bit_underflowl
                add     AL, 128
                mov     byte [EDI], AL
qmixer_s8bit_cutdonel:

                ;right
                mov     EAX, [ESI+4]                    ;Get value from clipbuffer
                mov     ECX, [_judas_zerolevelr]
                add     ECX, [EBX+4]                    ;Add from zladdbuffer to zerolevel
                mov     EBP, ECX                        ;zerolevel gets 1/2048 closer to real zero
                sar     EBP, QMIXER_ZEROLEVELDECAY
                sub     ECX, EBP
                mov     [_judas_zerolevelr], ECX
                add     EAX, ECX

                sar     EAX, SIGNIFICANT_BITS_32 - 8    ;Shrink to 8 bit
                cmp     EAX, 127
                jg      qmixer_s8bit_overflowr
                cmp     EAX, -128
                jl      qmixer_s8bit_underflowr
                add     AL, 128
                mov     byte [EDI+1], AL
qmixer_s8bit_cutdoner:

                add     ESI, 8
                add     EBX, 8
                add     EDI, 2
                dec     dword [postproc]
                jnz     qmixer_s8bit_endphase_loop
                ret
qmixer_s8bit_overflowl:
                mov     byte [EDI], 255
                shr     EAX, 1
                cmp     [_judas_clipped], AL
                jae     qmixer_s8bit_cutdonel
                mov     [_judas_clipped], AL
                jmp     qmixer_s8bit_cutdonel
qmixer_s8bit_underflowl:
                mov     byte [EDI], 0
                neg     EAX
                shr     EAX, 1
                cmp     [_judas_clipped], AL
                jae     qmixer_s8bit_cutdonel
                mov     [_judas_clipped], AL
                jmp     qmixer_s8bit_cutdonel
qmixer_s8bit_overflowr:
                mov     byte [EDI+1], 255
                shr     EAX, 1
                cmp     [_judas_clipped], AL
                jae     qmixer_s8bit_cutdoner
                mov     [_judas_clipped], AL
                jmp     qmixer_s8bit_cutdoner
qmixer_s8bit_underflowr:
                mov     byte [EDI+1], 0
                neg     EAX
                shr     EAX, 1
                cmp     [_judas_clipped], AL
                jae     qmixer_s8bit_cutdoner
                mov     [_judas_clipped], AL
                jmp     qmixer_s8bit_cutdoner
qmixer_nogusshit2: ;mono 8bit

                mov     ESI, [_judas_clipbuffer]
                mov     EBX, [_judas_zladdbuffer]
                mov     EDI, [fptr]
qmixer_m8bit_endphase_loop:

                mov     EAX, [ESI]                      ;Get value from clipbuffer
                sar     EAX, 1
                mov     ECX, [ESI+4]                    ; + right
                sar     ECX, 1
                add     EAX, ECX

                mov     ECX, [_judas_zerolevell]        ;left zerolevel
                add     ECX, [EBX]                      ;Add from zladdbuffer to zerolevel
                mov     EBP, ECX                        ;zerolevel gets 1/2048 closer to real zero
                sar     EBP, QMIXER_ZEROLEVELDECAY
                sub     ECX, EBP
                mov     [_judas_zerolevell], ECX
                sar     ECX, 1
                add     EAX, ECX
                mov     ECX, [_judas_zerolevelr]        ;right zerolevel
                add     ECX, [EBX+4]                    ;Add from zladdbuffer to zerolevel
                mov     EBP, ECX                        ;zerolevel gets 1/2048 closer to real zero
                sar     EBP, QMIXER_ZEROLEVELDECAY
                sub     ECX, EBP
                mov     [_judas_zerolevelr], ECX
                sar     ECX, 1
                add     EAX, ECX

                sar     EAX, SIGNIFICANT_BITS_32 - 8    ;Shrink to 8 bit
                cmp     EAX, 127
                jg      qmixer_m8bit_overflow
                cmp     EAX, -128
                jl      qmixer_m8bit_underflow
                add     AL, 128
                mov     byte [EDI], AL
qmixer_m8bit_cutdone:

                add     ESI, 8
                add     EBX, 8
                inc     EDI
                dec     dword [postproc]
                jnz     qmixer_m8bit_endphase_loop
                ret
qmixer_m8bit_overflow:
                mov     byte [EDI], 255
                shr     EAX, 1
                cmp     [_judas_clipped], AL
                jae     qmixer_m8bit_cutdone
                mov     [_judas_clipped], AL
                jmp     qmixer_m8bit_cutdone
qmixer_m8bit_underflow:
                mov     byte [EDI], 0
                neg     EAX
                shr     EAX, 1
                cmp     [_judas_clipped], AL
                jae     qmixer_m8bit_cutdone
                mov     [_judas_clipped], AL
                jmp     qmixer_m8bit_cutdone

qmixer_gus16s_endphase:
                mov     ESI, [_judas_clipbuffer]
                mov     EBX, [_judas_zladdbuffer]
                mov     EDI, [fptr]                     ;[EDI] = gus left
                mov     EDX, [_judas_bufferlength]
                shr     EDX, 1
                add     EDX, EDI
                add     EDX, 32                         ;[EDX] = gus right

qmixer_gus16s_endphase_loop:
                ;left
                mov     EAX, [ESI]                      ;Get value from clipbuffer
                mov     ECX, [_judas_zerolevell]
                add     ECX, [EBX]                      ;Add from zladdbuffer to zerolevel
                mov     EBP, ECX                        ;zerolevel gets 1/2048 closer to real zero
                sar     EBP, QMIXER_ZEROLEVELDECAY
                sub     ECX, EBP
                mov     [_judas_zerolevell], ECX
                add     EAX, ECX

                sar     EAX, SIGNIFICANT_BITS_32 - 16   ;Shrink to 16 bit
                cmp     EAX, 32767
                jg      qmixer_gus16s_overflowl
                cmp     EAX, -32768
                jl      qmixer_gus16s_underflowl
                mov     [EDI], AX
qmixer_gus16s_cutdonel:

                ;right
                mov     EAX, [ESI+4]                    ;Get value from clipbuffer
                mov     ECX, [_judas_zerolevelr]
                add     ECX, [EBX+4]                    ;Add from zladdbuffer to zerolevel
                mov     EBP, ECX                        ;zerolevel gets 1/2048 closer to real zero
                sar     EBP, QMIXER_ZEROLEVELDECAY
                sub     ECX, EBP
                mov     [_judas_zerolevelr], ECX
                add     EAX, ECX

                sar     EAX, SIGNIFICANT_BITS_32 - 16   ;Shrink to 16 bit
                cmp     EAX, 32767
                jg      qmixer_gus16s_overflowr
                cmp     EAX, -32768
                jl      qmixer_gus16s_underflowr
                mov     [EDX], AX
qmixer_gus16s_cutdoner:

                add     ESI, 8
                add     EBX, 8
                add     EDI, 2
                add     EDX, 2
                dec     dword [postproc]
                jnz     qmixer_gus16s_endphase_loop
                ret
qmixer_gus16s_overflowl:
                mov     word [EDI], 32767
                shr     EAX, 9
                cmp     [_judas_clipped], AL
                jae     qmixer_gus16s_cutdonel
                mov     [_judas_clipped], AL
                jmp     qmixer_gus16s_cutdonel
qmixer_gus16s_underflowl:
                mov     word [EDI], -32768
                neg     EAX
                shr     EAX, 9
                cmp     [_judas_clipped], AL
                jae     qmixer_gus16s_cutdonel
                mov     [_judas_clipped], AL
                jmp     qmixer_gus16s_cutdonel
qmixer_gus16s_overflowr:
                mov     word [EDX], 32767
                shr     EAX, 9
                cmp     [_judas_clipped], AL
                jae     qmixer_gus16s_cutdoner
                mov     [_judas_clipped], AL
                jmp     qmixer_gus16s_cutdoner
qmixer_gus16s_underflowr:
                mov     word [EDX], -32768
                neg     EAX
                shr     EAX, 9
                cmp     [_judas_clipped], AL
                jae     near qmixer_gus16s_cutdoner
                mov     [_judas_clipped], AL
                jmp     qmixer_gus16s_cutdoner

qmixer_gus8s_endphase:
                mov     ESI, [_judas_clipbuffer]
                mov     EBX, [_judas_zladdbuffer]
                mov     EDI, [fptr]                     ;[EDI] = gus left
                mov     EDX, [_judas_bufferlength]
                shr     EDX, 1
                add     EDX, EDI
                add     EDX, 32                         ;[EDX] = gus right
qmixer_gus8s_endphase_loop:
                ;left
                mov     EAX, [ESI]                      ;Get value from clipbuffer
                mov     ECX, [_judas_zerolevell]
                add     ECX, [EBX]                      ;Add from zladdbuffer to zerolevel
                mov     EBP, ECX                        ;zerolevel gets 1/2048 closer to real zero
                sar     EBP, QMIXER_ZEROLEVELDECAY
                sub     ECX, EBP
                mov     [_judas_zerolevell], ECX
                add     EAX, ECX

                sar     EAX, SIGNIFICANT_BITS_32 - 8    ;Shrink to 8 bit
                cmp     EAX, 127
                jg      qmixer_gus8s_overflowl
                cmp     EAX, -128
                jl      qmixer_gus8s_underflowl
                add     AL, 128
                mov     byte [EDI], AL
qmixer_gus8s_cutdonel:

                ;right
                mov     EAX, [ESI+4]                    ;Get value from clipbuffer
                mov     ECX, [_judas_zerolevelr]
                add     ECX, [EBX+4]                    ;Add from zladdbuffer to zerolevel
                mov     EBP, ECX                        ;zerolevel gets 1/2048 closer to real zero
                sar     EBP, QMIXER_ZEROLEVELDECAY
                sub     ECX, EBP
                mov     [_judas_zerolevelr], ECX
                add     EAX, ECX

                sar     EAX, SIGNIFICANT_BITS_32 - 8    ;Shrink to 8 bit
                cmp     EAX, 127
                jg      qmixer_gus8s_overflowr
                cmp     EAX, -128
                jl      qmixer_gus8s_underflowr
                add     AL, 128
                mov     byte [EDX], AL
qmixer_gus8s_cutdoner:

                add     ESI, 8
                add     EBX, 8
                inc     EDI
                inc     EDX
                dec     dword [postproc]
                jnz     qmixer_gus8s_endphase_loop
                ret
qmixer_gus8s_overflowl:
                mov     byte [EDI], 255
                shr     EAX, 1
                cmp     [_judas_clipped], AL
                jae     qmixer_gus8s_cutdonel
                mov     [_judas_clipped], AL
                jmp     qmixer_gus8s_cutdonel
qmixer_gus8s_underflowl:
                mov     byte [EDI], 0
                neg     EAX
                shr     EAX, 1
                cmp     [_judas_clipped], AL
                jae     qmixer_gus8s_cutdonel
                mov     [_judas_clipped], AL
                jmp     qmixer_gus8s_cutdonel
qmixer_gus8s_overflowr:
                mov     byte [EDX], 255
                shr     EAX, 1
                cmp     [_judas_clipped], AL
                jae     qmixer_gus8s_cutdoner
                mov     [_judas_clipped], AL
                jmp     qmixer_gus8s_cutdoner
qmixer_gus8s_underflowr:
                mov     byte [EDX], 0
                neg     EAX
                shr     EAX, 1
                cmp     [_judas_clipped], AL
                jae     qmixer_gus8s_cutdoner
                mov     [_judas_clipped], AL
                jmp     qmixer_gus8s_cutdoner
qmixer_quit:    ret

qmix_cubic_:
_qmix_cubic:
                mov     dword [cptr], offset _judas_channel
qmix_cubic_channelloop:
                call    qmixchannel_cubic
                add     dword [cptr], CHANNEL_size                                    ;type CHANNEL in TASM
                cmp     dword [cptr], offset _judas_channel + CHANNELS * CHANNEL_size ;type CHANNEL in TASM
                jne     qmix_cubic_channelloop
                ret

qmix_linear_:
_qmix_linear:
                mov     dword [cptr], offset _judas_channel
qmix_linear_channelloop:
                call    qmixchannel_linear
                add     dword [cptr], CHANNEL_size                                    ;type CHANNEL in TASM
                cmp     dword [cptr], offset _judas_channel + CHANNELS * CHANNEL_size ;type CHANNEL
                jne     qmix_cubic_channelloop
                ret

q_zerovolume:
                ; Zerolevel stuff...

                mov     EAX, [dptr]             ;Get correct place at zladdbuf
                sub     EAX, [_judas_clipbuffer]  ;
                add     EAX, [_judas_zladdbuffer] ;
                mov     EDX, [EBX + Chn_LastValL]
                add     [EAX], EDX
                mov     EDX, [EBX + Chn_LastValR]
                mov     dword [EBX + Chn_LastValL], 0
                add     [EAX+4], EDX
                mov     dword [EBX + Chn_LastValR], 0

                mov     ESI, [samples]
                mov     EBP, [EBX + Chn_Pos]
                mov     ECX, EBP
                shr     EBP, 16
                shl     ECX, 16
                mov     CX, [EBX + Chn_FractPos]
                mov     EAX, [EBX + Chn_Freq]
                mov     EDX, EAX
                shr     EDX, 16
                shl     EAX, 16
                div     dword [_judas_mixrate]          ;EAX = mixrate
                test    byte [EBX + Chn_VoiceMode], VM_16BIT   ;If 16bit then double
                jz      q_zerovolume_not16bit           ;the count
                shl     ESI, 1
q_zerovolume_not16bit:
                mul     ESI                             ;EDX:EAX = pos. add
                add     ECX, EAX                        ;Add low part
                adc     EBP, EDX                        ;Add high part
                mov     [EBX + Chn_FractPos], CX        ;Save fractpos
                shl     EBP, 16                         ;(won't change now)
                shr     ECX, 16                         ;Now shift back: ECX
                or      ECX, EBP                        ;is integer pos
                test    byte [EBX + Chn_VoiceMode], VM_16BIT   ;Final adjust for 16bit
                jz      q_zerovolume_no16bitadjust
                and     ECX, 0fffffffeh
q_zerovolume_no16bitadjust:
                test    byte [EBX + Chn_VoiceMode], VM_LOOP    ;Is it looped?
                jnz     q_zerovolume_looped
                cmp     ECX, [EBX + Chn_End]
                jae     q_zerovolume_oneshot_end
q_zerovolume_ready:
                mov     [EBX + Chn_Pos], ECX            ;Store pos
                ret
q_zerovolume_oneshot_end:
                mov     byte [EBX + Chn_VoiceMode], VM_OFF
                ret
q_zerovolume_looped:
                mov     EAX, [EBX + Chn_End]
                sub     EAX, [EBX + Chn_Repeat]         ;EAX = subtract value
q_zerovolume_looped_loop:
                cmp     ECX, [EBX + Chn_End]
                jb      q_zerovolume_ready
                sub     ECX, EAX
                jmp     q_zerovolume_looped_loop

        ;MACROS FOR QUALITY MIXER!

%macro          q_volpan14 0
                ;Volume!
                xor     EAX, EAX
                mov     AX, [EBX + Chn_Vol]
                xor     ECX, ECX
                mov     CL, [EBX + Chn_MasterVol]
                imul    EAX, ECX
                shr     EAX, 8
                mov     EBP, EAX                        ;14 bit volume in AX
                or      AX, word [EBX + Chn_SmoothVolL+1]
                or      AX, word [EBX + Chn_SmoothVolR+1]
                jz      near q_zerovolume
                ;Panning!
                xor     EAX, EAX
                mov     AL, [EBX + Chn_Panning]
                mov     ECX, EBP                        ;EBP = leftvol, ECX = rightvol
                imul    ECX, EAX
                neg     EAX      ;EAX = 255-EAX
                add     EAX, 255 ;
                imul    EBP, EAX
%endmacro

%macro          q_mix8_stereo_l 0
                movsx   EAX, byte [ESI]                 ;
                movsx   EBP, byte [ESI+1]               ;
                mov     EBX, [rightvol]                 ;Smooth volume slide ;uv
                sub     EBP, EAX                        ;EBP = 16bit slidevalue
                mov     EDX, ECX                        ;EDX = fractpos
                sub     EBX, [SmoothVolR]               ;
                shr     EDX, 17                         ;32768x interpolation
                add     EDI, 8                          ;
                imul    EBP, EDX                        ;EBP = interpolated value
                sar     EBX, QMIXER_VOLUMESMOOTH        ;1/32 closer
                mov     EDX, [leftvol]                  ;Smooth volume sli.
                add     EBX, [SmoothVolR]               ;
                sub     EDX, [SmoothVolL]               ;
                sal     EAX, 8                          ;
                sar     EDX, QMIXER_VOLUMESMOOTH        ;1/32 closer
                mov     [SmoothVolR], EBX               ;
                shr     EBX, 7                          ;
                add     EDX, [SmoothVolL]               ;
                sar     EBP, 7                          ;
                mov     [SmoothVolL], EDX               ;
                shr     EDX, 7                          ;
                add     EBP, EAX                        ;

                add     ECX, [fractadd]                 ;Sample pos fractional part
                mov     EAX, EBP                        ;
                adc     ESI, [integeradd]               ;Sample pos integer part

                imul    EBP, EBX                        ;

                imul    EAX, EDX                        ;

                add     [EDI-8], EAX                    ;
                add     [EDI-8+4], EBP                  ;
%endmacro

%macro          q_mix8_stereo_c 0                       ;Here starts the pervert Hermite interpolation!!!

                movsx   EBP, byte [ESI+IPMINUS1]
                movsx   EDX, byte [ESI+IP1]
                movsx   EBX, byte [ESI+IP2]
                movsx   EAX, byte [ESI+IP0]
                sal     EBX, 8                          ;
                sal     EDX, 8                          ;
                mov     dword [ip2], EBX                ;
                sal     EAX, 8                          ;
                mov     dword [ip1], EDX                ;
                mov     EBX, EAX                        ;
                sub     EAX, EDX                        ;
                sal     EBP, 8                          ;

                mov     [ipminus1], EBP                 ;
                lea     EAX, [EAX*4+EDX]                ;
                mov     EDX, ECX                        ;
                sub     EAX, EBX                        ;
                shr     EDX, 19                         ;8192x interpolation
                sub     EAX, EBP                        ;
                add     EAX, [ip2]                      ;
                lea     EBP, [EBX*4+EBX]                ;

                imul    EAX, EDX                        ;

                sar     EAX, 32-19+1                    ;
                add     EBP, [ip2]                      ;
                sar     EBP, 1                          ;
                add     EAX, [ip1]                      ;
                add     EAX, [ip1]                      ;
                add     EDI, 8                          ;
                sub     EAX, EBP                        ;
                mov     EBP, [ip1]                      ;
                add     EAX, [ipminus1]                 ;
                sub     EBP, [ipminus1]                 ;

                imul    EAX, EDX                        ;

                sar     EBP, 1                          ;
                sar     EAX, 32-19                      ;
                add     ECX, [fractadd]                 ;Sample pos fract.
                adc     ESI, [integeradd]               ;Sample pos integer
                add     EAX, EBP                        ;

                imul    EAX, EDX                        ;

                sar     EAX, 32-19                      ;
                mov     EDX, [leftvol]                  ;Smooth vol.sl.
                add     EAX, EBX                        ;
                mov     EBX, [rightvol]                 ;Smooth vol.sl.
                sub     EDX, [SmoothVolL]               ;
                sub     EBX, [SmoothVolR]               ;
                sar     EBX, QMIXER_VOLUMESMOOTH        ;1/32 closer
                add     EBX, [SmoothVolR]               ;
                sar     EDX, QMIXER_VOLUMESMOOTH        ;1/32 closer
                mov     [SmoothVolR], EBX               ;
                shr     EBX, 7                          ;
                add     EDX, [SmoothVolL]               ;
                mov     [SmoothVolL], EDX               ;
                mov     EBP, EAX                        ;
                shr     EDX, 7                          ;

                imul    EAX, EDX                        ;

                imul    EBP, EBX                        ;

                add     [EDI-8], EAX                    ;
                add     [EDI-8+4], EBP                  ;
%endmacro

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;pos adds are already defined. EBP contains volume for left channel
;and ECX for right.
%macro          q_mixloop8 1                            ; routine
                mov     [leftvol], EBP                  ;Save volumes
                mov     [rightvol], ECX                 ;

                mov     ESI, [EBX + Chn_Pos]            ;ESI:ECX = pos. ESI is integer
                mov     CX, [EBX + Chn_FractPos]        ;part and ECX fraction.
                shl     ECX, 16

                mov     EAX, [EBX + Chn_End]            ;Get sample end and repeat
                mov     [smpend], EAX                   ;positions. Calculate subtract
                sub     EAX, [EBX + Chn_Repeat]         ;value.
                mov     [smpsubtract], EAX

                mov     EDI, [dptr]                     ;Where to mix to?
                mov     EAX, [samples]                  ;How many samples to mix?
                mov     [loopcount], EAX

                ;Click removing for volume change and sample replace

                mov     EAX, [EBX + Chn_Pos]            ;Is it a new sound?
                cmp     EAX, [EBX + Chn_PrevPos]
                je      near %%soonmixloop              ;jump if not
;New sound
                mov     EDX, [leftvol]
%IFDEF QMIXER_STARTMUTE
                shr     EDX, QMIXER_STARTMUTE           ;Just to remove low frequency clicks
%ENDIF
                mov     [SmoothVolL], EDX
                mov     EDX, [rightvol]
%IFDEF QMIXER_STARTMUTE
                shr     EDX, QMIXER_STARTMUTE
%ENDIF
                mov     [SmoothVolR], EDX
                mov     [saved_reg], EBX
                %1                                      ;routine (macro parameter)
                mov     EBX, [saved_reg]
                mov     EDX, [EBX + Chn_LastValL]
                mov     [saved_reg], EAX
                sub     EDX, EAX
                mov     EAX, [_judas_zladdbuffer]       ;Get correct place at zladdbuf
                sub     EAX, [_judas_clipbuffer]        ;
                add     [EAX+EDI-8], EDX
                mov     EDX, [EBX + Chn_LastValR]
                sub     EDX, EBP
                add     [EAX+EDI-8+4], EDX
                mov     EAX, [saved_reg]                ;Maybe needed if sample ends now
                mov     [saved_reg], EBX
                jmp     near %%donezladjust

%%soonmixloop:  mov     EDX, [EBX + Chn_SmoothVolL]
                mov     EBP, [EBX + Chn_SmoothVolR]
                mov     [SmoothVolL], EDX
                mov     [SmoothVolR], EBP
                mov     [saved_reg], EBX

                align 16

%%mixloop:      %1                                      ; routine (macro parameter) - Mix one sample
%%donezladjust:
                cmp     ESI, [smpend]                   ;End of sample?
                jae     %%hitend

                dec     dword [loopcount]               ;Still shit to do?
                jnz     near %%mixloop

                mov     EBX, [saved_reg]
                mov     [EBX + Chn_Pos], ESI            ;No, all done. Save position
                mov     [EBX + Chn_PrevPos], ESI        ;..and prevpos
                shr     ECX, 16
                mov     [EBX + Chn_FractPos], CX
                mov     [EBX + Chn_LastValL], EAX       ;Save Last values also
                mov     [EBX + Chn_LastValR], EBP       ;
                mov     EAX, [SmoothVolL]               ;Save volume
                mov     EBP, [SmoothVolR]               ;
                mov     [EBX + Chn_SmoothVolL], EAX     ;
                mov     [EBX + Chn_SmoothVolR], EBP     ;
                ret

%%hitend:
                mov     EBX, [saved_reg]
                test    byte [EBX + Chn_VoiceMode], VM_LOOP    ;Is it a looped sample?
                jz      %%oneshot
%%subloop:
                sub     ESI, [smpsubtract]              ;Looped. Go to loop start.
                cmp     ESI, [smpend]
                jae     %%subloop                       ;Fucking shit, it got far beyond sample end.

                dec     dword [loopcount]               ;Still shit to do?
                jnz     near %%mixloop

                mov     [EBX + Chn_Pos], ESI            ;No, all done. Save position
                shr     ECX, 16
                mov     [EBX + Chn_FractPos], CX
                mov     [EBX + Chn_PrevPos], ESI        ;Save prevpos too
                mov     [EBX + Chn_LastValL], EAX       ;...Last values also
                mov     [EBX + Chn_LastValR], EBP       ;
                mov     EAX, [SmoothVolL]               ;Save volume
                mov     EBP, [SmoothVolR]               ;
                mov     [EBX + Chn_SmoothVolL], EAX     ;
                mov     [EBX + Chn_SmoothVolR], EBP     ;
                ret

%%oneshot:
                mov     byte [EBX + Chn_VoiceMode], VM_OFF ;Your time to die, sample!

                ;If sample doesn't end at zero, zerolevel must be adjusted.
                mov     dword [EBX + Chn_Pos], 0
                mov     dword [EBX + Chn_LastValL], 0
                mov     dword [EBX + Chn_LastValR], 0
                cmp     dword [loopcount], 1
                jbe     %%oneshot_lastsample
                mov     EDX, EDI                        ;Get correct place in zladdbuf
                sub     EDX, [_judas_clipbuffer]        ;
                add     EDX, [_judas_zladdbuffer]       ;
                add     [EDX], EAX
                add     [EDX+4], EBP
                mov     byte [EBX + Chn_PrevVM], VM_OFF ;No need to update zerolevel
                ret                                     ;at beginning of next chunk

%%oneshot_lastsample:                                   ;Last sample in mixed chunk
                mov     byte [EBX + Chn_PrevVM], VM_ON
                ret
%endmacro

%macro          q_mix16_stereo_l 0
                movsx   EAX, word [ESI*2]               ;
                movsx   EBP, word [ESI*2+2]             ;
                mov     EBX, [rightvol]                 ;Smooth volume slide ;uv
                sub     EBP, EAX                        ;EBP = 16bit slidevalue
                mov     EDX, ECX                        ;EDX = fractpos
                sub     EBX, [SmoothVolR]               ;
                shr     EDX, 17                         ;32768x interpolation
                add     EDI, 8                          ;
                imul    EBP, EDX                        ;EBP = interpolated value
                sar     EBX, QMIXER_VOLUMESMOOTH        ;1/32 closer
                mov     EDX, [leftvol]                  ;Smooth volume sli.
                add     EBX, [SmoothVolR]               ;
                sub     EDX, [SmoothVolL]               ;
                sar     EDX, QMIXER_VOLUMESMOOTH        ;1/32 closer
                mov     [SmoothVolR], EBX               ;
                shr     EBX, 7                          ;
                add     EDX, [SmoothVolL]               ;
                sar     EBP, 15                         ;
                mov     [SmoothVolL], EDX               ;
                shr     EDX, 7                          ;
                add     EBP, EAX                        ;

                add     ECX, [fractadd]                 ;Sample pos fractional part
                mov     EAX, EBP                        ;
                adc     ESI, [integeradd]               ;Sample pos integer part

                imul    EBP, EBX                        ;

                imul    EAX, EDX                        ;

                add     [EDI-8], EAX                    ;
                add     [EDI-8+4], EBP                  ;
%endmacro

%macro          q_mix16_stereo_c 0                      ;Here starts the pervert Hermite interpolation!!!
                movsx   EBP, word [ESI*2+IPMINUS1*2]
                movsx   EAX, word [ESI*2+IP0*2]
                movsx   EBX, word [ESI*2+IP2*2]
                movsx   EDX, word [ESI*2+IP1*2]
                mov     [ip2], EBX                      ;
                mov     [ip1], EDX                      ;
                mov     EBX, EAX                        ;

                sub     EAX, EDX                        ;
                mov     [ipminus1], EBP                 ;
                lea     EAX, [EAX*4+EDX]                ;
                mov     EDX, ECX                        ;
                sub     EAX, EBX                        ;
                shr     EDX, 19                         ;8192x interpolation
                sub     EAX, EBP                        ;
                add     EAX, [ip2]                      ;
                lea     EBP, [EBX*4+EBX]                ;

                imul    EAX, EDX                        ;

                sar     EAX, 32-19+1                    ;
                add     EBP, [ip2]                      ;
                sar     EBP, 1                          ;
                add     EAX, [ip1]                      ;
                add     EAX, [ip1]                      ;
                add     EDI, 8                          ;
                sub     EAX, EBP                        ;
                mov     EBP, [ip1]                      ;
                add     EAX, [ipminus1]                 ;
                sub     EBP, [ipminus1]                 ;

                imul    EAX, EDX                        ;

                sar     EBP, 1                          ;
                sar     EAX, 32-19                      ;
                add     ECX, [fractadd]                 ;Sample pos fract.
                adc     ESI, [integeradd]               ;Sample pos integer
                add     EAX, EBP                        ;

                imul    EAX, EDX                        ;

                sar     EAX, 32-19                      ;
                mov     EDX, [leftvol]                  ;Smooth vol.sl.
                add     EAX, EBX                        ;
                mov     EBX, [rightvol]                 ;Smooth vol.sl.
                sub     EDX, [SmoothVolL]               ;
                sub     EBX, [SmoothVolR]               ;
                sar     EBX, QMIXER_VOLUMESMOOTH        ;1/32 closer
                add     EBX, [SmoothVolR]               ;
                sar     EDX, QMIXER_VOLUMESMOOTH        ;1/32 closer
                mov     [SmoothVolR], EBX               ;
                shr     EBX, 7                          ;
                add     EDX, [SmoothVolL]               ;
                mov     [SmoothVolL], EDX               ;
                mov     EBP, EAX                        ;
                shr     EDX, 7                          ;

                imul    EAX, EDX                        ;

                imul    EBP, EBX                        ;

                add     [EDI-8], EAX                    ;
                add     [EDI-8+4], EBP                  ;
%endmacro

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;pos adds are already defined. EBP contains volume for left channel
;and ECX for right.
%macro          q_mixloop16 1                          ; routine
                mov     [leftvol], EBP                 ;Save volumes
                mov     [rightvol], ECX                ;

                mov     ESI, [EBX + Chn_Pos]           ;ESI:ECX = pos. ESI is integer
                shr     ESI, 1
                mov     CX, [EBX + Chn_FractPos]       ;part and ECX fraction.
                shl     ECX, 16

                mov     EAX, [EBX + Chn_End]           ;Get sample end and repeat
                shr     EAX, 1
                mov     [smpend], EAX                  ;positions. Calculate subtract
                mov     EAX, [EBX + Chn_End]
                sub     EAX, [EBX + Chn_Repeat]        ;value.
                shr     EAX, 1
                mov     [smpsubtract], EAX

                mov     EDI, [dptr]                    ;Where to mix to?
                mov     EAX, [samples]                 ;How many samples to mix?
                mov     [loopcount], EAX

                ;Click removing for volume change and sample replace

                mov     EAX, [EBX + Chn_Pos]           ;Is it a new sound?
                cmp     EAX, [EBX + Chn_PrevPos]
                je      near %%soonmixloop             ;jump if not
;New sound
                mov     EDX, [leftvol]
%IFDEF QMIXER_STARTMUTE
                shr     EDX, QMIXER_STARTMUTE          ;Just to remove low frequency clicks
%ENDIF
                mov     [SmoothVolL], EDX
                mov     EDX, [rightvol]
%IFDEF QMIXER_STARTMUTE
                shr     EDX, QMIXER_STARTMUTE
%ENDIF
                mov     [SmoothVolR], EDX
                mov     [saved_reg], EBX
                %1                                     ; routine macro parameter
                mov     EBX, [saved_reg]
                mov     EDX, [EBX + Chn_LastValL]
                mov     [saved_reg], EAX
                sub     EDX, EAX
                mov     EAX, [_judas_zladdbuffer]      ;Get correct place at zladdbuf
                sub     EAX, [_judas_clipbuffer]       ;
                add     [EAX+EDI-8], EDX
                mov     EDX, [EBX + Chn_LastValR]
                sub     EDX, EBP
                add     [EAX+EDI-8+4], EDX
                mov     EAX, [saved_reg]               ;Maybe needed if sample ends now
                mov     [saved_reg], EBX
                jmp     near %%donezladjust

%%soonmixloop:  mov     EDX, [EBX + Chn_SmoothVolL]
                mov     EBP, [EBX + Chn_SmoothVolR]
                mov     [SmoothVolL], EDX
                mov     [SmoothVolR], EBP
                mov     [saved_reg], EBX

                align 16

%%mixloop:      %1                                     ; routine macro parameter - Mix one sample
%%donezladjust:
                cmp     ESI, [smpend]                  ;End of sample?
                jae     %%hitend

                dec     dword [loopcount]              ;Still shit to do?
                jnz     near %%mixloop

                mov     EBX, [saved_reg]
                shl     ESI, 1
                mov     [EBX + Chn_Pos], ESI           ;No, all done. Save position
                mov     [EBX + Chn_PrevPos], ESI       ;...and prevpos
                shr     ECX, 16
                mov     [EBX + Chn_FractPos], CX
                mov     [EBX + Chn_LastValL], EAX      ;Save Last values also
                mov     [EBX + Chn_LastValR], EBP      ;
                mov     EAX, [SmoothVolL]              ;Save volume
                mov     EBP, [SmoothVolR]              ;
                mov     [EBX + Chn_SmoothVolL], EAX    ;
                mov     [EBX + Chn_SmoothVolR], EBP    ;
                ret

%%hitend:
                mov     EBX, [saved_reg]
                test    byte [EBX + Chn_VoiceMode], VM_LOOP    ;Is it a looped sample?
                jz      %%oneshot
%%subloop:
                sub     ESI, [smpsubtract]             ;Looped. Go to loop start.
                cmp     ESI, [smpend]
                jae     %%subloop                      ;Fucking shit, it got far beyond sample end.

                dec     dword [loopcount]              ;Still shit to do?
                jnz     near %%mixloop

                shl     ESI, 1
                mov     [EBX + Chn_Pos], ESI           ;No, all done. Save position
                mov     [EBX + Chn_PrevPos], ESI       ;Save prevpos too
                shr     ECX, 16
                mov     [EBX + Chn_FractPos], CX
                mov     [EBX + Chn_LastValL], EAX      ;...Last values also
                mov     [EBX + Chn_LastValR], EBP      ;
                mov     EAX, [SmoothVolL]              ;Save volume
                mov     EBP, [SmoothVolR]              ;
                mov     [EBX + Chn_SmoothVolL], EAX    ;
                mov     [EBX + Chn_SmoothVolR], EBP    ;
                ret

%%oneshot:
                mov     byte [EBX + Chn_VoiceMode], VM_OFF ;Your time to die, sample!

                ;If sample doesn't end at zero, zerolevel must be adjusted.
                mov     dword [EBX + Chn_Pos], 0
                mov     dword [EBX + Chn_LastValL], 0
                mov     dword [EBX + Chn_LastValR], 0
                cmp     dword [loopcount], 1
                jbe     %%oneshot_lastsample
                mov     EDX, EDI                       ;Get correct place in zladdbuf
                sub     EDX, [_judas_clipbuffer]       ;
                add     EDX, [_judas_zladdbuffer]      ;
                add     [EDX], EAX
                add     [EDX+4], EBP
                mov     byte [EBX + Chn_PrevVM], VM_OFF ;No need to update zerolevel
                ret                                    ;at beginning of next chunk

%%oneshot_lastsample:                                  ;Last sample in mixed chunk
                mov     byte [EBX + Chn_PrevVM], VM_ON
                ret
%endmacro

        ;Qualitymixes [samples] of channel [cptr] to buffer at [dptr]. Destroys
        ;every register. LINEAR INTERPOLATION!

qmixchannel_linear:
                mov     EBX, [cptr]
                mov     AL, [EBX + Chn_VoiceMode]
                test    AL, VM_ON
                jnz     qmixc_l_vm_on
                cmp     AL, [EBX + Chn_PrevVM]         ;Sound discontinuity?
                je      near qmixc_l_quit              ;no?
                mov     [EBX + Chn_PrevVM], AL         ;yes...
                mov     EDX, [_judas_zladdbuffer]      ;so must move zerolevel
                add     EDX, [dptr]
                mov     EAX, [EBX + Chn_LastValL]
                sub     EDX, [_judas_clipbuffer]
                mov     dword [EBX + Chn_LastValL], 0
                add     [EDX], EAX
                mov     EAX, [EBX + Chn_LastValR]
                mov     dword [EBX + Chn_LastValR], 0
                add     [EDX+4], EAX
                jmp     qmixc_l_quit
qmixc_l_vm_on:
                mov     EAX, [EBX + Chn_Freq]
                cmp     EAX, 535232                     ;Highest linear freq
                jbe     qmixc_l_freqok
                mov     EAX, 535232
qmixc_l_freqok:
                mov     EDX, EAX
                shr     EDX, 16
                shl     EAX, 16
                div     dword [_judas_mixrate]
                mov     word [fractadd + 2], AX
                shr     EAX, 16
                mov     [integeradd], EAX
                test    byte [EBX + Chn_VoiceMode], VM_16BIT   ;Is sampledata 16bit?
                jnz     near qmixc_l_16bit               ;Jump if yes.
                test    byte [_judas_mixmode], STEREO    ;No, 8bit. Stereo audio?
                jz      qmixc_l_mono_8bit                ;Jump if mono.
qmixc_l_stereo_8bit:
qmixc_l_mono_8bit:        ;Mix in stereo, even if mono output
                q_volpan14
                q_mixloop8 q_mix8_stereo_l
                ret
qmixc_l_16bit:
                test    byte [_judas_mixmode], STEREO    ;It's 16bit. Stereo?
                jz      qmixc_l_mono_16bit
qmixc_l_stereo_16bit:
qmixc_l_mono_16bit:        ;Mix in stereo, even if mono output
                q_volpan14
                q_mixloop16 q_mix16_stereo_l
qmixc_l_quit:     ret

        ;Qualitymixes [samples] of channel [cptr] to buffer at [dptr]. Destroys
        ;every register. CUBIC INTERPOLATION!

qmixchannel_cubic:
                mov     EBX, [cptr]
                mov     AL, [EBX + Chn_VoiceMode]
                test    AL, VM_ON
                jnz     qmixc_c_vm_on
                cmp     AL, [EBX + Chn_PrevVM]           ;Sound discontinuity?
                je      near qmixc_c_quit                ;no?
                mov     [EBX + Chn_PrevVM], AL           ;yes...
                mov     EDX, [_judas_zladdbuffer]        ;so must move zerolevel
                add     EDX, [dptr]
                mov     EAX, [EBX + Chn_LastValL]
                sub     EDX, [_judas_clipbuffer]
                mov     dword [EBX + Chn_LastValL], 0
                add     [EDX], EAX
                mov     EAX, [EBX + Chn_LastValR]
                mov     dword [EBX + Chn_LastValR], 0
                add     [EDX+4], EAX
                jmp     qmixc_c_quit
qmixc_c_vm_on:
                mov     [EBX + Chn_PrevVM], AL
                mov     EAX, [EBX + Chn_Freq]
                cmp     EAX, 535232                      ;Highest linear freq
                jbe     qmixc_c_freqok
                mov     EAX, 535232
qmixc_c_freqok:
                mov     EDX, EAX
                shr     EDX, 16
                shl     EAX, 16
                div     dword [_judas_mixrate]
                mov     word [fractadd + 2], AX
                shr     EAX, 16
                mov     [integeradd], EAX
                test    byte [EBX + Chn_VoiceMode], VM_16BIT   ;Is sampledata 16bit?
                jnz     near qmixc_c_16bit               ;Jump if yes.
                test    byte [_judas_mixmode], STEREO    ;No, 8bit. Stereo audio?
                jz      qmixc_c_mono_8bit                ;Jump if mono.
qmixc_c_stereo_8bit:
qmixc_c_mono_8bit:        ;Mix in stereo, even if mono output
                q_volpan14
                q_mixloop8 q_mix8_stereo_c
                ret
qmixc_c_16bit:
                test    byte [_judas_mixmode], STEREO    ;It's 16bit. Stereo?
                jz      qmixc_c_mono_16bit
qmixc_c_stereo_16bit:
qmixc_c_mono_16bit:        ;Mix in stereo, even if mono output
                q_volpan14
                q_mixloop16 q_mix16_stereo_c
qmixc_c_quit:   ret

;Safety mixer for calls from c program

safemixer_:
_safemixer:
                cmp     dword [_judas_mixersys], 0
                je      safemixer_helldontcall
                cmp     byte [_judas_initialized], 0
                je      safemixer_helldontcall
                pushad
                call    dword [_judas_mixersys]
                popad
safemixer_helldontcall:
                ret

judas_code_lock_end_:
_judas_code_lock_end:
;                end
