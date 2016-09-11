/*
 * Timer handler, calls a user function at the desired PIT clock frequency.
 *
 * Doesn't "officially" belong to JUDAS but feel free to use! This is
 * blasphemy-ware too!
 */

#include <conio.h>
#include <dos.h>
#include "judasmem.h"
#ifdef __DJGPP__
#include <go32.h>
#include <dpmi.h>
#endif

int timer_init(unsigned short frequency, void (*function)());
void timer_uninit(void);
static int timer_lock(void);
static void timer_unlock(void);
unsigned short timer_get_ds(void);
#ifdef __WATCOMC__
void __interrupt __far timer_handler(void);
#else
void timer_handler(void);
#endif

#ifdef __DJGPP__
/* DJGPP version */
inline unsigned short timer_get_ds(void) {
        return (unsigned short)_my_ds();
}

#else
/* WATCOM C++ version */
#pragma aux timer_get_ds = \
"mov ax, ds" \
modify [ax] \
value [ax];
#endif


static unsigned char timer_initialized = 0;
#ifdef __DJGPP__
/* DJGPP version
    __dpmi_paddr structure contains (unsigned long  offset32, unsigned short selector) */
extern __dpmi_paddr timer_oldvect;
static __dpmi_paddr timer_newvect;
 
#else
/* WATCOM C++ version */
extern void (__interrupt __far *timer_oldvect)();
static void (__interrupt __far *timer_newvect)();
#endif

extern void (*timer_function)();
extern unsigned timer_count;
extern unsigned short timer_frequency;
extern unsigned short timer_systemcount;
extern unsigned short timer_ds;
extern int timer_code_lock_start;
extern int timer_code_lock_end;


int timer_init(unsigned short frequency, void (*function)())
{
	if (timer_initialized) return 1;
        if (!timer_lock()) return 0;
        timer_function = function;
        timer_count = 0;
        timer_systemcount = 0;
        timer_frequency = frequency;
        timer_ds = timer_get_ds();

        #ifdef __DJGPP__
        /* DJGPP version */
        timer_newvect.offset32 = (unsigned long) &timer_handler;
        timer_newvect.selector = (unsigned short) _my_cs();
        __dpmi_get_protected_mode_interrupt_vector(8, &timer_oldvect);
        _disable();
        __dpmi_set_protected_mode_interrupt_vector(8, &timer_newvect);

		#else
        /* WATCOM C++ version */
        timer_newvect = &timer_handler;
        timer_oldvect = _dos_getvect(8);
        _disable();
        _dos_setvect(8, timer_newvect);
        #endif
          
        outp(0x43, 0x34);
        outp(0x40, frequency);
        outp(0x40, frequency >> 8);
        _enable();
        timer_initialized = 1;
        return 1;
}

void timer_uninit(void)
{
	if (!timer_initialized) return;
        _disable();

        #ifdef __DJGPP__
        /* DJGPP version */
        __dpmi_set_protected_mode_interrupt_vector(8, &timer_oldvect);

		#else
        /* WATCOM C++ version */
        _dos_setvect(8, timer_oldvect);
        #endif

        outp(0x43, 0x34);
        outp(0x40, 0x00);
        outp(0x40, 0x00);
        _enable();
        timer_unlock();
        timer_initialized = 0;
}

static int timer_lock(void)
{
        if (!judas_memlock(&timer_code_lock_start, (int)&timer_code_lock_end - (int)&timer_code_lock_start)) return 0;
        return 1;
}

static void timer_unlock(void)
{
        judas_memunlock(&timer_code_lock_start, (int)&timer_code_lock_end - (int)&timer_code_lock_start);
}
