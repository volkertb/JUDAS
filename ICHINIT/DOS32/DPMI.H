
/*

 Header file for basic DPMI functions
 (2000-2002) Piotr Ulaszewski

*/

#ifndef __DPMI_H__
#define __DPMI_H__

#ifndef FLIP2API
#define FLIP2API _cdecl
#endif


/* DPMI regs structure for simulate real mode interrupt */
#pragma pack (push, 1);
typedef struct _DPMIREGS{
    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int reserved;
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;
    unsigned short flags;
    unsigned short es;
    unsigned short ds;
    unsigned short fs;
    unsigned short gs;
    unsigned short ip;
    unsigned short cs;
    unsigned short sp;
    unsigned short ss;
} DPMIREGS;
#pragma pack (pop);


#ifdef __cplusplus
extern "C" {
#endif


extern bool   FLIP2API DPMI_DOSmalloc (unsigned long size, unsigned short *segment, unsigned short *selector);
extern void   FLIP2API DPMI_DOSfree (unsigned short *selector);
extern void   FLIP2API DPMI_GetRMVector (unsigned char IntNum, unsigned short *segment, unsigned short *offset);
extern void   FLIP2API DPMI_SetRMVector (unsigned char IntNum, unsigned short *segment, unsigned short *offset);
extern void   FLIP2API DPMI_GetPMVector (unsigned char IntNum, unsigned short *selector, unsigned long *offset);
extern void   FLIP2API DPMI_SetPMVector (unsigned char IntNum, unsigned short *selector, unsigned long *offset);
extern bool   FLIP2API DPMI_SimulateRMI (unsigned char IntNum, DPMIREGS *regs);
extern void * FLIP2API DPMI_Malloc (unsigned long size, unsigned long *handle);
extern void   FLIP2API DPMI_Free (unsigned long *handle);
extern bool   FLIP2API DPMI_Lock (unsigned long *address, unsigned long size);
extern bool   FLIP2API DPMI_Unlock (unsigned long *address, unsigned long size);
extern bool   FLIP2API DPMI_MapMemory (unsigned long *physaddress, unsigned long *linaddress, unsigned long size);
extern bool   FLIP2API DPMI_UnmapMemory (unsigned long *linaddress);


#ifdef __cplusplus
}
#endif

#endif
