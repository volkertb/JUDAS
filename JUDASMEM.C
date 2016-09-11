/*
 * JUDAS DPMI memory locking functions. Includes also functions to
 * allocate/free locked memory. Feel free to use in your own proggys.
 */

#define FALSE          0
#define TRUE           1
#define NULL           0

/*
 * Borland / Watcom REGS structure compatibility
 */
#ifdef __DJGPP__
#define _BORLAND_DOS_REGS
#endif

#include <stdlib.h>
#include <mem.h>
#include <dos.h>
#include <stdio.h>   // temporary
#include <string.h>
#ifdef __DJGPP__
#include <dpmi.h>
#include <go32.h>
#include <sys/nearptr.h>
#include <sys/farptr.h>
#endif

int judas_memlock(void *start, unsigned size);
int judas_memunlock(void *start, unsigned size);
void *locked_malloc(int size);
void locked_free(void *address);
void *dos_malloc(int size);
void dos_free(void *address);
int DPMI_MapMemory (unsigned long *physaddress, unsigned long *linaddress, unsigned long size);
int DPMI_UnmapMemory (unsigned long *linaddress);


/*
 * We allocate 4 bytes more and store the block size before the actual block
 * passed to user. Afterwards, when freeing, we know the size (needed in
 * unlocking!)
 */
void *locked_malloc(int size)
{
        unsigned *block = malloc(size + sizeof(unsigned));
        if (!block) return NULL;
        *block = size;
        if (!judas_memlock(block, size + sizeof(unsigned)))
        {
                free(block);
                return NULL;
        }
        block++;
        return block;
}

void locked_free(void *address)
{
        unsigned *block;

        if (!address) return;
        block = (unsigned *)address - 1;
        judas_memunlock(block, *block);
        free(block);
}

int judas_memlock(void *start, unsigned size)
{
        unsigned *address;

        #ifdef __DJGPP__
        __dpmi_meminfo dpmimeminfo;

        __dpmi_get_segment_base_address(_my_ds(), (unsigned long *)&address);
        address = (unsigned *)((unsigned char*)address + (unsigned)start);
        dpmimeminfo.address = (unsigned long)address;
        dpmimeminfo.size = (unsigned long)size;
        if (__dpmi_lock_linear_region(&dpmimeminfo) == -1) return 0;
        else return 1;


        #else
        union REGS glenregs;

        address = (unsigned *)start;
        memset(&glenregs, 0, sizeof(glenregs));
        glenregs.w.ax = 0x600;
        glenregs.w.bx = (unsigned)address >> 16;
        glenregs.w.cx = (unsigned)address & 0xffff;
        glenregs.w.si = size >> 16;
        glenregs.w.di = size & 0xffff;
        int386(0x31, &glenregs, &glenregs);
        if (glenregs.w.cflag) return 0;
        return 1;
        #endif
}

int judas_memunlock(void *start, unsigned size)
{
        unsigned *address;

        #ifdef __DJGPP__
        __dpmi_meminfo dpmimeminfo;

        __dpmi_get_segment_base_address(_my_ds(), (unsigned long *)&address);
        address = (unsigned *)((unsigned char*)address + (unsigned)start);
        dpmimeminfo.address = (unsigned long)address;
        dpmimeminfo.size = (unsigned long)size;
        if (__dpmi_unlock_linear_region(&dpmimeminfo) == -1) return 0;
        else return 1;

        #else
        union REGS glenregs;

        address = (unsigned *)start;
        memset(&glenregs, 0, sizeof(glenregs));
        glenregs.w.ax = 0x601;
        glenregs.w.bx = (unsigned)address >> 16;
        glenregs.w.cx = (unsigned)address & 0xffff;
        glenregs.w.si = size >> 16;
        glenregs.w.di = size & 0xffff;
        int386(0x31, &glenregs, &glenregs);
        if (glenregs.w.cflag) return 0;
        return 1;
        #endif
}

void *dos_malloc(int size)
{
        unsigned *address;

        #ifdef __DJGPP__
        int selector;

        /* allocate 16 bytes more to store selector of allocated block  and align black on para */
        if (__dpmi_allocate_dos_memory(((size + 16) >> 4) + 1, &selector) == -1) return 0;
        __dpmi_get_segment_base_address(selector, (unsigned long *)&address);
        _farpokel (_dos_ds, (unsigned) address, selector);
        address += 4;
        return address;

        #else
        union REGS glenregs;

        /* allocate 16 bytes more to store selector of allocated block and align block on parag */
        memset(&glenregs, 0, sizeof(glenregs));
        glenregs.w.ax = 0x100;
        glenregs.w.bx = (unsigned) (((size + 16) >> 4) + 1);
        int386(0x31, &glenregs, &glenregs);
        if (glenregs.w.cflag) return 0;
        address = (unsigned *) (glenregs.w.ax << 4);
        *address = (unsigned) glenregs.w.dx;
        address += 4;
        return address;
        #endif
}

void dos_free(void *address)
{
        #ifdef __DJGPP__
        __dpmi_free_dos_memory (_farpeekl (_dos_ds, (unsigned) ((unsigned *)address - 4) ) );

        #else
        union REGS glenregs;

        memset(&glenregs, 0, sizeof(glenregs));
        glenregs.w.ax = 0x101;
        glenregs.w.dx = (unsigned) *((unsigned *)address - 4);
        int386(0x31, &glenregs, &glenregs);
        #endif
}

int DPMI_MapMemory (unsigned long *physaddress, unsigned long *linaddress, unsigned long size)
{
        #ifdef __DJGPP__
        __dpmi_meminfo dpmimeminfo;
        unsigned *address;

        __dpmi_get_segment_base_address(_my_ds(), (unsigned long *)&address);
//        printf ("DS base Address = %#06x\n", (unsigned int)address);

        dpmimeminfo.address = (unsigned long)*physaddress;
        dpmimeminfo.size = (unsigned long)size;
//        printf ("Physical Address = %#06x\n", (unsigned int)dpmimeminfo.address);
        if (__dpmi_physical_address_mapping (&dpmimeminfo) == -1) {
            *linaddress = 0;
            return FALSE;
        }
//        printf ("Linear Address = %#06x\n", (unsigned int)dpmimeminfo.address);
        dpmimeminfo.address -= (unsigned long)address;
//        printf ("Final Address = %#06x\n", (unsigned int)dpmimeminfo.address);
        *linaddress = (unsigned long)dpmimeminfo.address;
        return TRUE;

        #else
        union REGS r;

        memset(&r, 0, sizeof(r));
        r.x.ebx = (*physaddress) >> 16;
        r.x.ecx = (*physaddress);
        r.x.esi = size >> 16;
        r.x.edi = size;
        r.x.eax = 0x800;
        int386 (0x31, &r, &r);
        if (r.x.cflag) {
            *linaddress = 0;
            return FALSE;
        }
        *linaddress = (r.w.bx << 16) + r.w.cx;
        return TRUE;
        #endif
}

int DPMI_UnmapMemory (unsigned long *linaddress)
{
        #ifdef __DJGPP__
        __dpmi_meminfo dpmimeminfo;
        unsigned *address;

        __dpmi_get_segment_base_address(_my_ds(), (unsigned long *)&address);
        memset(&dpmimeminfo, 0, sizeof(dpmimeminfo));
        dpmimeminfo.address = (unsigned long)*linaddress;
        dpmimeminfo.address += (unsigned long)address;
        if (__dpmi_free_physical_address_mapping (&dpmimeminfo) == -1) return FALSE;
        return TRUE;

        #else
        union REGS r;

        memset(&r, 0, sizeof(r));
        r.x.ebx = (*linaddress) >> 16;
        r.x.ecx = (*linaddress);
        r.x.eax = 0x801;
        int386 (0x31, &r, &r);
        if (r.x.cflag) return FALSE;
        return TRUE;
        #endif
}
