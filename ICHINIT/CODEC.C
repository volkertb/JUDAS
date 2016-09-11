
// codec module for ICHINIT v2.0+

#include <dos.h>
#include <io.h>
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


extern BOOL codec_config (AC97_PCI_DEV *ac97_pci, int sample_rate);
extern BOOL semaphore (AC97_PCI_DEV *ac97_pci);

extern unsigned int ich_INL (AC97_PCI_DEV *ac97_pci, int base, unsigned int a);
extern unsigned short ich_INW (AC97_PCI_DEV *ac97_pci, int base, unsigned short a);
extern unsigned char ich_INB (AC97_PCI_DEV *ac97_pci, int base, unsigned char a);
extern void ich_OUTL (AC97_PCI_DEV *ac97_pci, unsigned int d, int base, unsigned int a);
extern void ich_OUTW (AC97_PCI_DEV *ac97_pci, unsigned short d, int base, unsigned short a);
extern void ich_OUTB (AC97_PCI_DEV *ac97_pci, unsigned char d, int base, unsigned char a);

void codec_stop (AC97_PCI_DEV *ac97_pci);
void set_volume (AC97_PCI_DEV *ac97_pci, int volume);


unsigned int ich_INL (AC97_PCI_DEV *ac97_pci, int base, unsigned int a)
{
    if (ac97_pci->mem_mode == 1) {
        if (base == CTL_BASE) return *(unsigned int *) (ac97_pci->base3 + (a));   // bus master base memory
        else return *(unsigned int *) (ac97_pci->base2 + (a));                    // audio mixer base memory
    } else {
        if (base == CTL_BASE) return inpd (ac97_pci->base1 + a);                  // bus master base IO
        else return inpd (ac97_pci->base0 + a);                                   // audio mixer base IO
    }
}

unsigned short ich_INW (AC97_PCI_DEV *ac97_pci, int base, unsigned short a)
{
    if (ac97_pci->mem_mode == 1) {
        if (base == CTL_BASE) return *(unsigned short *) (ac97_pci->base3 + (a)); // bus master base memory
        else return *(unsigned short *) (ac97_pci->base2 + (a));                  // audio mixer base memory
    } else {
        if (base == CTL_BASE) return inpw (ac97_pci->base1 + a);                  // bus master base IO
        else return inpw (ac97_pci->base0 + a);                                   // audio mixer base IO
    }
}

unsigned char ich_INB (AC97_PCI_DEV *ac97_pci, int base, unsigned char a)
{
    if (ac97_pci->mem_mode == 1) {
        if (base == CTL_BASE) return *(unsigned char *) (ac97_pci->base3 + (a));  // bus master base memory
        else return *(unsigned char *) (ac97_pci->base2 + (a));                   // audio mixer base memory
    } else {
        if (base == CTL_BASE) return inp (ac97_pci->base1 + a);                   // bus master base IO
        else return inp (ac97_pci->base0 + a);                                    // audio mixer base IO
    }
}

void ich_OUTL (AC97_PCI_DEV *ac97_pci, unsigned int d, int base, unsigned int a)
{
    if (ac97_pci->mem_mode == 1) {
        if (base == CTL_BASE) *(unsigned int *) (ac97_pci->base3 + (a)) = d;      // bus master base memory
        else *(unsigned int *) (ac97_pci->base2 + (a)) = d;                       // audio mixer base memory
    } else {
        if (base == CTL_BASE) outpd (ac97_pci->base1 + a, d);                     // bus master base IO
        else outpd (ac97_pci->base0 + a, d);                                      // audio mixer base IO
    }
}

void ich_OUTW (AC97_PCI_DEV *ac97_pci, unsigned short d, int base, unsigned short a)
{
    if (ac97_pci->mem_mode == 1) {
        if (base == CTL_BASE) *(unsigned short *) (ac97_pci->base3 + (a)) = d;    // bus master base memory
        else *(unsigned short *) (ac97_pci->base2 + (a)) = d;                     // audio mixer base memory
    } else {
        if (base == CTL_BASE) outpw (ac97_pci->base1 + a, d);                     // bus master base IO
        else outpw (ac97_pci->base0 + a, d);                                      // audio mixer base IO
    }
}

void ich_OUTB (AC97_PCI_DEV *ac97_pci, unsigned char d, int base, unsigned char a)
{
    if (ac97_pci->mem_mode == 1) {
        if (base == CTL_BASE) *(unsigned char *) (ac97_pci->base3 + (a)) = d;     // bus master base memory
        else *(unsigned char *) (ac97_pci->base2 + (a)) = d;                      // audio mixer base memory
    } else {
        if (base == CTL_BASE) outp (ac97_pci->base1 + a, d);                      // bus master base IO
        else outp (ac97_pci->base0 + a, d);                                       // audio mixer base IO
    }
}

// stop the codec
void codec_stop (AC97_PCI_DEV *ac97_pci)
{
    // stop all PCM out data
    ich_OUTB (ac97_pci, 0, CTL_BASE, ICH_REG_PO_CR);                              // 1Bh control register at NABMBAR
    delay (50);                                                                   // 50ms delay

    // reset PCM out registers
    ich_OUTB (ac97_pci, ICH_RESETREGS, CTL_BASE, ICH_REG_PO_CR);                  // 1Bh control register at NABMBAR
    delay (50);                                                                   // 50ms delay
}

// wait until codec is reay - returns status (TRUE or FALSE)
BOOL semaphore (AC97_PCI_DEV *ac97_pci)
{
    DWORD flags = 0;
    int i = 0;

    flags = ich_INL (ac97_pci, CTL_BASE, ICH_REG_GLOB_STA);                       // 30h global status register at NABMBAR
    if ((flags & ICH_PCR) == 0) return TRUE;                                      // exit with success if primary codec not ready !

    for (i = 0; i < 0xffff; i++) {
        flags = ich_INB (ac97_pci, CTL_BASE, ICH_REG_ACC_SEMA);                   // 34h codec write semaphore register at NABMBAR
        if ((flags & ICH_CAS) == 0) return TRUE;                                  // exit if codec not busy!
    }

    return FALSE;                                                                 // exit with failure
}

// set voulme to volume level
void set_volume (AC97_PCI_DEV *ac97_pci, int volume)
{
    semaphore (ac97_pci);
    ich_OUTW (ac97_pci, 0, MIXER_BASE, AC97_RESET);                               // register reset the codec (0)
    semaphore (ac97_pci);
    ich_OUTW (ac97_pci, volume * 0x101, MIXER_BASE, AC97_MASTER);                 // set volume for both channels (2)
    semaphore (ac97_pci);
    ich_OUTW (ac97_pci, volume * 0x101, MIXER_BASE, AC97_HEADPHONE);              // set hp volume for both channels (4)
    semaphore (ac97_pci);
    ich_OUTW (ac97_pci, volume * 0x101, MIXER_BASE, AC97_CD_VOL);                 // set CD out volume for both channels (12h)
    semaphore (ac97_pci);
    ich_OUTW (ac97_pci, volume * 0x101, MIXER_BASE, AC97_PCM);                    // set CD out volume for both channels (12h)

}

// enable codec, unmute stuff, set output to desired rate
// in = desired sample rate
// out = true or false
BOOL codec_config (AC97_PCI_DEV *ac97_pci, int sample_rate)
{
    DWORD flags = 0;
    int i = 0;

    // stop the codec if currently playing
    codec_stop (ac97_pci);

    // do a cold reset
    // enable AC Link off clear
    ich_OUTL (ac97_pci, 0, CTL_BASE, ICH_REG_GLOB_CNT);                           // 2ch global control reg at NABMBAR
    delay (50);

    // cold reset + primary resume -> 2 channels and 16 bit samples
    ich_OUTL (ac97_pci, ICH_AC97COLD + ICH_PRIE, CTL_BASE, ICH_REG_GLOB_CNT);     // 2ch global control reg at NABMBAR

    // test cold reset
    flags = ich_INL (ac97_pci, CTL_BASE, ICH_REG_GLOB_CNT);                       // 2ch global control reg at NABMBAR
    if ((flags & ICH_AC97COLD) == 0) return FALSE;

    // wait for primary codec ready status - ignore for ICH4+
    if (ac97_pci->mem_mode == 0) {                                                 // for normal IO access only
        i = 128;
        while (i) {
            flags = ich_INL (ac97_pci, CTL_BASE, ICH_REG_GLOB_STA);               // 30h global status register at NABMBAR
            if (flags & ICH_PCR) break;                                           // primary codec ready bit set
            delay (1);                                                            // 1ms delay
            i--;
        }

        // wait until codec init ready (replaces warm reset wait)
        delay (800);                                                              // delay 800ms

        // test if codec ready bit is finally set
        flags = ich_INL (ac97_pci, CTL_BASE, ICH_REG_GLOB_STA);                   // 30h global status register at NABMBAR
        if ((flags & ICH_PCR) == 0) return FALSE;                                 // primary codec ready bit not set - error
    }

    // clear semaphore flag
    flags = ich_INW (ac97_pci, MIXER_BASE, AC97_RESET);                           // register reset the codec (0)

    // check if codec sections ready
    if (semaphore (ac97_pci) == 0) return FALSE;                                  // exit with error
    flags = ich_INW (ac97_pci, MIXER_BASE, AC97_POWER_CTRL);                      // 26h powerdown control
    flags &= BIT0 + BIT1 + BIT2 +BIT3;
    if (flags != (BIT0 + BIT1 + BIT2 +BIT3)) return FALSE;                        // codec sections not ready

    // disable interrupts
    ich_OUTB (ac97_pci, 0, CTL_BASE, ICH_REG_PI_CR);                              // 0Bh control register at NABMBAR
    ich_OUTB (ac97_pci, 0, CTL_BASE, ICH_REG_PO_CR);                              // 1Bh control register at NABMBAR
    ich_OUTB (ac97_pci, 0, CTL_BASE, ICH_REG_MC_CR);                              // 2Bh control register at NABMBAR

    // reset channels
    ich_OUTB (ac97_pci, ICH_RESETREGS, CTL_BASE, ICH_REG_PI_CR);                  // 0Bh control register at NABMBAR
    ich_OUTB (ac97_pci, ICH_RESETREGS, CTL_BASE, ICH_REG_PO_CR);                  // 1Bh control register at NABMBAR
    ich_OUTB (ac97_pci, ICH_RESETREGS, CTL_BASE, ICH_REG_MC_CR);                  // 2Bh control register at NABMBAR

    // set default volume
    // set_volume (ac97_pci, 15);

    // set VRA and clear DRA (if not supported will be skipped)
    if (semaphore (ac97_pci) == 0) return FALSE;                                  // exit with error
    flags = ich_INW (ac97_pci, MIXER_BASE, AC97_EXTENDED_STATUS);                 // 2ah get extended audio ctl NAMBAR
    if (semaphore (ac97_pci) == 0) return FALSE;                                  // exit with error
    flags &= 0xffff - BIT1;
    flags |= BIT0;
    ich_OUTW (ac97_pci, flags, MIXER_BASE, AC97_EXTENDED_STATUS);                 // 2ah set extended audio ctl NAMBAR

    // set desired sample rate
    if (semaphore (ac97_pci) == 0) return FALSE;                                  // exit with error
    ich_OUTW (ac97_pci, sample_rate, MIXER_BASE, AC97_PCM_FRONT_DAC_RATE);        // 2ch set sample rate NAMBAR

    return TRUE;
}
