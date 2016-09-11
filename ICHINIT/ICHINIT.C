
/*****************************************************************************
 * Intel ICH AC97?HDA Audio codec initialization for real mode DOS
 * By Piotr Ulaszewski (February 2003 - August 2007)
 * piotrkn22@poczta.onet.pl
 * http://wwww.republika.pl/piotrkn22/
 * some new controllers and codec IDs added by Matthias Goeke
 * developement ver 2.0 (August 2007)
 *****************************************************************************/


#include <dos.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <time.h>
#include <math.h>
#include <graph.h>
#include "judasac.h"
#include "dos32\dpmi.h"
#include "dos32\input.h"
#include "dos32\scans.h"
#include "dos32\timer.h"


// pci.c functions for ich ac97
BYTE  pci_config_read_byte (AC97_PCI_DEV *ac97_pci, int index);
WORD  pci_config_read_word (AC97_PCI_DEV *ac97_pci, int index);
DWORD pci_config_read_dword (AC97_PCI_DEV *ac97_pci, int index);
void  pci_config_write_byte (AC97_PCI_DEV *ac97_pci, int index, BYTE data);
void  pci_config_write_word (AC97_PCI_DEV *ac97_pci, int index, WORD data);
void  pci_config_write_dword (AC97_PCI_DEV *ac97_pci, int index, DWORD data);
BOOL  pci_check_bios (void);
BOOL  pci_find_device (AC97_PCI_DEV *ac97_pci);
void  pci_enable_io_access (AC97_PCI_DEV *ac97_pci);
void  pci_enable_memory_access (AC97_PCI_DEV *ac97_pci);
void  pci_enable_busmaster (AC97_PCI_DEV *ac97_pci);
BOOL  detect_windows(void);
BOOL  detect_os2(void);
BOOL  detect_linux(void);

// codec.c functions
BOOL  codec_config (AC97_PCI_DEV *ac97_pci, int sample_rate);
BOOL  semaphore (AC97_PCI_DEV *ac97_pci);

unsigned int ich_INL (AC97_PCI_DEV *ac97_pci, int base, unsigned int a);
unsigned short ich_INW (AC97_PCI_DEV *ac97_pci, int base, unsigned short a);
unsigned char ich_INB (AC97_PCI_DEV *ac97_pci, int base, unsigned char a);
void ich_OUTL (AC97_PCI_DEV *ac97_pci, unsigned int d, int base, unsigned int a);
void ich_OUTW (AC97_PCI_DEV *ac97_pci, unsigned short d, int base, unsigned short a);
void ich_OUTB (AC97_PCI_DEV *ac97_pci, unsigned char d, int base, unsigned char a);

// prototype for cclor display color_printf - no formating / prototype for map DPMI memory region
void  color_printf (char *string, unsigned char color);
BOOL DPMI_MapMemory (unsigned long *physaddress, unsigned long *linaddress, unsigned long size);
BOOL DPMI_UnmapMemory (unsigned long *linaddress);


int     master_vol = -1;
int     hp_vol = -1;
int     mono_vol = -1;
int     wave_vol = -1;
int     beep_vol = -1;
int     phone_vol = -1;
int     mic_vol = -1;
int     line_in_vol = -1;
int     cd_vol = -1;
int     video_vol = -1;
int     aux_vol = -1;
int     a3c_vol = -1;
int     a3d_vol = -1;
int     a3p_vol = -1;
int     bass_vol = -1;
int     treble_vol = -1;
int     sstereo_vol = -1;
int     bassbst_vol = -1;

AC97_PCI_DEV ac97_pci = {0};                  //  Pci device structure


void main (int argc, char **argv)
{
    int counter = 0;
    char *strptr = NULL;
    DWORD flags = 0;


    printf ("\n");
    printf ("-----------------------------------------------------------------------\n");
    printf ("Real mode DOS initializer for Intel ICH based AC97/HDA codecs.\n");
    printf ("Developement ver 2.0 (August 2007).\n");
    printf ("Written by Piotr Ulaszewski (PETERS).\n");
    printf ("-----------------------------------------------------------------------\n");

    if (argc > 1) {
        for (counter = 1; counter < argc; counter++) {
            strptr = argv[counter];
            strptr += 4;

            // master out
            if (!strnicmp (argv[counter], "/MO:", 4)) master_vol = atoi (strptr);
            else if (!strnicmp (argv[counter], "-MO:", 4)) master_vol = atoi (strptr);

            // headphones out
            else if (!strnicmp (argv[counter], "/HO:", 4)) hp_vol = atoi (strptr);
            else if (!strnicmp (argv[counter], "-HO:", 4)) hp_vol = atoi (strptr);

            // master mono
            else if (!strnicmp (argv[counter], "/MM:", 4)) mono_vol = atoi (strptr);
            else if (!strnicmp (argv[counter], "-MM:", 4)) mono_vol = atoi (strptr);

            // wave out
            else if (!strnicmp (argv[counter], "/WO:", 4)) wave_vol = atoi (strptr);
            else if (!strnicmp (argv[counter], "-WO:", 4)) wave_vol = atoi (strptr);

            // beep out
            else if (!strnicmp (argv[counter], "/BO:", 4)) beep_vol = atoi (strptr);
            else if (!strnicmp (argv[counter], "-BO:", 4)) beep_vol = atoi (strptr);

            // phone out
            else if (!strnicmp (argv[counter], "/PO:", 4)) phone_vol = atoi (strptr);
            else if (!strnicmp (argv[counter], "-PO:", 4)) phone_vol = atoi (strptr);

            // microphone in
            else if (!strnicmp (argv[counter], "/MI:", 4)) mic_vol = atoi (strptr);
            else if (!strnicmp (argv[counter], "-MI:", 4)) mic_vol = atoi (strptr);

            // line in
            else if (!strnicmp (argv[counter], "/LI:", 4)) line_in_vol = atoi (strptr);
            else if (!strnicmp (argv[counter], "-LI:", 4)) line_in_vol = atoi (strptr);

            // cd out
            else if (!strnicmp (argv[counter], "/CD:", 4)) cd_vol = atoi (strptr);
            else if (!strnicmp (argv[counter], "-CD:", 4)) cd_vol = atoi (strptr);

            // video out
            else if (!strnicmp (argv[counter], "/VD:", 4)) video_vol = atoi (strptr);
            else if (!strnicmp (argv[counter], "-VD:", 4)) video_vol = atoi (strptr);

            // aux in
            else if (!strnicmp (argv[counter], "/AX:", 4)) aux_vol = atoi (strptr);
            else if (!strnicmp (argv[counter], "-AX:", 4)) aux_vol = atoi (strptr);

            // 3d center adjustment
            else if (!strnicmp (argv[counter], "/3C:", 4)) a3c_vol = atoi (strptr);
            else if (!strnicmp (argv[counter], "-3C:", 4)) a3c_vol = atoi (strptr);

            // 3d depth adjustment
            else if (!strnicmp (argv[counter], "/3D:", 4)) a3d_vol = atoi (strptr);
            else if (!strnicmp (argv[counter], "-3D:", 4)) a3d_vol = atoi (strptr);

            // 3d path specifier
            else if (!strnicmp (argv[counter], "/3P:", 4)) a3p_vol = atoi (strptr);
            else if (!strnicmp (argv[counter], "-3P:", 4)) a3p_vol = atoi (strptr);

            // master bass gain
            else if (!strnicmp (argv[counter], "/MB:", 4)) bass_vol = atoi (strptr);
            else if (!strnicmp (argv[counter], "-MB:", 4)) bass_vol = atoi (strptr);

            // master treble gain
            else if (!strnicmp (argv[counter], "/MT:", 4)) treble_vol = atoi (strptr);
            else if (!strnicmp (argv[counter], "-MT:", 4)) treble_vol = atoi (strptr);

            // simulated stereo
            else if (!strnicmp (argv[counter], "/SS:", 4)) sstereo_vol = atoi (strptr);
            else if (!strnicmp (argv[counter], "-SS:", 4)) sstereo_vol = atoi (strptr);

            // bass bost
            else if (!strnicmp (argv[counter], "/BB:", 4)) bassbst_vol = atoi (strptr);
            else if (!strnicmp (argv[counter], "-BB:", 4)) bassbst_vol = atoi (strptr);

            else {
                printf ("Error : Invalid parameters!\n");
                printf ("\n");
                printf ("/MO: or /mo:   master out volume\n");
                printf ("/HO: or /ho:   headphones out volume\n");
                printf ("/MM: or /mm:   master mono out volume\n");
                printf ("/WO: or /wo:   wave out volume\n");

                printf ("/BO: or /bo:   beep out volume\n");
                printf ("/PO: or /po:   phone out volume\n");

                printf ("/MI: or /mi:   microphone in volume\n");
                printf ("/LI: or /li:   line in volume\n");

                printf ("/CD: or /cd:   CD out volume\n");
                printf ("/VD: or /vd:   VIDEO out volume\n");
                printf ("/AX: or /ax:   AUX in volume\n");
                printf ("/3C: or /3c:   3D center adjustment\n");
                printf ("/3D: or /3d:   3D depth adjustment\n");
                printf ("/3P: or /3p:   3D path specifier\n");

                printf ("/MB: or /mb:   master bass gain\n");
                printf ("/MT: or /mt:   master treble gain\n");
                printf ("/SS: or /ss:   simulated stereo\n");
                printf ("/BB: or /bb:   bass boost\n");
                printf ("\n");
                printf ("Volume values available range : 0 - 32\n");

                exit(0);
            }   // else
        }   // counter
    }   // argc

    // detect if we run under windows
    if (detect_windows() == TRUE) {
        printf ("Error : This program can not be run under Windows!");
        printf ("\n");
        exit(0);
    }

    // detect if we run under linux
    if (detect_linux() == TRUE) {
        printf ("Error : This program can not be run under Linux!");
        printf ("\n");
        exit(0);
    }

    // detect if we run under os2
    if (detect_os2() == TRUE) {
        printf ("Error : This program can not be run under OS2!");
        printf ("\n");
        exit(0);
    }

    // detect if PCI BIOS is available
    if (pci_check_bios() == FALSE) {
        printf ("Error : PCI local bus not available on this machine!");
        printf ("\n");
        exit(0);
    }

    // detect controller
    counter = 0;
    while (ac97_dev_list[counter].vender_id != 0x0000){
        ac97_pci.vender_id = ac97_dev_list[counter].vender_id;
        ac97_pci.device_id = ac97_dev_list[counter].device_id;
        ac97_pci.sub_vender_id = ac97_dev_list[counter].sub_vender_id;
        ac97_pci.sub_device_id = ac97_dev_list[counter].sub_device_id;
        if (pci_find_device(&ac97_pci) == TRUE){
            printf ("%s detected.\n", ac97_dev_list[counter].string);
            break;
        }
        counter++;
    }

    if (ac97_dev_list[counter].vender_id == NULL){
        printf ("Error : Could not find Intel ICH (AC97/HDA) compatible controller!\n");
        exit(0);
    }

    // save device info
    ac97_pci.device_type = ac97_dev_list[counter].type;
    strcpy(ac97_pci.device_name, ac97_dev_list[counter].string);

    // read pci configuration
    ac97_pci.command = pci_config_read_word (&ac97_pci, PCI_COMMAND);
    ac97_pci.irq = pci_config_read_byte (&ac97_pci, PCI_INTERRUPT_LINE);
    ac97_pci.pin = pci_config_read_byte (&ac97_pci, PCI_INT_LINE);
    ac97_pci.base0 = pci_config_read_dword (&ac97_pci, PCI_BASE_ADDRESS_0);   // NAMBAR
    ac97_pci.base1 = pci_config_read_dword (&ac97_pci, PCI_BASE_ADDRESS_1);   // NABMBAR

    // if memory type IO is enabled then use memory type IO (other manufacturers)
    if (ac97_pci.command & PCI_COMMAND_MEMORY) ac97_pci.mem_mode = 1;

    // start configuring devices
    switch (ac97_pci.device_type) {
        case DEVICE_INTEL_ICH4:
            // try to go for memory type IO as default for ICH4+ Intel controllers even if disabled in PCI command config
            ac97_pci.mem_mode = 1;
            break;
        case DEVICE_NFORCE:
            pci_config_write_dword (&ac97_pci, 0x4c, pci_config_read_dword (&ac97_pci, 0x4c) | 0x1000000);
            break;
        case DEVICE_HDA:
            ac97_pci.hda_mode = 1;
            break;
        default:
            break;
    }

    // HDA not supported yet
    if (ac97_pci.hda_mode) {
        printf ("Error : High Definition Audio controllers not supported yet!\n");
        exit(0);
    }

    // get memory mapped IO for ICH4+ and other new chips
    if ((ac97_pci.base0 == 0) || (ac97_pci.mem_mode)) {
        ac97_pci.base2 = pci_config_read_dword (&ac97_pci, PCI_MEM_BASE_ADDRESS_2);   // NAMBAR
        ac97_pci.base3 = pci_config_read_dword (&ac97_pci, PCI_MEM_BASE_ADDRESS_3);   // NABMBAR

        // map linear memory - convery physical address to linear address
        if (!DPMI_MapMemory ((unsigned long *)&ac97_pci.base2, (unsigned long *)&ac97_pci.base2, 0x1000)) ac97_pci.mem_mode = 0;
        if (!DPMI_MapMemory ((unsigned long *)&ac97_pci.base3, (unsigned long *)&ac97_pci.base3, 0x1000)) ac97_pci.mem_mode = 0;
    }

    // test if both I/O range and memory mapped range are NULL
    // note : memory range address will be NULL if DPMI_MapMemory function fails for some reason
    if ((ac97_pci.base0 == 0) && (ac97_pci.mem_mode == 0)) {
        printf ("Error : AC97/HDA normal I/O and memory mapped I/O access failed!\n");
        exit(0);
    }

    // legacy I/O access
    if (ac97_pci.mem_mode == FALSE) {
        if (ac97_pci.device_type == DEVICE_INTEL_ICH4) {
            // enable the IOSE bit in 0x41 for legacy mode for ICH4/ICH5
            pci_config_write_byte (&ac97_pci, PCI_ICH4_CFG_REG, 1);
            // Set the secondary codec ID
            pci_config_write_byte (&ac97_pci, PCI_COMMAND_PARITY, 0x39);
        }

        // Remove I/O space marker in bit 0
        ac97_pci.base0 &= ~0xf;
        ac97_pci.base1 &= ~0xf;

        // enable IO and bus master
        ac97_pci.command |= PCI_COMMAND_MASTER | PCI_COMMAND_IO;
        pci_config_write_word (&ac97_pci, PCI_COMMAND, ac97_pci.command);

        // diaplay codec information
        printf ("Mixer Base I/O port       : %#06x\n", ac97_pci.base0);
        printf ("Mixer Bus Master I/O port : %#06x\n", ac97_pci.base1);
        printf ("AC97  Interrupt Line      : IRQ %d", ac97_pci.irq);
        printf ("/PIN %c\n", ac97_pci.pin + 64);
        printf ("I/O range and Bus Master access enabled.\n");
        printf ("AC97 configured for normal IO.\n");
        printf ("-----------------------------------------------------------------------\n");
        color_printf ("Primary codec :", 15);
        printf ("\n");

    } else {

        //  enable port IO access compatibility even though we're going for memory mapped IO !!!
        if (ac97_pci.device_type == DEVICE_INTEL_ICH4) {
            // enable the IOSE bit in 0x41 for legacy mode for ICH4/ICH5
            pci_config_write_byte (&ac97_pci, PCI_ICH4_CFG_REG, 1);
            // Set the secondary codec ID
            pci_config_write_byte (&ac97_pci, PCI_COMMAND_PARITY, 0x39);
        }

        // Remove I/O space marker in bit 0
        ac97_pci.base0 &= ~0xf;
        ac97_pci.base1 &= ~0xf;

        // enable memory, IO and bus master
        ac97_pci.command |= PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY;
        pci_config_write_word (&ac97_pci, PCI_COMMAND, ac97_pci.command);

        // diaplay codec information
        printf ("Mixer Base I/O port              : %#06x\n", ac97_pci.base0);
        printf ("Mixer Bus Master I/O port        : %#06x\n", ac97_pci.base1);
        printf ("Mixer Base memory address        : %#06x\n", pci_config_read_dword (&ac97_pci, PCI_MEM_BASE_ADDRESS_2));
        printf ("Mixer Bus Master memory address  : %#06x\n", pci_config_read_dword (&ac97_pci, PCI_MEM_BASE_ADDRESS_3));
        printf ("Mixer Base virtual address       : %#06x\n", ac97_pci.base2);
        printf ("Mixer Bus Master virtual address : %#06x\n", ac97_pci.base3);
        printf ("AC97  Interrupt Line             : IRQ %d", ac97_pci.irq);
        printf ("/PIN %c\n", ac97_pci.pin + 64);
        printf ("Memory range, I/O range and Bus Master access enabled.\n");
        printf ("AC97 configured for memory mapped IO.\n");
        printf ("-----------------------------------------------------------------------\n");
        color_printf ("Primary codec :", 15);
        printf ("\n");

    }


    // initialize codec to 44100Hz sample rate
    if (codec_config (&ac97_pci, 44100) == FALSE) {
        DPMI_UnmapMemory ((unsigned long *)&ac97_pci.base2);
        DPMI_UnmapMemory ((unsigned long *)&ac97_pci.base3);
        printf("Error : Could not initialize Intel ICH AC97/HDA audio codec!\n");
        exit(0);
    }

    // set the volume entered on the command line
    semaphore (&ac97_pci);
    ich_OUTW (&ac97_pci, 0, MIXER_BASE, AC97_RESET);                               // register reset the codec (0)

    semaphore (&ac97_pci);
    ich_OUTW (&ac97_pci, 44100, MIXER_BASE, AC97_PCM_FRONT_DAC_RATE);              // set sample rate  - VRA already set (2ch)

    // set master volume (range is 0 - 64, our range is 0 - 32) - leave lower level range to adjust x2
    if (master_vol == -1) master_vol = 16;                                                    // attenuation by default
    if (master_vol < 0) master_vol = 0;
    if (master_vol > 32) master_vol = 32;
    semaphore (&ac97_pci);
    if (master_vol == 0) ich_OUTW (&ac97_pci, BIT15, MIXER_BASE, AC97_MASTER);                // mute master volume (2)
    else ich_OUTW (&ac97_pci, abs (master_vol - 32) * 0x101, MIXER_BASE, AC97_MASTER);        // set master volume (2)

    // set hp volume (range is 0 - 64, our range is 0 - 32) - leave lower level range to adjust x2
    if (hp_vol == -1) hp_vol = 16;                                                            // attenuation by default
    if (hp_vol < 0) hp_vol = 0;
    if (hp_vol > 32) hp_vol = 32;
    semaphore (&ac97_pci);
    if (hp_vol == 0) ich_OUTW (&ac97_pci, BIT15, MIXER_BASE, AC97_HEADPHONE);                 // mute hp volume (4)
    else ich_OUTW (&ac97_pci, abs (hp_vol - 32) * 0x101, MIXER_BASE, AC97_HEADPHONE);         // set hp volume (4)

    // set mono volume (range is 0 - 64, our range is 0 - 32) - leave lower level range to adjust x2
    if (mono_vol == -1) mono_vol = 16;                                                        // attenuation by default
    if (mono_vol < 0) mono_vol = 0;
    if (mono_vol > 32) mono_vol = 32;
    semaphore (&ac97_pci);
    if (mono_vol == 0) ich_OUTW (&ac97_pci, BIT15, MIXER_BASE, AC97_MASTER_MONO);             // mute mono volume (6)
    else ich_OUTW (&ac97_pci, abs (mono_vol - 32), MIXER_BASE, AC97_MASTER_MONO);             // set mono volume (6)

    // set beep volume (range is 0 - 16, our range is 0 - 32)
    if (beep_vol == -1) beep_vol = 32;                                                        // 0b attenuation by default
    if (beep_vol < 0) beep_vol = 0;
    if (beep_vol > 32) beep_vol = 32;
    semaphore (&ac97_pci);
    if (beep_vol == 0) ich_OUTW (&ac97_pci, BIT15, MIXER_BASE, AC97_PCBEEP_VOL);              // mute beep volume (0ah)
    else ich_OUTW (&ac97_pci, abs ((beep_vol >> 1) - 16) << 1, MIXER_BASE, AC97_PCBEEP_VOL);  // set beep volume (0ah)

    // set wave volume PCM out (range is 0 - 32, our range is 0 - 32)
    if (wave_vol == -1) wave_vol = 24;                                                        //  0db gain by default
    if (wave_vol < 0) wave_vol = 0;
    if (wave_vol > 32) wave_vol = 32;
    semaphore (&ac97_pci);
    if (wave_vol == 0) ich_OUTW (&ac97_pci, BIT15, MIXER_BASE, AC97_PCM);                     // mute wave volume (18h)
    else ich_OUTW (&ac97_pci, abs (wave_vol - 32) * 0x101, MIXER_BASE, AC97_PCM);             // set wave volume (18h)

    // set phone volume (range is 0 - 32, our range is 0 - 32)
    if (phone_vol == -1) phone_vol = 28;                                                      // gain by default
    if (phone_vol < 0) phone_vol = 0;
    if (phone_vol > 32) phone_vol = 32;
    semaphore (&ac97_pci);
    if (phone_vol == 0) ich_OUTW (&ac97_pci, BIT15, MIXER_BASE, AC97_PHONE_VOL);              // mute phonee volume (0ch)
    else ich_OUTW (&ac97_pci, abs (phone_vol - 32), MIXER_BASE, AC97_PHONE_VOL);              // set phone volume (0ch)

    // set mic volume (range is 0 - 32, our range is 0 - 32)
    if (mic_vol == -1) mic_vol = 0;                                                           // gain by default
    if (mic_vol < 0) mic_vol = 0;
    if (mic_vol > 32) mic_vol = 32;
    semaphore (&ac97_pci);
    if (mic_vol == 0) ich_OUTW (&ac97_pci, BIT15, MIXER_BASE, AC97_MIC_VOL);                  // mute mic volume (0eh)
    else ich_OUTW (&ac97_pci, abs (mic_vol - 32), MIXER_BASE, AC97_MIC_VOL);                  // set mic volume (0eh)

    // set line in volume (range is 0 - 32, our range is 0 - 32)
    if (line_in_vol == -1) line_in_vol = 28;                                                  // gain by default
    if (line_in_vol < 0) line_in_vol = 0;
    if (line_in_vol > 32) line_in_vol = 32;
    semaphore (&ac97_pci);
    if (line_in_vol == 0) ich_OUTW (&ac97_pci, BIT15, MIXER_BASE, AC97_LINE_IN_VOL);          // mute line in volume (10h)
    else ich_OUTW (&ac97_pci, abs (line_in_vol - 32) * 0x101, MIXER_BASE, AC97_LINE_IN_VOL);  // set line in volume (10h)

    // set CD volume (range is 0 - 32, our range is 0 - 32)
    if (cd_vol == -1) cd_vol = 28;                                                            // gain by default
    if (cd_vol < 0) cd_vol = 0;
    if (cd_vol > 32) cd_vol = 32;
    semaphore (&ac97_pci);
    if (cd_vol == 0) ich_OUTW (&ac97_pci, BIT15, MIXER_BASE, AC97_CD_VOL);                    // mute CD volume (12h)
    else ich_OUTW (&ac97_pci, abs (cd_vol - 32) * 0x101, MIXER_BASE, AC97_CD_VOL);            // set CD volume (12h)

    // set video volume (range is 0 - 32, our range is 0 - 32)
    if (video_vol == -1) video_vol = 28;                                                      // gain by default
    if (video_vol < 0) video_vol = 0;
    if (video_vol > 32) video_vol = 32;
    semaphore (&ac97_pci);
    if (video_vol == 0) ich_OUTW (&ac97_pci, BIT15, MIXER_BASE, AC97_VID_VOL);                // mute video volume (14h)
    else ich_OUTW (&ac97_pci, abs (video_vol - 32) * 0x101, MIXER_BASE, AC97_VID_VOL);        // set video volume (14h)

    // set aux volume (range is 0 - 32, our range is 0 - 32)
    if (aux_vol == -1) aux_vol = 28;                                                          // gain by default
    if (aux_vol < 0) aux_vol = 0;
    if (aux_vol > 32) aux_vol = 32;
    semaphore (&ac97_pci);
    if (aux_vol == 0) ich_OUTW (&ac97_pci, BIT15, MIXER_BASE, AC97_AUX_VOL);                  // mute aux volume (16h)
    else ich_OUTW (&ac97_pci, abs (aux_vol - 32) * 0x101, MIXER_BASE, AC97_AUX_VOL);          // set aux volume (16h)

    // check if 3D effect available - adjust to SE4..SE0 bits
    semaphore (&ac97_pci);
    flags = (ich_INW (&ac97_pci, MIXER_BASE, AC97_RESET) >> 10) & (BIT0 + BIT1 + BIT2 + BIT3 + BIT4);
    if (flags) {   // get Ac97 reset status (0)
        printf ("%s", ac97_stereo_technology[flags]);
        printf (" technology detected.\n");

        // enable 3D depth and center if requested
        semaphore (&ac97_pci);
        flags = ich_INW (&ac97_pci, MIXER_BASE, AC97_GP_REG);                                // general purpose 20h register
        semaphore (&ac97_pci);
        if ( ((a3d_vol != 0) && (a3d_vol != -1)) || ((a3c_vol != 0) && (a3c_vol != -1)) ) flags |= BIT13;   // set BIT13 (enable 3D)
        else flags &= (0xffff - BIT13);
        ich_OUTW (&ac97_pci, flags, MIXER_BASE, AC97_GP_REG);                                // general purpose 20h register

        // set 3D depth and center (range is 0 - 15, our range is 0 - 31)
        if (a3d_vol < 0) a3d_vol = 0;                                                        // no 3D depth as default
        if (a3d_vol > 31) a3d_vol = 31;
        if (a3c_vol < 0) a3c_vol = 0;                                                        // no 3D center as default
        if (a3c_vol > 31) a3c_vol = 31;
        semaphore (&ac97_pci);
        ich_OUTW (&ac97_pci, (a3d_vol >> 1) + (0x100 * (a3c_vol >> 1)), MIXER_BASE, AC97_3D_CONTROL_REG);   // 3D control register 22h
        semaphore (&ac97_pci);
        flags = ich_INW (&ac97_pci, MIXER_BASE, AC97_3D_CONTROL_REG);                        // 3D control register 22h
        if ((flags & 0x0f00) != (0x100 * (a3c_vol >> 1))) {
            color_printf ("3D center adjustment not supported by hardware!", 12);
            printf ("\n");
        }
        if ((flags & 0x0f) != (a3d_vol >> 1)) {
            color_printf ("3D depth adjustment not supported by hardware!", 12);
            printf ("\n");
        }

        // set 3D path
        semaphore (&ac97_pci);
        flags = ich_INW (&ac97_pci, MIXER_BASE, AC97_GP_REG);                                // general purpose 20h register
        semaphore (&ac97_pci);
        if (a3p_vol <= 0) ich_OUTW (&ac97_pci, flags & (0xffff - BIT15), MIXER_BASE, AC97_GP_REG);   // PRE 3D path as default
        if (a3p_vol >= 1) ich_OUTW (&ac97_pci, flags | BIT15, MIXER_BASE, AC97_GP_REG);              // POST 3D path if set
        semaphore (&ac97_pci);
        flags = ich_INW (&ac97_pci, MIXER_BASE, AC97_GP_REG);                                // 3D control register 22h
        if ((a3p_vol >= 1) && !(flags & BIT15)) {
            color_printf ("3D path control not supported by hardware!", 12);
            printf ("\n");
        }

    } else {
        printf ("3D Stereo Enhancement technology not detected.\n");
        if ((a3c_vol != -1) || (a3d_vol != -1)) {
            color_printf ("3D center/depth adjustment not supported by hardware!", 12);
            printf ("\n");
        }
        if (a3p_vol != -1) {
            color_printf ("3D path control not supported by hardware!", 12);
            printf ("\n");
        }
    }

    // check for bass/treble support
    semaphore (&ac97_pci);
    flags = ich_INW (&ac97_pci, MIXER_BASE, AC97_RESET) & BIT2;                      // ID2 bass and treble control
    if (flags) {
        // set bass/treble range 0 - 16, our range is 0 - 31
        if (bass_vol < 2) bass_vol = 2;                                              // default bass = bypass
        if (bass_vol > 32) bass_vol = 32;
        if (treble_vol < 2) treble_vol = 2;                                          // default treble = bypass
        if (treble_vol > 32) treble_vol = 32;
        semaphore (&ac97_pci);
        ich_OUTW (&ac97_pci, abs ((treble_vol >> 1) - 16) + (0x100 * abs ((bass_vol >> 1) - 16)), MIXER_BASE, AC97_MASTER_TONE);   // master tone register 8
    } else {
        if ((bass_vol != -1) || (treble_vol != -1)) {
            color_printf ("Bass/Treble gain control not supported by hardware!", 12);
            printf ("\n");
        }
    }

    // check for bass boost support
    semaphore (&ac97_pci);
    flags = ich_INW (&ac97_pci, MIXER_BASE, AC97_RESET) & BIT5;                      // ID5 bass boost control
    if (flags) {
        // set/clear  bass boost
        semaphore (&ac97_pci);
        flags = ich_INW (&ac97_pci, MIXER_BASE, AC97_GP_REG);
        semaphore (&ac97_pci);
        if (bassbst_vol <= 0) ich_OUTW (&ac97_pci, flags & (0xffff - BIT12), MIXER_BASE, AC97_GP_REG);   // default bass = 0
        if (bassbst_vol >= 1) ich_OUTW (&ac97_pci, flags | BIT12, MIXER_BASE, AC97_GP_REG);
    } else {
        if (bassbst_vol != -1) {
            color_printf ("Bass boost control not supported by hardware!", 12);
            printf ("\n");
        }
    }

    // check for simulated stereo support
    semaphore (&ac97_pci);
    flags = ich_INW (&ac97_pci, MIXER_BASE, AC97_RESET) & BIT3;                      // ID3 simulated stereo control
    if (flags) {
        // set/clear  bass boost
        semaphore (&ac97_pci);
        flags = ich_INW (&ac97_pci, MIXER_BASE, AC97_GP_REG);
        semaphore (&ac97_pci);
        if (sstereo_vol <= 0) ich_OUTW (&ac97_pci, flags & (0xffff - BIT14), MIXER_BASE, AC97_GP_REG);   // default ss = 0
        if (sstereo_vol >= 1) ich_OUTW (&ac97_pci, flags | BIT14, MIXER_BASE, AC97_GP_REG);
    } else {
        if (sstereo_vol != -1) {
            color_printf ("Simulated stereo control not supported by hardware!", 12);
            printf ("\n");
        }
    }

    // identify codec manufacturer
    semaphore (&ac97_pci);
    ac97_pci.codec_id1 = ich_INW (&ac97_pci, MIXER_BASE, AC97_VENDERID1_REG);        // codec vender ID1
    semaphore (&ac97_pci);
    ac97_pci.codec_id2 = ich_INW (&ac97_pci, MIXER_BASE, AC97_VENDERID2_REG);        // codec vender ID2
    counter = 0;
    while (ac97_codec_list[counter].vender_id1 != 0x0000){
        if ( (ac97_pci.codec_id1 == ac97_codec_list[counter].vender_id1) && (ac97_pci.codec_id2 == ac97_codec_list[counter].vender_id2) ) {
            printf ("%s initialization success.\n", ac97_codec_list[counter].string);
            printf ("\n");
            strcpy (ac97_pci.codec_name, ac97_codec_list[counter].string);
            break;
        }
        counter++;
    }

    // could not identify manufacturer
    if (ac97_codec_list[counter].vender_id1 == 0x0000) {
            printf ("%s initialization success.\n", ac97_codec_list[counter].string);
            printf ("\n");
            strcpy (ac97_pci.codec_name, ac97_codec_list[counter].string);
    }

    // unmap linear memory
    DPMI_UnmapMemory ((unsigned long *)&ac97_pci.base2);
    DPMI_UnmapMemory ((unsigned long *)&ac97_pci.base3);

    // end of initialization
}

void  color_printf (char *string, unsigned char color)
{
    // foreground colors (bits 0-3)
    // 0  = black
    // 1  = blue
    // 2  = green
    // 3  = cyan
    // 4  = red
    // 5  = magneta
    // 6  = brown
    // 7  = light gray
    // 8  = dark grey
    // 9  = light blue
    // 10 = light green
    // 11 = light cyan
    // 12 = light red
    // 13 = light magneta
    // 14 = yellow
    // 15 = white
    // background colors (bits 4-7)

    int counter = 0;
    union REGS r;

    memset(&r, 0, sizeof(r));
    r.w.ax = 0x1A00;                     // test VGA mode
    r.w.bx = 0;
    int386 (0x10, &r, &r);
    if (r.w.bx == 0) {
        printf (string);
        return;
    }

    memset(&r, 0, sizeof(r));
    r.h.ah = 0x0F;                       // get current page
    int386 (0x10, &r, &r);               // bh = current page
    r.h.bl = color;                      // bl = attribute (text mode)

    while (string[counter]) {            // NULL char ends
        r.w.cx = 1;                      // nr of times to write char
        r.h.al = string[counter];
        r.h.ah = 9;                      // write char at cursor position
        int386 (0x10, &r, &r);

        r.h.ah = 3;                      // get cursor position
        int386 (0x10, &r, &r);

        r.h.dl++;                        // advance cursor
        r.h.ah = 2;                      // set cursor position
        int386 (0x10, &r, &r);

        counter++;
    }
}
