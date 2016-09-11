
#include <dos.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <time.h>
#include <math.h>
#include "judasac.h"
#include "dos32\dpmi.h"
#include "dos32\input.h"
#include "dos32\scans.h"
#include "dos32\timer.h"


extern BYTE  pci_config_read_byte(AC97_PCI_DEV *ac97_pci, int index);
extern WORD  pci_config_read_word(AC97_PCI_DEV *ac97_pci, int index);
extern DWORD pci_config_read_dword(AC97_PCI_DEV *ac97_pci, int index);
extern void  pci_config_write_byte(AC97_PCI_DEV *ac97_pci, int index, BYTE data);
extern void  pci_config_write_word(AC97_PCI_DEV *ac97_pci, int index, WORD data);
extern void  pci_config_write_dword(AC97_PCI_DEV *ac97_pci, int index, DWORD data);
extern BOOL  pci_check_bios(void);
extern BOOL  pci_find_device(AC97_PCI_DEV *ac97_pci);
extern void  pci_enable_io_access(AC97_PCI_DEV *ac97_pci);
extern void  pci_enable_memory_access(AC97_PCI_DEV *ac97_pci);
extern void  pci_enable_busmaster(AC97_PCI_DEV *ac97_pci);
extern BOOL detect_windows(void);
extern BOOL detect_os2(void);
extern BOOL detect_linux(void);


/********************************************************************
 *      Intel PCI BIOS helper funtions
 ********************************************************************/

#ifndef PCI_ANY_ID
#define PCI_ANY_ID      ((WORD)(~0))
#endif

BYTE pci_config_read_byte(AC97_PCI_DEV *ac97_pci, int index)
{
        union REGS r;

        memset(&r, 0, sizeof(r));
        r.x.eax = 0x0000B108;                // config read byte
        r.x.ebx = (DWORD)ac97_pci->device_bus_number;
        r.x.edi = (DWORD)index;
        int386(0x1a, &r, &r);
        if (r.h.ah != 0) {
                #ifdef AC97_DEBUG
                logmessage("Error : PCI read config byte failed\n");
                #endif
                r.x.ecx = 0;
        }
        return (BYTE)r.x.ecx;
}

WORD pci_config_read_word(AC97_PCI_DEV *ac97_pci, int index)
{
        union REGS r;

        memset(&r, 0, sizeof(r));
        r.x.eax = 0x0000B109;                // config read word
        r.x.ebx = (DWORD)ac97_pci->device_bus_number;
        r.x.edi = (DWORD)index;
        int386(0x1a, &r, &r);
        if (r.h.ah != 0 ){
                #ifdef AC97_DEBUG
                logmessage("Error : PCI read config word failed\n");
                #endif
                r.x.ecx = 0;
        }
        return (WORD)r.x.ecx;
}

DWORD pci_config_read_dword(AC97_PCI_DEV *ac97_pci, int index)
{
        union REGS r;

        memset(&r, 0, sizeof(r));
        r.x.eax = 0x0000B10A;                // config read dword
        r.x.ebx = (DWORD)ac97_pci->device_bus_number;
        r.x.edi = (DWORD)index;
        int386(0x1a, &r, &r);
        if (r.h.ah != 0 ){
                #ifdef AC97_DEBUG
                logmessage("Error : PCI read config dword failed\n");
                #endif
                r.x.ecx = 0;
        }
        return (DWORD)r.x.ecx;
}

void pci_config_write_byte(AC97_PCI_DEV *ac97_pci, int index, BYTE data)
{
        union REGS r;

        memset(&r, 0, sizeof(r));
        r.x.eax = 0x0000B10B;                // config write byte
        r.x.ebx = (DWORD)ac97_pci->device_bus_number;
        r.x.ecx = (DWORD)data;
        r.x.edi = (DWORD)index;
        int386(0x1a, &r, &r);
        if (r.h.ah != 0 ){
                #ifdef AC97_DEBUG
                logmessage("Error : PCI write config byte failed\n");
                #endif
        }
}

void pci_config_write_word(AC97_PCI_DEV *ac97_pci, int index, WORD data)
{
        union REGS r;

        memset(&r, 0, sizeof(r));
        r.x.eax = 0x0000B10C;                // config write word
        r.x.ebx = (DWORD)ac97_pci->device_bus_number;
        r.x.ecx = (DWORD)data;
        r.x.edi = (DWORD)index;
        int386(0x1a, &r, &r);
        if (r.h.ah != 0 ){
                #ifdef AC97_DEBUG
                logmessage("Error : PCI write config word failed\n");
                #endif
        }
}

void pci_config_write_dword(AC97_PCI_DEV *ac97_pci, int index, DWORD data)
{
        union REGS r;

        memset(&r, 0, sizeof(r));
        r.x.eax = 0x0000B10D;                // config write dword
        r.x.ebx = (DWORD)ac97_pci->device_bus_number;
        r.x.ecx = (DWORD)data;
        r.x.edi = (DWORD)index;
        int386(0x1a, &r, &r);
        if (r.h.ah != 0 ){
                #ifdef AC97_DEBUG
                logmessage("Error : PCI write config dword failed\n");
                #endif
        }
}

BOOL pci_check_bios(void)
{
        union REGS r;

        memset(&r, 0, sizeof(r));
        r.x.eax = 0x0000B101;                // PCI BIOS - installation check
        r.x.edi = 0x00000000;
        int386(0x1a, &r, &r);
        if (r.x.edx != 0x20494350) return FALSE;   // ' ICP' identifier found ?
        return TRUE;
}

BOOL pci_find_device(AC97_PCI_DEV *ac97_pci)
{
        union REGS r;

        memset(&r, 0, sizeof(r));
        r.x.eax = 0x0000B102;                   // PCI BIOS - find PCI device
        r.x.ecx = ac97_pci->device_id;          // device ID
        r.x.edx = ac97_pci->vender_id;          // vender ID
        r.x.esi = 0x00000000;                   // device index
        int386(0x1a, &r, &r);
        if (r.h.ah != 0 ) return FALSE;         // device not found
        ac97_pci->device_bus_number = r.w.bx;   // save device & bus/funct number
        if(ac97_pci->sub_vender_id != PCI_ANY_ID){
                // check subsystem vender id
                if(pci_config_read_word(ac97_pci, 0x2C) != ac97_pci->sub_vender_id) return FALSE;
        }
        if(ac97_pci->sub_device_id != PCI_ANY_ID){
                // check subsystem device id
                if(pci_config_read_word(ac97_pci, 0x2E) != ac97_pci->sub_device_id) return FALSE;
        }
        return TRUE;                             // device found
}

void pci_enable_io_access(AC97_PCI_DEV *ac97_pci)
{
        pci_config_write_word(ac97_pci, 0x04, pci_config_read_word(ac97_pci, 0x04) | BIT0);
}

void pci_enable_memory_access(AC97_PCI_DEV *ac97_pci)
{
        pci_config_write_word(ac97_pci, 0x04, pci_config_read_word(ac97_pci, 0x04) | BIT1);
}

void pci_enable_busmaster(AC97_PCI_DEV *ac97_pci)
{
        pci_config_write_word(ac97_pci, 0x04, pci_config_read_word(ac97_pci, 0x04) | BIT2);
}


/********************************************************************
 *    Windows/OS2/Linux detection helper functions
 ********************************************************************/

BOOL detect_windows(void)   // Win 3.1+ detection
{
        union REGS r;

        memset(&r, 0, sizeof(r));
        r.x.eax = 0x1600;
        int386(0x2F, &r, &r);
        if ((r.h.al & 0x7F) != 0) return TRUE;
        return FALSE;
}

BOOL detect_os2(void)   // OS2 2.0+ detection
{
        union REGS r;

        memset(&r, 0, sizeof(r));
        r.x.eax = 0x4010;
        int386(0x2F, &r, &r);
        if (r.w.ax == 0) return TRUE;
        return FALSE;
}

BOOL detect_linux(void)   // Linux DOSEMU detection ???
{
        union REGS r;

        memset(&r, 0, sizeof(r));
        r.x.eax = 0x0200;
        r.x.ebx = 0x00E6;
        int386(0x31, &r, &r);    // segment of this vector should be 0xF000
        if (r.w.cx != 0xF000) return FALSE;

        memset(&r, 0, sizeof(r));
        r.x.eax = 0;
        int386(0xE6, &r, &r);
        if (r.w.ax == 0xAA55) return TRUE;
        return FALSE;
}
