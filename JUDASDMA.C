/*
 * JUDAS DMA allocation.
 */

 /*
 * Borland / Watcom REGS structure compatibility
 */
#ifdef __DJGPP__
#include <dpmi.h>
#include <go32.h>
#include <sys/nearptr.h>
#include <sys/farptr.h>
#define _BORLAND_DOS_REGS
#endif

#include <dos.h>
#include <conio.h>
#include "judasmem.h"

#define DMA_MAXSIZE 65536

int dma_reserve(int size);

unsigned dma_address;
static char dma_initialized = 0;

int dma_reserve(int size)
{
        #ifdef __DJGPP__
        int selector;
        #else
        static union REGS glenregs;
        #endif
        
        if (dma_initialized) return 1;

        /* Round size upward to paragraph limit */
        size += 15;
        size &= 0xfffffff0;

        /* Limit size to maximum */
        if (size > DMA_MAXSIZE) size = DMA_MAXSIZE;

        /* Buffer address will be used from interrupt, lock it! */
        if (!judas_memlock(&dma_address, sizeof (dma_address))) return 0;

        /* Use DPMI functions because _dos_allocmem() doesn't work reliably */
        #ifdef __DJGPP__
        if (__dpmi_allocate_dos_memory((size * 2) >> 4, &selector) == -1) return 0;
        __dpmi_get_segment_base_address(selector, (unsigned long *)&dma_address);

        #else
        glenregs.w.ax = 0x100;
        glenregs.w.bx = (size * 2) >> 4;
        int386(0x31, &glenregs, &glenregs);
        if (glenregs.w.cflag) return 0;
        glenregs.w.ax = 0x6;
        glenregs.w.bx = glenregs.w.dx;
        int386(0x31, &glenregs, &glenregs);
        if (glenregs.w.cflag) return 0;
        dma_address = glenregs.w.cx << 16 | glenregs.w.dx;
        #endif
        
        /* Check for DMA page wrap */
        if ((dma_address & 0xffff) > (0x10000 - size))
        {
                dma_address += 65535;
                dma_address &= 0xffff0000;
        }

        dma_initialized = 1;
        return 1;
}
