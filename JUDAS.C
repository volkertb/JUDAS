/*
 * JUDAS Apocalyptic Softwaremixing Sound System V2.10d 
 * original version 2.06y by Faust & Yehar
 * extended version 2.10b with Intel ICH AC97/HDA support by Black Spider
 * Please be aware that HDA support in this version falls under the GNU/GPL
 * license. The HDA code was written after analyzing various HDA linux related
 * sources, the drivers from ALSA project made by Takashi Iwai, the OSS sound
 * system project and others. Although this code is different and pretty unique
 * some similarities especially in the HDA nodes access strategy can be found.
 * That is why I decided to publicly release this version of Judas under the
 * GNU/GPL license.
 *
 * Supported:
 * - SOUND BLASTER (8bit, mono, up to 22050 Hz)
 * - SOUND BLASTER PRO (8bit, stereo, up to 22050 Hz or mono up to 44100 Hz)
 * - SOUND BLASTER 16 (16bit, stereo, up to 44100 Hz)
 * - ULTRASOUND (16bit, stereo, up to 44100 Hz)
 * - INTEL ICH0/ICH/ICH2/ICH3/ICH4 AC97 CODEC (16bit, stereo, up to 48000 Hz)
 * - High Definition Audio codecs (downgrade to 16bit stereo at 48000 Hz max)
 * - WAV file output
 * - XMs, MODs, S3Ms, WAVs, raw samples
 *
 * Other features:
 * - Clipping of sound output
 * - Interpolation options: None, linear, cubic
 * - Quality mixer: Click removing, high quality, 32 bit, clipping indicator
 *
 * This is the main module, where soundcards & mixer are initialized. It's not
 * very good coding style, but who cares!
 *
 * Changes:
 * V2.01  Doesn't fuck up if soundcard variables contain XXX-shit!
 * V2.04y Added selectable quality mixer. Old mixer is now called fast mixer
 *        Added file writer
 * V2.06y Wrote "char *volatile pos" instead of old "volatile char *pos"!
 *        Removed file writer and made the WAV writer in JUDASWAV.C
 * V2.07a Support for Intel ICH0/ICH/ICH2/ICH3 and compatible AC97 Codecs
 *        added by Black Spider
 *        (Read JUDAS.DOC for complete history)
 * V2.07y Improved Intel ICH0/ICH/ICH2/ICH3 AC97 code
 *        Added ICH4 controller support
 * V2.08y Improved Intel ICH4 controller support
 *        (Read JUDAS.DOC for complete history)
 * V2.09a Added support for SIS7012 - now it works :)
 * V2.09b Corrected ICH4 and above chips support
 * V2.09c dev version
 * V2.09d corrected dev version with full WATCOM C++ & DJGPP support
 * V2.09e all AC97 devices now use memory mapped IO if available
 * V2.09f WAV library builder added to JUDAS
 * V2.10a Intel HDA codecs support added
 *      basing on info from ALSA sources and other interesting HDA resources
 *      on the web i was finally able to add High Definition audio codecs support
 *      for Judas Sound System (this is not a public release).
 * V2.10b judas_setwavlib function added - first public release with HDA code
 *        licensed under the GNU/GPL
 * V2.10d HDA codecs support update for intel 5, 6, 7 series test by RayeR
 */

/*
 * Borland / Watcom REGS structure compatibility
 */
#ifdef __DJGPP__
#define _BORLAND_DOS_REGS
#endif

#include <io.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <mem.h>
#include <stdarg.h>
#include "judasdma.h"
#include "judasmem.h"
#include "judascfg.h"
#include "judasgus.h"
#include "judaserr.h"
#include "judasac.h"

#ifdef __DJGPP__
#include <go32.h>
#include <dpmi.h>
#include <sys/nearptr.h>
#define HANDLE_PRAGMA_PACK_PUSH_POP 1
/* define as nothing nothing */
#define interrupt
/* make port access compatible with Watcom */
#define inp(P) inportb(P)
#define inpw(P) inportw(P)
#define inpd(P) inportl(P)
#define outp(P,V) outportb(P,V)
#define outpw(P,V) outportw(P,V)
#define outpd(P,V) outportl(P,V)
#else
#define __djgpp_conventional_base 0
#endif

/*
  Debug or retail release (only for AC97/HDA related code)
*/
// #define AC97_DEBUG
// #define HDA_DEBUG

/*
 * Sound device numbers
 */
#define DEV_NOSOUND 0
#define DEV_SB 1
#define DEV_SBPRO 2
#define DEV_SB16 3
#define DEV_GUS 4
#define DEV_AC97 5
#define DEV_HDA 6
#define DEV_FILE 7

/*
 * Interrupt controller ports
 */
#define PICPORT1 0x20
#define PICPORT2 0xa0
#define PICMASK1 0x21
#define PICMASK2 0xa1

/*
 * Mixer numbers
 */
#define FASTMIXER 1
#define QUALITYMIXER 2

/*
 * Mixmode bits
 */
#define MONO 0
#define STEREO 1
#define EIGHTBIT 0
#define SIXTEENBIT 2

/*
 * Voicemode bits
 */
#define VM_OFF 0
#define VM_ON 1
#define VM_LOOP 2
#define VM_16BIT 4

/*
 * Sample & channel structures
 */
typedef struct
{
        char *start;
        char *repeat;
        char *end;
        char *vuprofile;
        unsigned char voicemode;
} SAMPLE;

#pragma pack(push,1)
typedef struct
{
        char *volatile pos;
        char *repeat;
        char *end;
        SAMPLE *volatile sample;
        unsigned freq;
        volatile unsigned short fractpos;
        unsigned char mastervol;
        unsigned char panning;
        signed short vol;
        volatile unsigned char voicemode;

        /* For quality mixer */
        volatile char prevvm;
        char *volatile prevpos;
        volatile int lastvalleft;
        volatile int lastvalright;
        volatile int smoothvoll;
        volatile int smoothvolr;
} CHANNEL;
#pragma pack(pop)


/*
 * Prototypes
 */
void judas_config(void);
int judas_init(unsigned mixrate, unsigned mixer, unsigned mixmode, int interpolation);
void judas_uninit(void);
int judas_songisplaying(void);

static int judas_lock(void);
static int initmixer(void);
static void sb_delay(void);
static void sb_write(unsigned char value);
static unsigned char sb_read(void);
static int sb_reset(void);
static void sb_getversion(void);

static int gus_detect(void);
static void gus_reset(void);
static void gus_delay(void);
static void gus_setupchannels(void);
static void gus_setupchannel(unsigned char channel, unsigned start, unsigned length);
static void gus_stopchannel(unsigned char channel);

/* debug and log functions for ac97 code - internal to judas.c */
static void ac97_set_error_message(char *message);
static void ac97_display_error_message(void);
static void ac97_clear_error_message(void);
static void logmessage(const char *text,...);

/* pci stuff for ich ac97 compatible codecs */
static BYTE  pci_config_read_byte(int index);
static WORD  pci_config_read_word(int index);
static DWORD pci_config_read_dword(int index);
static void  pci_config_write_byte(int index, BYTE data);
static void  pci_config_write_word(int index, WORD data);
static void  pci_config_write_dword(int index, DWORD data);
static BOOL  pci_check_bios(void);
static BOOL  pci_find_device();
static void  pci_enable_io_access();
static void  pci_enable_memory_access();
static void  pci_enable_busmaster();
static BOOL  detect_windows(void);
static BOOL  detect_os2(void);
static BOOL  detect_linux(void);

/* ac97 audio mixer registers access helper functions */
static BOOL ac97_codec_semaphore(void);
static WORD ac97_read_codec(BYTE index);
static void ac97_write_codec(BYTE index, WORD data);
static void ac97_stop(void);
static int ac97_reset(void);
static unsigned int ich_INL (int base, unsigned int a);
static unsigned short ich_INW (int base, unsigned short a);
static unsigned char ich_INB (int base, unsigned char a);
static void ich_OUTL (unsigned int d, int base, unsigned int a);
static void ich_OUTW (unsigned short d, int base, unsigned short a);
static void ich_OUTB (unsigned char d, int base, unsigned char a);

/* HDA audio mixer registers access helper functions */
static unsigned int hda_calc_stream_format (int mixrate);

/* HDA register access */
static unsigned int hda_INL (unsigned int a);
static unsigned short hda_INW (unsigned int a);
static unsigned char hda_INB (unsigned int a);
static void hda_OUTL (unsigned int a, unsigned int d);
static void hda_OUTW (unsigned int a, unsigned short d);
static void hda_OUTB (unsigned int a, unsigned char d);

/* HDA Basic read/write to codecs - Single immediate command instead of CORB/RIRB buffers for simplicity */
static BOOL hda_single_send_cmd (unsigned short nid, unsigned int direct, unsigned int verb, unsigned int param);
static unsigned int hda_single_get_response (void);
static unsigned int hda_codec_read (unsigned short nid, unsigned int direct, unsigned int verb, unsigned int param);
static unsigned int hda_param_read (unsigned short nid ,unsigned int param);
static int hda_codec_write (unsigned short nid, unsigned int direct, unsigned int verb, unsigned int param);

/* HDA main functions */
static void hda_codec_stop (void);
static void hda_codec_start (void);
extern long hda_getbufpos (void);
static BOOL hda_reset (void);

/* HDA mixer functions */
static unsigned int hda_mixer_init (void);
static void hda_set_volume (unsigned long reg, unsigned long val);
static unsigned long hda_get_volume (unsigned long reg);

/* nodes identification prototypes */
static unsigned int hda_get_sub_nodes (unsigned short nid, unsigned short *start_node);
static void hda_search_audio_node (void);
static int hda_get_connections (unsigned short nid, unsigned short *conn_list, int max_conns);
static int hda_add_new_node (struct hda_node *node, unsigned short nid);
static struct hda_node *hda_get_node (unsigned short nid);

/* nodes functions prototypes */
static void hda_set_vol_mute (unsigned short nid, int ch, int direction, int index, int val);
static unsigned int hda_get_vol_mute (unsigned short nid, int ch, int direction, int index);
static int hda_codec_amp_update (unsigned short nid, int ch, int direction, int idx, int mask, int val);
static void hda_unmute_output (struct hda_node *node);
static void hda_unmute_input (struct hda_node *node, unsigned int index);
static int hda_select_input_connection (struct hda_node *node, unsigned int index);
static void hda_clear_check_flags (void);

/* output path - select connections */
static int hda_parse_output_path (struct hda_node *node, int dac_idx);
static struct hda_node *hda_parse_output_jack (int jack_type);
static int hda_parse_output (void);


/*
 * Assembler functions in JUDASASM.ASM / JASMDJ.ASM
 */
void judas_update(void);
void interrupt sb_handler(void);
void interrupt sb_aihandler(void);
void interrupt sb16_handler(void);
void interrupt gus_handler(void);

void gus_poke(unsigned location, unsigned char data);
unsigned char gus_peek(unsigned location);
void gus_startchannels(void);
void gus_dmaprogram(unsigned pos, unsigned length);
void gus_dmainit(void);
void gus_dmawait(void);

void ac97_poke(unsigned location, unsigned char data);
unsigned char ac97_peek(unsigned location);
void ac97_startchannels(void);
void ac97_dmaprogram(unsigned pos, unsigned length);
void ac97_dmainit(void);
void ac97_dmawait(void);

void fmixer(void *address, int length);
void qmixer(void *address, int length);
void safemixer(void *address, int length);
void normalmix(void);
void ipmix(void);
void qmix_linear(void);
void qmix_cubic(void);
void judas_code_lock_start(void);
void judas_code_lock_end(void);
unsigned short judas_get_ds(void);


/*
 * Variables
 */
int judas_error = JUDAS_OK;
void (*judas_player)(void) = NULL;
void (*judas_mixroutine)(void) = &normalmix;
void (*judas_mixersys)(void *address, int length) = &qmixer;

#ifdef __DJGPP__
/* DJGPP version
    _go32_dpmi_seginfo structure contains (  unsigned long size; unsigned long pm_offset; unsigned short pm_selector; unsigned short rm_offset; unsigned short rm_segment; ) */
static _go32_dpmi_seginfo judas_oldvect;
static _go32_dpmi_seginfo judas_newvect;

#else
/* WATCOM C++ version */
static void (__interrupt __far *judas_oldvect)();
static void (__interrupt __far *judas_newvect)();
#endif

unsigned judascfg_device = DEV_NOSOUND;
unsigned judascfg_port = -1;
unsigned judascfg_irq = -1;
unsigned judascfg_dma1 = -1;
unsigned judascfg_dma2 = -1;
unsigned judas_irqcount = 0;
unsigned judas_device;
unsigned judas_port;
unsigned judas_irq;
unsigned judas_dma;
unsigned judas_int;
unsigned judas_mixrate;
unsigned judas_mixpos;
unsigned judas_bufferlength;
unsigned judas_buffermask;
unsigned judas_bpmcount;
int *judas_clipbuffer;
int *judas_zladdbuffer; /* Alustukset, „„li”! *** */
int judas_zerolevell = 0;
int judas_zerolevelr = 0;
short *judas_cliptable;
int *judas_volumetable;
unsigned short judas_ds;
static unsigned short dsp_version;
static unsigned char mixer_firsttime = 1;
static unsigned char judas_locked = 0;
static unsigned char judas_oldpicmask1;
static unsigned char judas_oldpicmask2;
unsigned char judas_initialized = 0;
unsigned char judas_mixer;
unsigned char judas_mixmode;
unsigned char judas_bpmtempo;
unsigned char judas_samplesize;
CHANNEL judas_channel[CHANNELS];
char judas_clipped = 0;
char *filewriterbuffer;
int filewriterbuffersize = 65536;
AUDIO_PCI_DEV audio_pci = {0}; /*  Pci device structure for AC97/HDA */
unsigned long hda_civ = 0;
unsigned long hda_lpib = 0;


/*
 * JUDAS device names
 */
char *judas_devname[] =
{
        "No Sound",
        "Sound Blaster",
        "Sound Blaster Pro",
        "Sound Blaster 16",
        "UltraSound",
        "AC97 Audio Codec",
        "HDA Audio Codec",
        "File Writer"
};

/*
 * Mixer names
 */
char *judas_mixername[] =
{
        "(No mixer, big shit will happen)",
        "Fast Mixer",
        "Quality Mixer",
};

/*
 * Mixmode names
 */
char *judas_mixmodename[] =
{
        "8-bit mono",
        "8-bit stereo",
        "16-bit mono",
        "16-bit stereo"
};

/*
 * Interpolation mode names
 */
char *judas_ipmodename[] =
{
        "(?)",
        "(?)"
};

/*
 * JUDAS error texts
 */
char *judas_errortext[] =
{
        "Everything OK",
        "Couldn't open file",
        "Couldn't read file",
        "Incorrect file format",
        "Out of memory",
        "Hardware init failure",
        "Configuration incorrect",
        "Out of channels"
};

static unsigned char gus_dmalatch[] =
{
        0x40, 0x41, 0x40, 0x42, 0x40, 0x43, 0x44, 0x45
};

static unsigned char gus_irqlatch[] =
{
        0x40, 0x40, 0x40, 0x43, 0x40, 0x42, 0x40, 0x44,
        0x40, 0x41, 0x40, 0x45, 0x46, 0x40, 0x40, 0x47
};

/*
 * Fake sample returned by the routines when in NOSOUND mode
 */
SAMPLE fakesample = {NULL, NULL, NULL, NULL, VM_OFF};

int judas_songisplaying(void) {
  return (judas_player != NULL)? 1:0;
}


/********************************************************************
 *      Error and debug functions for AC97 (internal to judas.c)
 ********************************************************************/

#define AC97_ERROR_MESSAGE_BUFFER_SIZE 256
static char ac97_error_message[AC97_ERROR_MESSAGE_BUFFER_SIZE] = {0};

static void ac97_set_error_message(char *message)
{
        int count = 0;
        char cvalue;

        while (count < AC97_ERROR_MESSAGE_BUFFER_SIZE - 3) {
                cvalue = message[count];
                if (!cvalue) break;
                ac97_error_message[count] = cvalue;
                count++;
                }
        ac97_error_message[count] = '\n';
        ac97_error_message[count + 1] = '\0';
}

static void ac97_display_error_message(void)
{
        printf(">>> %s", &ac97_error_message[0]);
}

static void ac97_clear_error_message(void)
{
        ac97_error_message[0] = '\0';
}

static void logmessage(const char *text,...)
{
          va_list arg;

          va_start(arg, text);
          vfprintf(stdout, text, arg);
          va_end(arg);
}


/********************************************************************
 *      Intel PCI BIOS helper funtions
 ********************************************************************/

#ifndef PCI_ANY_ID
#define PCI_ANY_ID      ((WORD)(~0))
#endif

static BYTE pci_config_read_byte(int index)
{
        union REGS r;

        memset(&r, 0, sizeof(r));
        r.x.eax = 0x0000B108;                // config read byte
        r.x.ebx = (DWORD)audio_pci.device_bus_number;
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

static WORD pci_config_read_word(int index)
{
        union REGS r;

        memset(&r, 0, sizeof(r));
        r.x.eax = 0x0000B109;                // config read word
        r.x.ebx = (DWORD)audio_pci.device_bus_number;
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

static DWORD pci_config_read_dword(int index)
{
        union REGS r;

        memset(&r, 0, sizeof(r));
        r.x.eax = 0x0000B10A;                // config read dword
        r.x.ebx = (DWORD)audio_pci.device_bus_number;
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

static void pci_config_write_byte(int index, BYTE data)
{
        union REGS r;

        memset(&r, 0, sizeof(r));
        r.x.eax = 0x0000B10B;                // config write byte
        r.x.ebx = (DWORD)audio_pci.device_bus_number;
        r.x.ecx = (DWORD)data;
        r.x.edi = (DWORD)index;
        int386(0x1a, &r, &r);
        if (r.h.ah != 0 ){
                #ifdef AC97_DEBUG
                logmessage("Error : PCI write config byte failed\n");
                #endif
        }
}

static void pci_config_write_word(int index, WORD data)
{
        union REGS r;

        memset(&r, 0, sizeof(r));
        r.x.eax = 0x0000B10C;                // config write word
        r.x.ebx = (DWORD)audio_pci.device_bus_number;
        r.x.ecx = (DWORD)data;
        r.x.edi = (DWORD)index;
        int386(0x1a, &r, &r);
        if (r.h.ah != 0 ){
                #ifdef AC97_DEBUG
                logmessage("Error : PCI write config word failed\n");
                #endif
        }
}

static void pci_config_write_dword(int index, DWORD data)
{
        union REGS r;

        memset(&r, 0, sizeof(r));
        r.x.eax = 0x0000B10D;                // config write dword
        r.x.ebx = (DWORD)audio_pci.device_bus_number;
        r.x.ecx = (DWORD)data;
        r.x.edi = (DWORD)index;
        int386(0x1a, &r, &r);
        if (r.h.ah != 0 ){
                #ifdef AC97_DEBUG
                logmessage("Error : PCI write config dword failed\n");
                #endif
        }
}

static BOOL pci_check_bios(void)
{
        union REGS r;

        memset(&r, 0, sizeof(r));
        r.x.eax = 0x0000B101;                // PCI BIOS - installation check
        r.x.edi = 0x00000000;
        int386(0x1a, &r, &r);
        if (r.x.edx != 0x20494350) return FALSE;   // ' ICP' identifier found ?
        return TRUE;
}

static BOOL pci_find_device()
{
        union REGS r;

        memset(&r, 0, sizeof(r));
        r.x.eax = 0x0000B102;                   // PCI BIOS - find PCI device
        r.x.ecx = audio_pci.device_id;           // device ID
        r.x.edx = audio_pci.vender_id;           // vender ID
        r.x.esi = 0x00000000;                   // device index
        int386(0x1a, &r, &r);
        if (r.h.ah != 0 ) return FALSE;         // device not found
        audio_pci.device_bus_number = r.w.bx;    // save device & bus/funct number
        if(audio_pci.sub_vender_id != PCI_ANY_ID){
                // check subsystem vender id
                if(pci_config_read_word(0x2C) != audio_pci.sub_vender_id) return FALSE;
        }
        if(audio_pci.sub_device_id != PCI_ANY_ID){
                // check subsystem device id
                if(pci_config_read_word(0x2E) != audio_pci.sub_device_id) return FALSE;
        }
        return TRUE;                             // device found
}

static void pci_enable_io_access()
{
        pci_config_write_word(0x04, pci_config_read_word(0x04) | BIT0);
}

static void pci_enable_memory_access()
{
        pci_config_write_word(0x04, pci_config_read_word(0x04) | BIT1);
}

static void pci_enable_busmaster()
{
        pci_config_write_word(0x04, pci_config_read_word(0x04) | BIT2);
}


/********************************************************************
 *    Windows/OS2/Linux detection helper functions
 ********************************************************************/

static BOOL detect_windows(void)   // Win 3.1+ detection
{
        union REGS r;

        memset(&r, 0, sizeof(r));
        r.x.eax = 0x1600;
        int386(0x2F, &r, &r);
        if ((r.h.al & 0x7F) != 0) return TRUE;
        return FALSE;
}

static BOOL detect_os2(void)   // OS2 2.0+ detection
{
        union REGS r;

        memset(&r, 0, sizeof(r));
        r.x.eax = 0x4010;
        int386(0x2F, &r, &r);
        if (r.w.ax == 0) return TRUE;
        return FALSE;
}

static BOOL detect_linux(void)   // Linux DOSEMU detection ???
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


/********************************************************************
 *    Judas config and init procedures
 ********************************************************************/

void judas_config(void)
{
        char *envstr;
        judascfg_device = DEV_NOSOUND;


        /*
                       * Try to find BLASTER enviroment variable
                       */
        envstr = getenv("BLASTER");
        if (envstr)
        {
                judascfg_device = DEV_SB;
                judascfg_port = -1;
                judascfg_irq = -1;
                judascfg_dma1 = -1;
                judascfg_dma2 = -1;
                while (*envstr)
                {
                        unsigned sb_type = 0;
                        if ((*envstr == 'A') || (*envstr == 'a')) sscanf(envstr + 1, "%x", &judascfg_port);
                        if ((*envstr == 'I') || (*envstr == 'i')) sscanf(envstr + 1, "%d", &judascfg_irq);
                        if ((*envstr == 'D') || (*envstr == 'd'))
                        {
                                sscanf(envstr + 1, "%d", &judascfg_dma1);
                        }
                        if ((*envstr == 'T') || (*envstr == 't'))
                        {
                                sscanf(envstr + 1, "%d", &sb_type);
                                if ((sb_type == 2) || (sb_type == 4)) judascfg_device = DEV_SBPRO;
                        }
                        if ((*envstr == 'H') || (*envstr == 'h'))
                        {
                                sscanf(envstr + 1, "%d", &judascfg_dma2);
                                judascfg_device = DEV_SB16;
                        }
                        envstr++;
                }
        }

        /*
                       * Try to find ULTRASND enviroment variable
                       */
        envstr = getenv("ULTRASND");
        if (envstr)
        {
                unsigned irq2;
                judascfg_device = DEV_GUS;
                judascfg_port = -1;
                judascfg_irq = -1;
                judascfg_dma1 = -1;
                judascfg_dma2 = -1;
                sscanf(envstr, "%x,%d,%d,%d,%d", &judascfg_port, &judascfg_dma1,
                        &judascfg_dma2, &judascfg_irq, &irq2);
                /* If MIDI IRQ is lower then use it (for DOS4G) */
                if (irq2 < judascfg_irq) judascfg_irq = irq2;
        }

        /*
                       * Try to detect Intel ICH AC97 via PCI access
                       */
        if (judascfg_device == DEV_NOSOUND)
        {
                int device;
                if (detect_windows() == TRUE) {
                        ac97_set_error_message("Error : AC97 Audio Codec driver works only in pure DOS.\n");
                        return;
                }
                if (detect_os2() == TRUE) {
                        ac97_set_error_message("Error : AC97 Audio Codec driver works only in pure DOS.\n");
                        return;
                }
                if (detect_linux() == TRUE) {
                        ac97_set_error_message("Error : AC97 Audio Codec driver works only in pure DOS.\n");
                        return;
                }
                if (pci_check_bios() == FALSE) {
                        ac97_set_error_message("Error : PCI BIOS not found.\n");
                        return;
                }
                device = 0;
                while (audio_dev_list[device].vender_id != 0x0000) {
                        audio_pci.vender_id         = audio_dev_list[device].vender_id;
                        audio_pci.device_id         = audio_dev_list[device].device_id;
                        audio_pci.sub_vender_id     = audio_dev_list[device].sub_vender_id;
                        audio_pci.sub_device_id     = audio_dev_list[device].sub_device_id;
                        if (pci_find_device() == TRUE){
                                #ifdef AC97_DEBUG
                                logmessage("%s found.\n", audio_dev_list[device].string);
                                #endif
                                break;
                        }
                        device++;
                }
                if (audio_dev_list[device].vender_id == NULL) {
                        ac97_set_error_message("Error : AC97/HDA Audio Codec compatible device could not be found.\n");
                        return;
                }

                // save device info
                audio_pci.device_type = audio_dev_list[device].type;
                strcpy (audio_pci.device_name, audio_dev_list[device].string);

                // read pci configuration
                audio_pci.command = pci_config_read_word (PCI_COMMAND);
                audio_pci.irq = pci_config_read_byte (PCI_INTERRUPT_LINE);
                audio_pci.pin = pci_config_read_byte (PCI_INT_LINE);
                audio_pci.base0 = pci_config_read_dword (PCI_BASE_ADDRESS_0);   // NAMBAR
                audio_pci.base1 = pci_config_read_dword (PCI_BASE_ADDRESS_1);   // NABMBAR

                #ifdef AC97_DEBUG
                logmessage("AC97/HDA Audio Codec PCI IRQ at %d\n", audio_pci.irq);
                logmessage("AC97/HDA Audio Codec IRQ PIN nr %d\n", audio_pci.pin);
                #endif

                // if memory type IO is enabled then use memory type IO (other manufacturers)
                if (audio_pci.command & PCI_COMMAND_MEMORY) audio_pci.mem_mode = 1;

                // start configuring devices
                switch (audio_pci.device_type) {
                        case DEVICE_INTEL_ICH4:
                                // try to go for memory type IO as default for ICH4+ Intel controllers even if disabled in PCI command config
                                audio_pci.mem_mode = 1;
                                break;
                        case DEVICE_NFORCE:
                                pci_config_write_dword (0x4c, pci_config_read_dword (0x4c) | 0x1000000);
                                break;

                        case DEVICE_HDA_INTEL:
                                audio_pci.hda_mode = 1;
                                audio_pci.mem_mode = 1;
                                break;
                        case DEVICE_HDA_ATI:
                                audio_pci.hda_mode = 1;
                                audio_pci.mem_mode = 1;
                                // enable snoop for ATI SB450 Azalia HD Audio
                                pci_config_write_byte (0x42, (pci_config_read_byte (0x42) & 0xf8) | 0x2);
                                break;
                        case DEVICE_HDA_NVIDIA:
                                audio_pci.hda_mode = 1;
                                audio_pci.mem_mode = 1;
                                // enable snoop for nVidia Azalia HD Audio
                                pci_config_write_byte (0x4e, (pci_config_read_byte (0x4e) & 0xf0) | 0x0f);
                        break;
                                case DEVICE_HDA_SIS:
                                audio_pci.hda_mode = 1;
                                audio_pci.mem_mode = 1;
                                break;
                        case DEVICE_HDA_ULI:
                                audio_pci.hda_mode = 1;
                                audio_pci.mem_mode = 1;
                                pci_config_write_word (0x40, pci_config_read_word (0x40) | 0x10);
                                pci_config_write_dword (PCI_MEM_BASE_ADDRESS_1, 0);
                                break;
                        case DEVICE_HDA_VIA:
                                audio_pci.hda_mode = 1;
                                audio_pci.mem_mode = 1;
                                break;

                        default:
                                break;
                }

                // HDA configuration
                if (audio_pci.hda_mode) {
                        audio_pci.base0 = pci_config_read_dword (PCI_MEM_BASE_ADDRESS_0);   // AZBAR
                        audio_pci.base0 &= ~7;
                        #ifdef AC97_DEBUG
                        logmessage("HDA Audio Codec PCI BASE0 at I/O %04X\n", audio_pci.base0);
                        #endif

                        // map linear memory - convert physical address to linear address
                        if (!DPMI_MapMemory ((unsigned long *)&audio_pci.base0, (unsigned long *)&audio_pci.base0, 0x4000)) audio_pci.mem_mode = 0;

                        // enable memory, IO and bus master - activate the device
                        audio_pci.command |= PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
                        pci_config_write_word (PCI_COMMAND, audio_pci.command);

                        // Clear bits 0-2 of PCI register TCSEL (Traffic Class Select Register at offset 0x44)
                        // Ensuring these bits are 0 clears playback static on some Azalia HD Audio codecs
                        pci_config_write_byte (0x44, pci_config_read_byte (0x44) & 0xf8);

                        // HDA config done now we need to initialize the codec

                 // AC97 configuration
                } else {
                        // get I/O base0 port - this is the Audio Mixer base port
                        audio_pci.base0 = pci_config_read_dword (PCI_BASE_ADDRESS_0);                   // NAMBAR
                        // Remove I/O space marker in bit 0
                        audio_pci.base0 &= ~0xf;
                        #ifdef AC97_DEBUG
                        logmessage("AC97 Audio Codec PCI BASE0 at I/O %04X\n", audio_pci.base0);
                        #endif

                        // get I/O base1 port - this is the Bus Master base port
                        audio_pci.base1 = pci_config_read_dword (PCI_BASE_ADDRESS_1);                   // NABMBAR
                        // Remove I/O space marker in bit 0
                        audio_pci.base1 &= ~0xf;
                        judascfg_port = audio_pci.base1;
                        #ifdef AC97_DEBUG
                        logmessage("AC97 Audio Codec PCI BASE1 at I/O %04X\n", audio_pci.base1);
                        #endif

                        // get memory mapped IO for ICH4+ and other new chips
                        if ((audio_pci.base0 == 0) || (audio_pci.mem_mode)) {
                                audio_pci.base2 = pci_config_read_dword (PCI_MEM_BASE_ADDRESS_2);       // NAMBAR
                                audio_pci.base3 = pci_config_read_dword (PCI_MEM_BASE_ADDRESS_3);       // NABMBAR

                                // map linear memory - convery physical address to linear address
                                if (!DPMI_MapMemory ((unsigned long *)&audio_pci.base2, (unsigned long *)&audio_pci.base2, 0x1000)) audio_pci.mem_mode = FALSE;
                                if (!DPMI_MapMemory ((unsigned long *)&audio_pci.base3, (unsigned long *)&audio_pci.base3, 0x1000)) audio_pci.mem_mode = FALSE;

                                // for information purposes only
                                if (audio_pci.mem_mode) judascfg_port = audio_pci.base3;
                        }

                        // legacy I/O access
                        if (audio_pci.mem_mode == FALSE) {
                                if (audio_pci.device_type == DEVICE_INTEL_ICH4) {
                                        // enable the IOSE bit in 0x41 for legacy mode for ICH4/ICH5
                                        pci_config_write_byte (PCI_ICH4_CFG_REG, 1);
                                        // Set the secondary codec ID
                                        pci_config_write_byte (PCI_COMMAND_PARITY, 0x39);
                                }

                                // enable IO and bus master
                                audio_pci.command |= PCI_COMMAND_MASTER | PCI_COMMAND_IO;
                                pci_config_write_word (PCI_COMMAND, audio_pci.command);

                        } else {
                                //  enable port IO access compatibility even though we're going for memory mapped IO !!!
                                if (audio_pci.device_type == DEVICE_INTEL_ICH4) {
                                        // enable the IOSE bit in 0x41 for legacy mode for ICH4/ICH5
                                        pci_config_write_byte (PCI_ICH4_CFG_REG, 1);
                                        // Set the secondary codec ID
                                        pci_config_write_byte (PCI_COMMAND_PARITY, 0x39);
                                }

                                // enable memory, IO and bus master
                                audio_pci.command |= PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY;
                                pci_config_write_word (PCI_COMMAND, audio_pci.command);
                        }

                        // AC97 config done now we need to initialize the codec
                }

                // test if both I/O range and memory mapped range are NULL
                // note : memory range address will be NULL if DPMI_MapMemory function fails for some reason
                if ((audio_pci.base0 == 0) && (audio_pci.mem_mode == 0)) {
                        ac97_set_error_message("Error : AC97/HDA normal I/O and memory mapped I/O access failed!\n");
                        if (audio_pci.base0) DPMI_UnmapMemory ((unsigned long *)&audio_pci.base0);
                        if (audio_pci.base2) DPMI_UnmapMemory ((unsigned long *)&audio_pci.base2);
                        if (audio_pci.base3) DPMI_UnmapMemory ((unsigned long *)&audio_pci.base3);
                        return;                              // still DEV_NOSOUND at this point
                }

                if ((audio_pci.base0 == 0) && (audio_pci.base2 == 0)) {
                        ac97_set_error_message("Error : AC97/HDA Audio Codec device found, but disabled.\n");
                        if (audio_pci.base0) DPMI_UnmapMemory ((unsigned long *)&audio_pci.base0);
                        if (audio_pci.base2) DPMI_UnmapMemory ((unsigned long *)&audio_pci.base2);
                        if (audio_pci.base3) DPMI_UnmapMemory ((unsigned long *)&audio_pci.base3);
                        return;                              // still DEV_NOSOUND at this point
                }

                if (audio_pci.hda_mode) judascfg_device = DEV_HDA;
                else judascfg_device = DEV_AC97;
        }
}

int judas_init (unsigned mixrate, unsigned mixer, unsigned mixmode, int interpolation)
{
        int sbrate = 0;

        /*
                      enable near pointers for DJGPP
                      some DPMI hosts might reject this call
                      */
        judas_error = JUDAS_ILLEGAL_CONFIG;
        #ifdef __DJGPP__
        if (__djgpp_nearptr_enable() == 0) return 0;
        /* Trick: Avoid re-setting DS selector limit on each memory allocation call */
            __djgpp_selector_limit = 0xffffffff;
        #endif

        /*
                       * Check for illegal values
                      */
        judas_error = JUDAS_ILLEGAL_CONFIG;
        if (judascfg_device > DEV_FILE) return 0;
        if (judascfg_device != DEV_NOSOUND && judascfg_device != DEV_AC97 && judascfg_device != DEV_HDA && judascfg_device != DEV_FILE)
        {
                if (judascfg_port < 0x200) return 0;
                if (judascfg_port > 0x2ff) return 0;
                if (judascfg_irq < 2) return 0;
                if (judascfg_irq > 15) return 0;
                if (judascfg_dma1 > 7) return 0;
                if (judascfg_device == DEV_SB16)
                {
                        if (judascfg_dma2 > 7) return 0;
                }
                if (judascfg_irq == 9) judascfg_irq = 2;
        }
        if (mixrate < 5000) mixrate = 5000;
        if (mixrate > 44100 && judascfg_device != DEV_AC97 && judascfg_device != DEV_HDA) mixrate = 44100;
        if ((mixer != FASTMIXER) && (mixer != QUALITYMIXER)) return 0;

        /* enable pci bus master and i/o access for AC97 */
        if (judascfg_device == DEV_AC97)
        {
                pci_enable_busmaster();
                pci_enable_io_access();
                if (audio_pci.mem_mode) pci_enable_memory_access();
        }

        /* enable pci bus master and mem access for HDA */
        if (judascfg_device == DEV_HDA)
        {
                pci_enable_busmaster();
                if (audio_pci.mem_mode) pci_enable_memory_access();
        }

        /*
                       * If user wants to re-initialize, shutdown first
                      */
        if (judas_initialized) judas_uninit();
        judas_mixrate = mixrate;
        judas_mixmode = mixmode & (SIXTEENBIT | STEREO);
        judas_mixer = mixer;

        /*
                       * Copy the config to currently used values
                       */
        judas_device = judascfg_device;
        judas_port = judascfg_port;
        judas_irq = judascfg_irq;
        judas_dma = judascfg_dma1;
        if (mixer == QUALITYMIXER) {
                judas_mixersys = &qmixer;
                if (interpolation) judas_mixroutine = &qmix_cubic;
                else judas_mixroutine = &qmix_linear;
                judas_ipmodename[0] = "Linear";
                judas_ipmodename[1] = "Cubic";
        } else {
                judas_ipmodename[0] = "No";
                judas_ipmodename[1] = "Linear";
                judas_mixersys = &fmixer;
                if (interpolation) judas_mixroutine = &ipmix;
                else judas_mixroutine = &normalmix;
        }

        /*
                       * If it's NOSOUND, don't go further
                      */
        if (judas_device == DEV_NOSOUND)
        {
                judas_error = JUDAS_OK;
                return 1;
        }

        /*
                       * Detect soundcard
                       */
        judas_error = JUDAS_HARDWARE_ERROR;
        switch (judas_device)
        {
                case DEV_SB:
                {
                        sbrate = 256 - 1000000 / judas_mixrate;
                        if (sbrate > 210) sbrate = 210;
                        judas_mixrate = 1000000 / (256 - sbrate);
                }
                if (!sb_reset()) return 0;
                sb_getversion();

                #ifdef __DJGPP__
                /* DJGPP version */
                if (dsp_version > 0x200) judas_newvect.pm_offset = (unsigned long) sb_aihandler;
                else judas_newvect.pm_offset = (unsigned long) sb_handler;
                judas_newvect.pm_selector = (unsigned short) _go32_my_cs();

                #else
                /* WATCOM C++ version */
                if (dsp_version > 0x200) judas_newvect = &sb_aihandler;
                else judas_newvect = &sb_handler;
                #endif

                judas_mixmode = EIGHTBIT | MONO;
                break;

                case DEV_SBPRO:
                if (judas_mixmode & STEREO)
                {
                        sbrate = 256 - 500000 / judas_mixrate;
                        if (sbrate > 233) sbrate = 233;
                        judas_mixrate = 500000 / (256 - sbrate);
                }
                else
                {
                        sbrate = 256 - 1000000 / judas_mixrate;
                        if (sbrate > 233) sbrate = 233;
                        judas_mixrate = 1000000 / (256 - sbrate);
                }
                if (!sb_reset()) return 0;
                sb_getversion();
                if (dsp_version < 0x300) return 0;

                #ifdef __DJGPP__
                /* DJGPP version */
                judas_newvect.pm_offset = (unsigned long) sb_aihandler;
                judas_newvect.pm_selector = (unsigned short) _go32_my_cs();

                #else
                /* WATCOM C++ version */
                judas_newvect = &sb_aihandler;
                #endif

                judas_mixmode &= STEREO;
                break;

                case DEV_SB16:
                if (!sb_reset()) return 0;
                sb_getversion();
                if (dsp_version < 0x400) return 0;

                #ifdef __DJGPP__
                /* DJGPP version */
                if (judas_mixmode & SIXTEENBIT)
                {
                        judas_dma = judascfg_dma2;
                        judas_newvect.pm_offset = (unsigned long) sb16_handler;
                }
                else judas_newvect.pm_offset = (unsigned long) sb_aihandler;
                judas_newvect.pm_selector = (unsigned short) _go32_my_cs();

                #else
                /* WATCOM C++ version */
                if (judas_mixmode & SIXTEENBIT)
                {
                        judas_dma = judascfg_dma2;
                        judas_newvect = &sb16_handler;
                }
                else judas_newvect = &sb_aihandler;
                #endif
                break;

                case DEV_GUS:
                if (!gus_detect()) return 0;
                gus_reset();
                gus_dmainit();
                {
                        unsigned gus_rate;

                        /*
                                                                  * This stupidity is needed to keep mixrate above 5000
                                                                  */
                        if (judas_mixrate < 5100) judas_mixrate = 5100;
                        gus_rate = (judas_mixrate << 9) / 44100;
                        judas_mixrate = (gus_rate * 44100) >> 9;
                }

                #ifdef __DJGPP__
                /* DJGPP version */
                judas_newvect.pm_offset = (unsigned long) gus_handler;
                judas_newvect.pm_selector = (unsigned short) _go32_my_cs();

                #else
                /* WATCOM C++ version */
                judas_newvect = &gus_handler;
                #endif
                break;

                case DEV_AC97:
                if (!ac97_reset()) return 0;
                /* only 7 rates supported by AC97 - 48000Hz included */
                if (judas_mixrate < 11025) judas_mixrate = 8000;
                else if (judas_mixrate < 16000) judas_mixrate = 11025;
                else if (judas_mixrate < 22050) judas_mixrate = 16000;
                else if (judas_mixrate < 32000) judas_mixrate = 22050;
                else if (judas_mixrate < 44100) judas_mixrate = 32000;
                else if (judas_mixrate < 48000) judas_mixrate = 44100;
                else if (judas_mixrate > 48000) judas_mixrate = 48000;
                if (!audio_pci.ac97_vra_supported) judas_mixrate = 48000;
                /* force 16-bit stereo output */
                judas_mixmode = STEREO | SIXTEENBIT;
                break;

                case DEV_HDA:
                /* reset, initialize the HDA and setup output node */
                if (!hda_reset()) return 0;
                /* only 7 rates supported by HDA - 48000Hz included */
                if (judas_mixrate < 11025) judas_mixrate = 8000;
                else if (judas_mixrate < 16000) judas_mixrate = 11025;
                else if (judas_mixrate < 22050) judas_mixrate = 16000;
                else if (judas_mixrate < 32000) judas_mixrate = 22050;
                else if (judas_mixrate < 44100) judas_mixrate = 32000;
                else if (judas_mixrate < 48000) judas_mixrate = 44100;
                else if (judas_mixrate > 48000) judas_mixrate = 48000;
                /* force 16-bit stereo output */
                judas_mixmode = STEREO | SIXTEENBIT;
                break;
        }

        /*
                       * Calculate sample size & buffer length, set initial mixing pos.
                       */
        judas_samplesize = 1;
        judas_buffermask = 0xfffffff8;
        if (judas_mixmode & STEREO)
        {
                judas_samplesize <<= 1;
                judas_buffermask <<= 1;
        }
        if (judas_mixmode & SIXTEENBIT)
        {
                judas_samplesize <<= 1;
                judas_buffermask <<= 1;
        }
        /*
                       * For Standard GUS, mask is always 64 bytes to ensure proper DMA
                       * transfer alignment even with 16-bit DMA + stereo output.
                       */
        if (judas_device == DEV_GUS)
        {
                judas_buffermask = 0xffffffc0;
        }
        if (judas_device == DEV_HDA)
        {
                judas_buffermask = 0xffffff00;
        }
        judas_bufferlength = judas_mixrate / PER_SECOND * judas_samplesize;
        if (judas_bufferlength < 512) judas_bufferlength = 512;
        if (judas_bufferlength > (DMA_MAXSIZE - 64)) judas_bufferlength = DMA_MAXSIZE - 64;
        judas_bufferlength &= judas_buffermask;
        judas_mixpos = 0;

        /*
                       * Reserve dma buffer, initialize mixer tables and lock JUDAS code &
                       * data, each of them must be successful
                       */
        judas_error = JUDAS_OUT_OF_MEMORY;
        if (judas_device != DEV_FILE && judas_device != DEV_AC97 && judas_device != DEV_HDA) {
                if (!dma_reserve(44100 / PER_SECOND * 4 + 64)) return 0;
        }

        if (judas_device == DEV_AC97) {
                int count;
                audio_pci.pcmout_buffer0 = (DWORD *)dos_malloc(judas_bufferlength);
                audio_pci.pcmout_buffer1 = (DWORD *)dos_malloc(judas_bufferlength);
                if (!audio_pci.pcmout_buffer0 || !audio_pci.pcmout_buffer1) return 0;
                audio_pci.bdl_buffer = (DWORD *)dos_malloc (256);   // allocate memory for BDL (256 bytes)
                if (!audio_pci.bdl_buffer) return 0;
                count = 0;
                while (count < 64){
                        // in low DOS memory
                        *(audio_pci.bdl_buffer + count + (__djgpp_conventional_base >> 2)) = (DWORD) audio_pci.pcmout_buffer0;
                        *(audio_pci.bdl_buffer + 2 + count + (__djgpp_conventional_base >> 2)) = (DWORD) audio_pci.pcmout_buffer1;
                        if(audio_pci.device_type == DEVICE_SIS){        // bit 30 - BUP enabled
                                *(audio_pci.bdl_buffer + 1 + count + (__djgpp_conventional_base >> 2)) = BIT30 + judas_bufferlength;          // in bytes
                                *(audio_pci.bdl_buffer + 3 + count + (__djgpp_conventional_base >> 2)) = BIT30 + judas_bufferlength;          // in bytes
                        }else{
                                *(audio_pci.bdl_buffer + 1 + count + (__djgpp_conventional_base >> 2)) = BIT30 + (judas_bufferlength >> 1);   // in samples (16bit stereo)
                                *(audio_pci.bdl_buffer + 3 + count + (__djgpp_conventional_base >> 2)) = BIT30 + (judas_bufferlength >> 1);   // in samples (16bit stereo)
                        }
                        count += 4;
                }
                ich_OUTL ((DWORD)audio_pci.bdl_buffer, CTL_BASE, ICH_REG_PO_BDBAR);   // set BDL PCM OUT address (low DOS memory)
                ich_OUTB (0x1F, CTL_BASE, ICH_REG_PO_LVI);                // set LVI to 31 (last index)
        }

        if (judas_device == DEV_HDA) {
                int count;
                unsigned int mem_aligned;

                // clear the BDL info in HDA codec (just in case)
                int timeout;
                unsigned char val;
                unsigned long uval;

                //  stop DMA engine if playing and disable interrupts (Interrupt on Completion, FIFO Error Interrupt, Descriptor Error Interrupt) - RUN bit set to 0
                hda_OUTB (HDA_SDO0CTL, hda_INB (HDA_SDO0CTL) & ~(SD_CTL_DMA_START | SD_INT_MASK));
                delay (1);

                // Software must read a 0 from the DMA run bit before modifying related control registers or restarting the DMA engine
                timeout = 300;
                while (timeout--) {
                        if (!(hda_INB (HDA_SDO0CTL) & SD_CTL_DMA_START)) break;
                        delay (1);
                }

                // reset the HDA stream 0
                hda_OUTB (HDA_SDO0CTL, hda_INB (HDA_SDO0CTL) | SD_CTL_STREAM_RESET);
                delay (1);
                
                timeout = 300;
                while (!((val = hda_INB (HDA_SDO0CTL)) & SD_CTL_STREAM_RESET) && --timeout) delay(1);
                val &= ~SD_CTL_STREAM_RESET;
                hda_OUTB (HDA_SDO0CTL, val);
                delay (1);
                
                // wait for out of reset confirmation from hardware
                timeout = 300;
                while (((val = hda_INB (HDA_SDO0CTL)) & SD_CTL_STREAM_RESET) && --timeout) delay(1);

                // clear stream 0 data since we're not using other output streams anyway
                hda_OUTL (HDA_DPLBASE, 0);     // DMA Position Lower Base Address
                hda_OUTL (HDA_DPUBASE, 0);     // DMA Position Upper Base Address
                hda_OUTL (HDA_SDO0BDLPL, 0);   // SD0 outpuy BDL - lower
                hda_OUTL (HDA_SDO0BDLPU, 0);   // SD0 output BDL - upper
                // hda_OUTL ( HDA_SDO0CTL, 0);     // SD0 output stream descriptor control

                // prepare BDL in low DOS memory - we will have at least 32 entries in the BDL (but allocate for 256 entries)
                // perform 4096 bytes boundary alignement - page align
                audio_pci.hda_buffer = (DWORD *)dos_malloc ((judas_bufferlength * 2) + 8192 + (4096 * 3));
                if (!audio_pci.hda_buffer) return 0;
                // set buffer 0
                mem_aligned = (((unsigned int)audio_pci.hda_buffer + 4095) & (~4095));
                audio_pci.pcmout_buffer0 = (unsigned int *) mem_aligned;

                // set buffer 1
                mem_aligned += judas_bufferlength;
                mem_aligned = ((mem_aligned + 4095) & (~4095));
                audio_pci.pcmout_buffer1 = (unsigned int *) mem_aligned;

                // set BDL and clear it
                mem_aligned += judas_bufferlength;
                mem_aligned = ((mem_aligned + 4095) & (~4095));
                audio_pci.bdl_buffer = (unsigned int *) mem_aligned;
                memset (audio_pci.bdl_buffer, 0, 8192);

                count = 0;
                while (count < 128) {
                        // in low DOS memory (address lower 32bits, address upper 32bits, size field of BDL entry, IOC flag)
                        unsigned int off = count << 3; /* 8 dword step */
                                                // 4 DWORD steps
                        *(audio_pci.bdl_buffer + 0 + off + (__djgpp_conventional_base >> 2)) = (DWORD) audio_pci.pcmout_buffer0;
                        *(audio_pci.bdl_buffer + 1 + off + (__djgpp_conventional_base >> 2)) = 0;
                        *(audio_pci.bdl_buffer + 2 + off + (__djgpp_conventional_base >> 2)) = judas_bufferlength;
                        *(audio_pci.bdl_buffer + 3 + off + (__djgpp_conventional_base >> 2)) = 0;   // no IOC
                        // 4 DWORD steps
                        *(audio_pci.bdl_buffer + 4 + off + (__djgpp_conventional_base >> 2)) = (DWORD) audio_pci.pcmout_buffer1;
                        *(audio_pci.bdl_buffer + 5 + off + (__djgpp_conventional_base >> 2)) = 0;
                        *(audio_pci.bdl_buffer + 6 + off + (__djgpp_conventional_base >> 2)) = judas_bufferlength;
                        *(audio_pci.bdl_buffer + 7 + off + (__djgpp_conventional_base >> 2)) = 0;   // no IOC
                        count++;
                }

                // program the stream tag - bits 20 - 23 of SD_CTL register
                audio_pci.stream_tag = 1;
                hda_OUTL (HDA_SDO0CTL, (hda_INL (HDA_SDO0CTL) & ~SD_CTL_STREAM_TAG_MASK) | (audio_pci.stream_tag << SD_CTL_STREAM_TAG_SHIFT));
                delay (1);
                uval = hda_INL (HDA_SDO0CTL);
                
                // program the length of samples in cyclic buffer
                hda_OUTL (HDA_SDO0CBL, judas_bufferlength);
                delay (1);
                uval = hda_INL (HDA_SDO0CBL);

                // program the stream format on the controller - this value needs to be the same as the one programmed later for output widget (16bits PCM)
                audio_pci.dacout_num_bits = 16;
                audio_pci.dacout_num_channels = 2;
                audio_pci.format_val = hda_calc_stream_format (judas_mixrate);
                hda_OUTW (HDA_SDO0FORMAT, audio_pci.format_val);
                delay (1);
                uval = hda_INW (HDA_SDO0FORMAT);

                // program the stream LVI (last valid index)  of the BDL - set it to 31
                hda_OUTW (HDA_SDO0LVI, 31);
                delay (1);
                uval = hda_INW (HDA_SDO0LVI);

                // program the BDL address (lower 32bits and upper 32bits)
                hda_OUTL (HDA_SDO0BDLPL, (DWORD) audio_pci.bdl_buffer);
                delay (1);
                uval = hda_INL (HDA_SDO0BDLPL);
                hda_OUTL (HDA_SDO0BDLPU, 0);
                delay (1);
                uval = hda_INL (HDA_SDO0BDLPU);

                // enable the DMA position buffer to have info on current DMA position which is updated every frame by hardware (not initialized for JUDAS at this stage)
                // if (!(hda_INL ( HDA_DPLBASE) & HDA_DPLBASE_ENABLE)) hda_OUTL ( HDA_DPLBASE, audio_pci.pcmout_dma_pos_ptr | HDA_DPLBASE_ENABLE);
                // disable the DMA position buffer
                hda_OUTL (HDA_DPLBASE, hda_INL (HDA_DPLBASE) & ~HDA_DPLBASE_ENABLE);
                delay (1);
                uval = hda_INL (HDA_DPLBASE);

                // setup stream 1 - link it to our output widget programmed earlier (channel ID is 0) and program the same format as on the controller (16bits PCM)
                if (audio_pci.dac_node[0]) {
                        hda_codec_write (audio_pci.dac_node[0]->nid, 0, AC_VERB_SET_CHANNEL_STREAMID, (audio_pci.stream_tag << 4) | 0);
                        delay (150);
                        hda_codec_write (audio_pci.dac_node[0]->nid, 0, AC_VERB_SET_STREAM_FORMAT, audio_pci.format_val);
                }
                if (audio_pci.dac_node[1]) {
                        hda_codec_write (audio_pci.dac_node[0]->nid, 0, AC_VERB_SET_CHANNEL_STREAMID, (audio_pci.stream_tag << 4) | 0);
                        delay (150);
                        hda_codec_write (audio_pci.dac_node[0]->nid, 0, AC_VERB_SET_STREAM_FORMAT, audio_pci.format_val);
                }
        }

        if (!initmixer()) return 0;
        if (!judas_lock()) return 0;
        judas_ds = judas_get_ds();

        /*
                       * Unmask IRQ & set vector
                       */
        if (judas_device != DEV_FILE && judas_device != DEV_AC97 && judas_device != DEV_HDA) {
          judas_oldpicmask1 = inp(PICMASK1);
          judas_oldpicmask2 = inp(PICMASK2);
          if (judas_irq < 8)
          {
                  outp(PICMASK1, judas_oldpicmask1 & ~(1 << judas_irq));
                  judas_int = judas_irq + 0x8;
          }
          else
          {
                  outp(PICMASK1, judas_oldpicmask1 & 0xfb);
                  outp(PICMASK2, judas_oldpicmask2 & ~(1 << (judas_irq & 3)));
                  judas_int = judas_irq + 0x68;
          }

          #ifdef __DJGPP__
          /* DJGPP version */
          _go32_dpmi_get_protected_mode_interrupt_vector(judas_int, &judas_oldvect);
          printf("Old handler at %04X:%08X\n", judas_oldvect.pm_selector, (unsigned int)judas_oldvect.pm_offset);
          printf("New handler at %04X:%08X\n", judas_newvect.pm_selector, (unsigned int)judas_newvect.pm_offset);
//          _go32_dpmi_allocate_iret_wrapper(&judas_newvect);
//          printf("New wrapper at %04X:%08X\n", judas_newvect.pm_selector, (unsigned int)judas_newvect.pm_offset);
          _go32_dpmi_set_protected_mode_interrupt_vector(judas_int, &judas_newvect);

                  #else
          /* WATCOM C++ version */
          judas_oldvect = _dos_getvect(judas_int);
          _dos_setvect(judas_int, judas_newvect);
          #endif
        }

        /*
                       * Everything is prepared. Now just set up the soundcard for output!
                       */
        switch (judas_device)
        {
                case DEV_SB:
                memset((char *)dma_address + __djgpp_conventional_base, 0x80, judas_bufferlength);
                dma_program(DMA_WRITE_LOOP, 0, judas_bufferlength);
                sb_write(0xd1);
                sb_write(0x40);
                sb_write(sbrate);
                if (dsp_version <= 0x200)
                {
                        sb_write(0x14);
                        sb_write(0xf0);
                        sb_write(0xff);
                }
                else
                {
                        sb_write(0x48);
                        sb_write(0xf0);
                        sb_write(0xff);
                        sb_write(0x1c);
                }
                break;

                case DEV_SBPRO:
                memset((char *)dma_address + __djgpp_conventional_base, 0x80, judas_bufferlength);
                if (judas_mixmode & STEREO)
                {
                        int timeout = 0xfffff;

                        outp(judas_port + 4, 0xe);
                        outp(judas_port + 5, inp(judas_port + 5) | 0x2);
                        /*
                                                                  * To make left & right correct, send one silent
                                                                  * byte with singlecycle output, and wait for the
                                                                  * transfer to complete.
                                                                  */
                        judas_irqcount = 0;
                        dma_program(DMA_WRITE_ONESHOT, 0, 1);
                        sb_write(0x14);
                        sb_write(0x0);
                        sb_write(0x0);
                        while (!judas_irqcount)
                        {
                                timeout--;
                                if (!timeout) break;
                        }
                }
                else
                {
                        outp(judas_port + 4, 0xe);
                        outp(judas_port + 5, inp(judas_port + 5) & 0xfd);
                }
                dma_program(DMA_WRITE_LOOP, 0, judas_bufferlength);
                sb_write(0xd1);
                sb_write(0x40);
                sb_write(sbrate);
                sb_write(0x48);
                sb_write(0xf0);
                sb_write(0xff);
                /* Use highspeed mode if timeconstant > 210 */
                if (sbrate > 210) sb_write(0x90);
                else sb_write(0x1c);
                break;

                case DEV_SB16:
                sb_write(0x41);
                sb_write(judas_mixrate >> 8);
                sb_write(judas_mixrate & 0xff);
                if (judas_mixmode & SIXTEENBIT) memset((char *)dma_address + __djgpp_conventional_base, 0, judas_bufferlength);
                else memset((char *)dma_address + __djgpp_conventional_base, 0x80, judas_bufferlength);
                dma_program(DMA_WRITE_LOOP, 0, judas_bufferlength);
                if (judas_mixmode & SIXTEENBIT)
                {
                        if (judas_mixmode & STEREO)
                        {
                                sb_write(0xb6);
                                sb_write(0x30);
                                sb_write(0xf0);
                                sb_write(0xff);
                        }
                        else
                        {
                                sb_write(0xb6);
                                sb_write(0x10);
                                sb_write(0xf0);
                                sb_write(0xff);
                        }
                }
                else
                {
                        if (judas_mixmode & STEREO)
                        {
                                sb_write(0xc6);
                                sb_write(0x20);
                                sb_write(0xf0);
                                sb_write(0xff);
                        }
                        else
                        {
                                sb_write(0xc6);
                                sb_write(0x00);
                                sb_write(0xf0);
                                sb_write(0xff);
                        }
                }
                break;

                case DEV_GUS:
                if (judas_mixmode & SIXTEENBIT) memset((char *)dma_address + __djgpp_conventional_base, 0, judas_bufferlength + 64);
                else memset((char *)dma_address + __djgpp_conventional_base, 0x80, judas_bufferlength + 64);
                gus_dmaprogram(0, judas_bufferlength + 64);
                gus_dmawait();
                gus_setupchannels();
                gus_startchannels();
                break;

                case DEV_AC97:
                /* clear for 8 and 16bit - although now ac97 will only work with 16bit */
                if (judas_mixmode & SIXTEENBIT) memset((char *)audio_pci.pcmout_buffer0 + __djgpp_conventional_base, 0, judas_bufferlength);
                else memset((char *)audio_pci.pcmout_buffer0 + __djgpp_conventional_base, 0x80, judas_bufferlength);
                if (judas_mixmode & SIXTEENBIT) memset((char *)audio_pci.pcmout_buffer1 + __djgpp_conventional_base, 0, judas_bufferlength);
                else memset((char *)audio_pci.pcmout_buffer1 + __djgpp_conventional_base, 0x80, judas_bufferlength);
                /* 16bit PCM, stereo sound */
                {
                        DWORD data;
                        data = ich_INL (CTL_BASE, ICH_REG_GLOB_CNT);
                        switch (audio_pci.device_type) {
                                case DEVICE_SIS:
                                data = data & ~(ICH_SIS_PCM_246_MASK | ICH_20_BIT | ICH_24_BIT);
                                data = data & ~ICH_GIE;
                                break;

                                /* as for DEVICE_INTEL */
                                default:
                                data = data & ~(ICH_PCM_246_MASK | ICH_20_BIT | ICH_24_BIT);
                                data = data & ~ICH_GIE;
                                break;
                        }

                        ich_OUTL (data, CTL_BASE, ICH_REG_GLOB_CNT);
                        if (audio_pci.ac97_vra_supported) ac97_write_codec (AC97_PCM_FRONT_DAC_RATE, judas_mixrate);
                        delay(100);

                        /* start the Bus Master DMA engine */
                        ich_OUTB (ICH_STARTBM, CTL_BASE, ICH_REG_PO_CR);
                }
                break;

                case DEV_HDA:
                /* clear for 8 and 16bit - although now ac97 will only work with 16bit */
                if (judas_mixmode & SIXTEENBIT) memset((char *)audio_pci.pcmout_buffer0 + __djgpp_conventional_base, 0, judas_bufferlength);
                else memset((char *)audio_pci.pcmout_buffer0 + __djgpp_conventional_base, 0x80, judas_bufferlength);
                if (judas_mixmode & SIXTEENBIT) memset((char *)audio_pci.pcmout_buffer1 + __djgpp_conventional_base, 0, judas_bufferlength);
                else memset((char *)audio_pci.pcmout_buffer1 + __djgpp_conventional_base, 0x80, judas_bufferlength);

                /* start the Bus Master DMA engine with 16bit PCM stereo sound programmed earlier */
//                hda_OUTB (HDA_SDO0CTL, hda_INB (HDA_SDO0CTL) | SD_CTL_DMA_START);
                break;

        }
        judas_initialized = 1;
        judas_clipped = 0;
        judas_error = JUDAS_OK;
        return 1;
}


void judas_uninit(void)
{
        judas_error = JUDAS_OK;
        if (!judas_initialized) return;
        judas_initialized = 0;
        /*
                       * Soundblaster is best shut down by doing a DSP reset twice. For SBPRO,
                       * we also deactivate the stereo mode. GUS is shut down by simply
                       * stopping the channels.
                       */
        switch (judas_device)
        {
                case DEV_SB:
                case DEV_SB16:
                sb_reset();
                sb_reset();
                break;

                case DEV_SBPRO:
                sb_reset();
                sb_reset();
                outp(judas_port + 4, 0xe);
                outp(judas_port + 5, inp(judas_port + 5) & 0xfd);
                break;

                case DEV_GUS:
                gus_stopchannel(0);
                gus_stopchannel(1);
                break;

                case DEV_AC97:
                ac97_stop();
                dos_free(audio_pci.bdl_buffer);
                dos_free(audio_pci.pcmout_buffer1);
                dos_free(audio_pci.pcmout_buffer0);
                break;

                case DEV_HDA:
                hda_codec_stop();
                if (audio_pci.hda_buffer) dos_free (audio_pci.hda_buffer);
                if (audio_pci.afg_nodes) locked_free (audio_pci.afg_nodes);
                break;
        }
        /*
                       * Then restore the PIC mask and IRQ vector.
                       */
        if (judas_device != DEV_FILE && judas_device != DEV_AC97 && judas_device != DEV_HDA) {
          outp(PICMASK1, judas_oldpicmask1);
          outp(PICMASK2, judas_oldpicmask2);
          #ifdef __DJGPP__
          /* DJGPP version */
          _go32_dpmi_set_protected_mode_interrupt_vector(judas_int, &judas_oldvect);
//          _go32_dpmi_free_iret_wrapper(&judas_newvect);
          #else
          /* WATCOM C++ version */
          _dos_setvect(judas_int, judas_oldvect);
          #endif
          }
}


/*****************************************************************************
   Sound Blaster stuff
*****************************************************************************/

static void sb_delay(void)
{
        unsigned char temp;
        char counter = 15;

        while (counter--) temp = inp(judas_port + 6);
}

static void sb_write(unsigned char value)
{
        int timeout = 0xfffff;
        while (inp(judas_port + 12) & 0x80)
        {
                timeout--;
                if (!timeout) return;
        }
        outp(judas_port + 12, value);
}

static unsigned char sb_read(void)
{
        int timeout = 0xfffff;
        while (!(inp(judas_port + 14) & 0x80))
        {
                timeout--;
                if (!timeout) return 0;
        }
        return inp(judas_port + 10);
}

static int sb_reset(void)
{
        outp(judas_port + 6, 1);
        sb_delay();
        outp(judas_port + 6, 0);
        if (sb_read() == 0xaa) return 1;
        return 0;
}

static void sb_getversion(void)
{
        sb_write(0xe1);
        dsp_version = sb_read() << 8;
        dsp_version += sb_read();
}


/*****************************************************************************
   GUS stuff
*****************************************************************************/

static void gus_delay(void)
{
        int count = 70;
        unsigned char temp;
        while (count--) temp = inp(judas_port);
}

static int gus_detect(void)
{
        outp(judas_port + GF1_REG_SELECT, MASTER_RESET);
        outp(judas_port + GF1_DATA_HI, 0x0);
        gus_delay();
        outp(judas_port + GF1_REG_SELECT, MASTER_RESET);
        outp(judas_port + GF1_DATA_HI, GF1_MASTER_RESET);
        gus_delay();
        gus_poke(0, 0xaa);
        gus_poke(1, 0x55);
        if (gus_peek(0) != 0xaa) return 0;
        return 1;
}

static void gus_reset(void)
{
        unsigned char temp;

        outp(judas_port + 0xf, 0x5);
        outp(judas_port, ENABLE_LINE_IN | ENABLE_OUTPUT);
        outp(judas_port + GF1_IRQ_CTRL, 0x0);
        outp(judas_port + 0xf, 0x0);
        outp(judas_port, ENABLE_LINE_IN | ENABLE_OUTPUT);
        outp(judas_port + GF1_IRQ_CTRL, gus_dmalatch[judas_dma] | 0x80);
        outp(judas_port, ENABLE_LINE_IN | ENABLE_OUTPUT | SELECT_GF1_REG);
        outp(judas_port + GF1_IRQ_CTRL, gus_irqlatch[judas_irq]);
        outp(judas_port, ENABLE_LINE_IN | ENABLE_OUTPUT);
        outp(judas_port + GF1_IRQ_CTRL, gus_dmalatch[judas_dma] | 0x80);
        outp(judas_port, ENABLE_LINE_IN | ENABLE_OUTPUT | SELECT_GF1_REG);
        outp(judas_port + GF1_IRQ_CTRL, gus_irqlatch[judas_irq]);
        outp(judas_port + GF1_PAGE, 0x0);
        outp(judas_port, ENABLE_LINE_IN | ENABLE_GF1_IRQ);
        outp(judas_port + GF1_REG_SELECT, DMA_CONTROL);
        outp(judas_port + GF1_DATA_HI, 0x0);
        outp(judas_port + GF1_REG_SELECT, TIMER_CONTROL);
        outp(judas_port + GF1_DATA_HI, 0x0);
        outp(judas_port + GF1_REG_SELECT, SAMPLE_CONTROL);
        outp(judas_port + GF1_DATA_HI, 0x0);
        outp(judas_port + GF1_REG_SELECT, SET_VOICES);
        outp(judas_port + GF1_DATA_HI, 13 | 0xc0);
        temp = inp(judas_port + GF1_IRQ_STAT);
        outp(judas_port + GF1_REG_SELECT, DMA_CONTROL);
        temp = inp(judas_port + GF1_DATA_HI);
        outp(judas_port + GF1_REG_SELECT, SAMPLE_CONTROL);
        temp = inp(judas_port + GF1_DATA_HI);
        outp(judas_port + GF1_REG_SELECT, GET_IRQV);
        temp = inp(judas_port + GF1_DATA_HI);
        for (temp = 0; temp < 32; temp++)
        {
                outp(judas_port + GF1_PAGE, temp);
                outp(judas_port + GF1_REG_SELECT, SET_CONTROL);
                outp(judas_port + GF1_DATA_HI, VOICE_STOPPED | STOP_VOICE);
                gus_delay();
                outp(judas_port + GF1_DATA_HI, VOICE_STOPPED | STOP_VOICE);
                outp(judas_port + GF1_REG_SELECT, SET_VOLUME_CONTROL);
                outp(judas_port + GF1_DATA_HI, VOLUME_STOPPED | STOP_VOLUME);
                gus_delay();
                outp(judas_port + GF1_DATA_HI, VOLUME_STOPPED | STOP_VOLUME);
                outp(judas_port + GF1_REG_SELECT, SET_VOLUME);
                outpw(judas_port + GF1_DATA_LOW, 0x0);
                outp(judas_port + GF1_REG_SELECT, SET_START_HIGH);
                outpw(judas_port + GF1_DATA_LOW, 0x0);
                outp(judas_port + GF1_REG_SELECT, SET_START_LOW);
                outpw(judas_port + GF1_DATA_LOW, 0x0);
                outp(judas_port + GF1_REG_SELECT, SET_END_HIGH);
                outpw(judas_port + GF1_DATA_LOW, 0x0);
                outp(judas_port + GF1_REG_SELECT, SET_END_LOW);
                outpw(judas_port + GF1_DATA_LOW, 0x0);
                outp(judas_port + GF1_REG_SELECT, SET_ACC_HIGH);
                outpw(judas_port + GF1_DATA_LOW, 0x0);
                outp(judas_port + GF1_REG_SELECT, SET_ACC_LOW);
                outpw(judas_port + GF1_DATA_LOW, 0x0);
        }
        temp = inp(judas_port + GF1_IRQ_STAT);
        outp(judas_port + GF1_REG_SELECT, DMA_CONTROL);
        temp = inp(judas_port + GF1_DATA_HI);
        outp(judas_port + GF1_REG_SELECT, SAMPLE_CONTROL);
        temp = inp(judas_port + GF1_DATA_HI);
        outp(judas_port + GF1_REG_SELECT, GET_IRQV);
        temp = inp(judas_port + GF1_DATA_HI);
        outp(judas_port + GF1_REG_SELECT, MASTER_RESET);
        outp(judas_port + GF1_DATA_HI, GF1_MASTER_RESET | GF1_OUTPUT_ENABLE | GF1_MASTER_IRQ);
}

static void gus_setupchannels(void)
{
        if (judas_mixmode & SIXTEENBIT)
        {
                if (judas_mixmode & STEREO)
                {
                        gus_setupchannel(0, 0, judas_bufferlength >> 2);
                        gus_setupchannel(1, (judas_bufferlength >> 2) + 16, judas_bufferlength >> 2);
                }
                else
                {
                        gus_setupchannel(0, 0, judas_bufferlength >> 1);
                        gus_setupchannel(1, 0, judas_bufferlength >> 1);
                }
        }
        else
        {
                if (judas_mixmode & STEREO)
                {
                        gus_setupchannel(0, 0, judas_bufferlength >> 1);
                        gus_setupchannel(1, (judas_bufferlength >> 1) + 32, judas_bufferlength >> 1);
                }
                else
                {
                        gus_setupchannel(0, 0, judas_bufferlength);
                        gus_setupchannel(1, 0, judas_bufferlength);
                }
        }
}

static void gus_setupchannel(unsigned char channel, unsigned start, unsigned length)
{
        length += start; /* Length is actually end address */
        outp(judas_port + GF1_PAGE, channel);
        outp(judas_port + GF1_REG_SELECT, SET_BALANCE);
        outp(judas_port + GF1_DATA_HI, channel * 15);
        outp(judas_port + GF1_REG_SELECT, SET_START_HIGH);
        outpw(judas_port + GF1_DATA_LOW, start >> 7);
        outp(judas_port + GF1_REG_SELECT, SET_START_LOW);
        outpw(judas_port + GF1_DATA_LOW, start << 9);
        outp(judas_port + GF1_REG_SELECT, SET_END_HIGH);
        outpw(judas_port + GF1_DATA_LOW, length >> 7);
        outp(judas_port + GF1_REG_SELECT, SET_END_LOW);
        outpw(judas_port + GF1_DATA_LOW, length << 9);
        outp(judas_port + GF1_REG_SELECT, SET_ACC_HIGH);
        outpw(judas_port + GF1_DATA_LOW, start >> 7);
        outp(judas_port + GF1_REG_SELECT, SET_ACC_LOW);
        outpw(judas_port + GF1_DATA_LOW, start << 9);
        outp(judas_port + GF1_REG_SELECT, SET_FREQUENCY);
        outpw(judas_port + GF1_DATA_LOW, (((judas_mixrate << 9) + 22050) / 44100) << 1);
        outp(judas_port + GF1_REG_SELECT, SET_VOLUME);
        outpw(judas_port + GF1_DATA_LOW, 0xf800);
}

static void gus_stopchannel(unsigned char channel)
{
        outp(judas_port + GF1_PAGE, channel);
        outp(judas_port + GF1_REG_SELECT, SET_VOLUME);
        outpw(judas_port + GF1_DATA_LOW, 0x0);
        outp(judas_port + GF1_REG_SELECT, SET_CONTROL);
        outp(judas_port + GF1_DATA_HI, VOICE_STOPPED | STOP_VOICE);
        gus_delay();
        outp(judas_port + GF1_DATA_HI, VOICE_STOPPED | STOP_VOICE);
}


/*****************************************************************************
        AC97 audio mixer registers access helper functions
******************************************************************************/

static BOOL ac97_codec_semaphore(void)
{
        DWORD status;
        int limit;

        // check if primary codec ready
        status = ich_INL (CTL_BASE, ICH_REG_GLOB_STA);                      // 30h global status register at NABMBAR
        if (!(status & ICH_PCR)) return FALSE;                              // not ready (not important here)

        // wait until codec ready - fail if wait limit exceeded
        limit = 100;
        while (limit)
        {
                status = ich_INB (CTL_BASE, ICH_REG_ACC_SEMA);              // 34h codec write semaphore register at NABMBAR
                if (!(status & ICH_CAS)) return TRUE;
                delay(10);
                limit--;
        }

                // clear semaphore flag
        ich_INW (MIXER_BASE, AC97_RESET);                                   // register reset the codec (0)

                return FALSE;   // busy (not important here)
}

static WORD ac97_read_codec(BYTE index)
{
        index &= 0xff;
        ac97_codec_semaphore();
        return ich_INW (MIXER_BASE, index);
}

static void ac97_write_codec(BYTE index, WORD data)
{
        index &= 0xff;
                ac97_codec_semaphore();
        ich_OUTW (data, MIXER_BASE, index);
}

static void ac97_stop(void)
{
        // stop all PCM out data
        ich_OUTB (0, CTL_BASE, ICH_REG_PO_CR);                              // control register at NABMBAR
        delay (100);                                                        // 100ms delay

        // reset PCM out registers
        ich_OUTB (ICH_RESETREGS, CTL_BASE, ICH_REG_PO_CR);                  // control register at NABMBAR
        delay (50);
}

static int ac97_reset(void)
{
        DWORD data;
        WORD wdata;

        // ICH4+ may fail when busmastering is enabled and memory mapped IO is on. Continue and ignore PCR status.
                if (!audio_pci.mem_mode) {
            // exit if primary codec is not ready
            if ((ich_INL (CTL_BASE, ICH_REG_GLOB_STA) & ICH_PCR) == 0) return 0;
                }

        // opt for 2 channels with 16bit samples (Global Control Register)
        // ACLink is open because we already have the primary codec ready status
        data = ich_INL (CTL_BASE, ICH_REG_GLOB_CNT);
        switch (audio_pci.device_type) {
                    case DEVICE_SIS:
                data = data & ~(ICH_SIS_PCM_246_MASK | ICH_20_BIT | ICH_24_BIT);
                                data = data & ~ICH_GIE;
                break;

                        /* as for DEVICE_INTEL */
                        default:
                                data = data & ~(ICH_PCM_246_MASK | ICH_20_BIT | ICH_24_BIT);
                data = data & ~ICH_GIE;
                                break;
                }
        ich_OUTL (data, CTL_BASE, ICH_REG_GLOB_CNT);

        // clear semaphore flag by reading Audio Mixer reset register
        ich_INW (MIXER_BASE, AC97_RESET);

        // disable interrupts
        ich_OUTB (0, CTL_BASE, ICH_REG_PI_CR);                              // 0Bh control register at NABMBAR
        ich_OUTB (0, CTL_BASE, ICH_REG_PO_CR);                              // 1Bh control register at NABMBAR
        ich_OUTB (0, CTL_BASE, ICH_REG_MC_CR);                              // 2Bh control register at NABMBAR

        // reset channels
        ich_OUTB (ICH_RESETREGS, CTL_BASE, ICH_REG_PI_CR);                  // 0Bh control register at NABMBAR
        ich_OUTB (ICH_RESETREGS, CTL_BASE, ICH_REG_PO_CR);                  // 1Bh control register at NABMBAR
        ich_OUTB (ICH_RESETREGS, CTL_BASE, ICH_REG_MC_CR);                  // 2Bh control register at NABMBAR

        // enable VRA (if supported) and set default dacrate to 44100Hz
        if (ac97_read_codec (AC97_EXTENDED_ID) & AC97_EA_VRA) {
                ac97_write_codec (AC97_EXTENDED_STATUS, AC97_EA_VRA | ac97_read_codec(AC97_EXTENDED_STATUS));
                ac97_write_codec (AC97_PCM_FRONT_DAC_RATE, 44100);
                audio_pci.ac97_vra_supported = TRUE;
        } else audio_pci.ac97_vra_supported = FALSE;

        // disable DRA (if supported)
        if (ac97_read_codec (AC97_EXTENDED_ID) & AC97_EA_DRA) {
                wdata = ac97_read_codec (AC97_EXTENDED_STATUS);
                wdata &= ~(AC97_EA_DRA);
                ac97_write_codec (AC97_EXTENDED_STATUS, wdata);
        }

        return 1;
}

unsigned int ich_INL (int base, unsigned int a)
{
    if (audio_pci.mem_mode == 1) {
        if (base == CTL_BASE) return *(unsigned int *) (audio_pci.base3 + (a));   // bus master base memory
        else return *(unsigned int *) (audio_pci.base2 + (a));                    // audio mixer base memory
    } else {
        if (base == CTL_BASE) return inpd (audio_pci.base1 + a);                  // bus master base IO
        else return inpd (audio_pci.base0 + a);                                   // audio mixer base IO
    }
}

unsigned short ich_INW (int base, unsigned short a)
{
    if (audio_pci.mem_mode == 1) {
        if (base == CTL_BASE) return *(unsigned short *) (audio_pci.base3 + (a)); // bus master base memory
        else return *(unsigned short *) (audio_pci.base2 + (a));                  // audio mixer base memory
    } else {
        if (base == CTL_BASE) return inpw (audio_pci.base1 + a);                  // bus master base IO
        else return inpw (audio_pci.base0 + a);                                   // audio mixer base IO
    }
}

unsigned char ich_INB (int base, unsigned char a)
{
    if (audio_pci.mem_mode == 1) {
        if (base == CTL_BASE) return *(unsigned char *) (audio_pci.base3 + (a));  // bus master base memory
        else return *(unsigned char *) (audio_pci.base2 + (a));                   // audio mixer base memory
    } else {
        if (base == CTL_BASE) return inp (audio_pci.base1 + a);                   // bus master base IO
        else return inp (audio_pci.base0 + a);                                    // audio mixer base IO
    }
}

void ich_OUTL (unsigned int d, int base, unsigned int a)
{
    if (audio_pci.mem_mode == 1) {
        if (base == CTL_BASE) *(unsigned int *) (audio_pci.base3 + (a)) = d;      // bus master base memory
        else *(unsigned int *) (audio_pci.base2 + (a)) = d;                       // audio mixer base memory
    } else {
        if (base == CTL_BASE) outpd (audio_pci.base1 + a, d);                     // bus master base IO
        else outpd (audio_pci.base0 + a, d);                                      // audio mixer base IO
    }
}

void ich_OUTW (unsigned short d, int base, unsigned short a)
{
    if (audio_pci.mem_mode == 1) {
        if (base == CTL_BASE) *(unsigned short *) (audio_pci.base3 + (a)) = d;    // bus master base memory
        else *(unsigned short *) (audio_pci.base2 + (a)) = d;                     // audio mixer base memory
    } else {
        if (base == CTL_BASE) outpw (audio_pci.base1 + a, d);                     // bus master base IO
        else outpw (audio_pci.base0 + a, d);                                      // audio mixer base IO
    }
}

void ich_OUTB (unsigned char d, int base, unsigned char a)
{
    if (audio_pci.mem_mode == 1) {
        if (base == CTL_BASE) *(unsigned char *) (audio_pci.base3 + (a)) = d;     // bus master base memory
        else *(unsigned char *) (audio_pci.base2 + (a)) = d;                      // audio mixer base memory
    } else {
        if (base == CTL_BASE) outp (audio_pci.base1 + a, d);                      // bus master base IO
        else outp (audio_pci.base0 + a, d);                                       // audio mixer base IO
    }
}





/*****************************************************************************
        HDA audio mixer registers access helper functions

             HDA code falls under the GNU/GPL license

        This code is based on info from ALSA sources and other
        GNU/GLP sources like Open Sound System. Although it differs
        much from those projects I decided to put it on the same
        license as the above mentioned sources - the GNU/GPL license.

******************************************************************************/
static unsigned int hda_calc_stream_format (int mixrate)
{
    unsigned int value = 0;

    // we only support 7 rates for HDA - 48000Hz included
    if (mixrate < 11025) value = BIT8 + BIT10;                // 8000 (48000 / 6)
    else if (mixrate < 16000) value = BIT14 + BIT8 + BIT9;    // 11025 (44000 / 4)
    else if (mixrate < 22050) value = BIT9;                   // 16000 (48000 / 3)
    else if (mixrate < 32000) value = BIT14 + BIT8;           // 22050 (44000 / 2)
    else if (mixrate < 44100) value = BIT11 + BIT9;           // 32000 (96000 / 3)
    else if (mixrate < 48000) value = BIT14;                  // 44100
    else value = 0;                                           // 48000

    // 8bit not supported - we force 16bits PCM anyway
    switch (audio_pci.dacout_num_bits) {
        case 16:
            value |= BIT4;
            break;
        case 20:
            value |= BIT5;
            break;
        case 24:
            value |= (BIT4 + BIT5);
            break;
        case 32:
            value |= BIT6;
            break;
        default:
            value |= BIT4;
            break;
    }

    value += (audio_pci.dacout_num_channels - 1);

    return value;
}

/*************************************
  * HDA low level register access
  *************************************/
// AZBAR is base0 only
static unsigned int hda_INL (unsigned int a)
{
    if (audio_pci.hda_mode == 1) return *(unsigned int *) (audio_pci.base0 + (a));
    else return 0;
}

static unsigned short hda_INW (unsigned int a)
{
    if (audio_pci.hda_mode == 1) return *(unsigned short *) (audio_pci.base0 + (a));
    else return 0;
}

static unsigned char hda_INB (unsigned int a)
{
    if (audio_pci.hda_mode == 1) return *(unsigned char *) (audio_pci.base0 + (a));
    else return 0;
}

static void hda_OUTL (unsigned int a, unsigned int d)
{
    if (audio_pci.hda_mode == 1) *(unsigned int *) (audio_pci.base0 + (a)) = d;
}

static void hda_OUTW (unsigned int a, unsigned short d)
{
    if (audio_pci.hda_mode == 1) *(unsigned short *) (audio_pci.base0 + (a)) = d;
}

static void hda_OUTB (unsigned int a, unsigned char d)
{
    if (audio_pci.hda_mode == 1) *(unsigned char *) (audio_pci.base0 + (a)) = d;
}

/*********************************************************************************************
  * HDA Basic read/write to codecs - Single immediate command instead of CORB/RIRB buffers for simplicity
  *********************************************************************************************/
static BOOL hda_single_send_cmd (unsigned short nid, unsigned int direct, unsigned int verb, unsigned int param)
{
    int timeout = 1000;
    unsigned int value;

    value = (unsigned int) (audio_pci.codec_index & 0x0f) << 28;   // CaD - codec address
    value |= (unsigned int) direct << 27;   // 0 = direct NID reference, 1 = indirect NID reference
    value |= (unsigned int) nid << 20;   // node ID
    value |= verb << 8;   // verb
    value |= param;   // parameter

    while (timeout--) {
        if (!(hda_INW (HDA_IRS) & IRS_BUSY)) {
            hda_OUTW (HDA_IRS, hda_INW (HDA_IRS) | IRS_VALID);
            hda_OUTL (HDA_IC, value);
            hda_OUTW (HDA_IRS, hda_INW (HDA_IRS) | IRS_BUSY);
            return TRUE;
        }
    }
    return FALSE;
}

static unsigned int hda_single_get_response (void)
{
    int timeout = 1000;

    while (timeout--) {
        if (hda_INW (HDA_IRS) & IRS_VALID) return hda_INL (HDA_IR);
    }
    return 0;
}

// standard codec read
static unsigned int hda_codec_read (unsigned short nid, unsigned int direct, unsigned int verb, unsigned int param)
{
    if (hda_codec_write (nid, direct, verb, param) == TRUE) return hda_single_get_response();
    return 0;
}

// simplified codec read parameters - direct NID reference
static unsigned int hda_param_read (unsigned short nid ,unsigned int param)
{
    return hda_codec_read (nid, 0, AC_VERB_PARAMETERS, param);
}

// standard codec write
static int hda_codec_write (unsigned short nid, unsigned int direct, unsigned int verb, unsigned int param)
{
    return hda_single_send_cmd (nid, direct, verb, param);
}

/*************************************************************
  * Main HDA functions
  *************************************************************/
// stop and close all output azalia hd audio operations if currently playing
static void hda_codec_stop (void)
{
    int timeout = 300;

    // hda codec stop - stop DMA engine for output and disable interrupts (Interrupt on Completion, FIFO Error Interrupt, Descriptor Error Interrupt)
    hda_OUTB (HDA_SDO0CTL, hda_INB (HDA_SDO0CTL) & ~(SD_CTL_DMA_START | SD_INT_MASK));

    // Software must read a 0 from the DMA run bit before modifying related control registers or restarting the DMA engine
    while (timeout--) {
        if (!(hda_INB (HDA_SDO0CTL) & SD_CTL_DMA_START)) break;
        delay(1);
    }

    // hda codec close - reset stream 0 data since we're not using other output streams anyway
    hda_OUTL (HDA_DPLBASE, 0);     // DMA Position Lower Base Address
    hda_OUTL (HDA_DPUBASE, 0);     // DMA Position Upper Base Address
    hda_OUTL (HDA_SDO0BDLPL, 0);   // SD0 outpuy BDL - lower
    hda_OUTL (HDA_SDO0BDLPU, 0);   // SD0 output BDL - upper
    hda_OUTL (HDA_SDO0CTL, 0);     // SD0 output stream descriptor control
    
    // zero out HDA start values in ASM module (current pos and CIV)
    hda_civ = 0;
    hda_lpib = 0;
}

// start the DMA engine with currently allocated buffers (only for playback) - obsolete - start is done in ASM module
static void hda_codec_start (void)
{
     // start SD0 output DMA engine
     hda_OUTB (HDA_SDO0CTL, hda_INB (HDA_SDO0CTL) | SD_CTL_DMA_START);
}

// read Link Position in Current Buffer - obsolete - used for testing purposes
extern long hda_getbufpos (void)
{
     unsigned long bufpos;

     // temporary
     audio_pci.pcmout_dmasize = judas_bufferlength;
     audio_pci.pcmout_bdl_entries = 32;
     audio_pci.pcmout_bdl_size = (4 * sizeof(DWORD)) * audio_pci.pcmout_bdl_entries;

     // read Link Position in Current Buffer
     bufpos = hda_INL (HDA_SDO0LPIB);

     #ifdef HDA_DEBUG
     logmessage ("ctl:%8.8X sts:%8.8X cbl:%d ds:%d ps:%d pn:%d bufpos:%5d",
         hda_INB (HDA_SDO0CTL), hda_INL (HDA_SDO0STS), hda_INL (HDA_SDO0CBL), audio_pci.pcmout_dmasize, audio_pci.pcmout_bdl_size, audio_pci.pcmout_bdl_entries, bufpos);
     #endif
         
     if (bufpos < audio_pci.pcmout_dmasize) audio_pci.pcmout_dma_lastgoodpos = bufpos;

     return audio_pci.pcmout_dma_lastgoodpos;
}

// enable codec, unmute stuff, set output to desired rate
// in = desired sample rate
// out = true or false
static BOOL hda_reset (void)
{
    DWORD flags = 0;
    int i = 0;

    //  stop the codec if currently playing
    hda_codec_stop ();

    // disable global controller interrupts CIE and GIE if enabled
    hda_OUTL (HDA_INTCTL, hda_INL (HDA_INTCTL) & ~(HDA_INT_CTRL_EN | HDA_INT_GLOBAL_EN));

    // ---------------------
    // reset controller
    // ---------------------
    flags = hda_INL (HDA_GCTL);                                         // AZBAR + HDA_GCTL
    flags &= ~CRST;
    hda_OUTL (HDA_GCTL, flags);                                         // reset bit0 of GCTL

    i = 50;
    while (hda_INL (HDA_GCTL) && --i) delay (1);                        // 1 ms delay
    delay(500);                                                         // must read 0 to verify the controller is in reset

    // bring controller out of reset
    flags = hda_INL (HDA_GCTL);                                         // AZBAR + HDA_GCTL
    flags |= CRST;
    hda_OUTL (HDA_GCTL, flags);                                         // set bit0 of GCTL

    i = 50;
    while (!hda_INL (HDA_GCTL) && --i) delay (1);                       // 1 ms delay
    delay(500);                                                         // must read 1 before accessing controller registers

    if (!hda_INL (HDA_GCTL)) return FALSE;                              // controller not ready - exit with error

    // disable unsolicited responses
    hda_OUTL (HDA_GCTL, (hda_INL (HDA_GCTL) & (~UREN)) );
    delay(1);

    // detect codecs
    if (!audio_pci.codec_mask) {
        audio_pci.codec_mask = hda_INW (HDA_STATESTS);
    }

    // ------------------------------------
    // hardware init the controller
    //-------------------------------------
    // clear interrupt status for stream 0  - set bits in Buffer Completion Interrupt, FIFO Error, Descriptor Error
    hda_OUTB (HDA_SDO0STS, SD_INT_MASK);
    // enable interrupts for stream 0 - I do not use AC97/HDA interrupts in INCHINIT or JUDAS so skip this part of initialization
    // hda_OUTB ( HDA_SDO0CTL, SD_INT_MASK);
    // clear STATESTS - Flag bits that indicate which SDI signal(s) received a state change event. The bits are cleared by writing 1’s to them.
    hda_OUTW (HDA_STATESTS, STATESTS_INT_MASK);
    // clear RIRB status - Response Interrupt, Response Overrun Interrupt Status - not used in ICHINIT and JUDAS either
    hda_OUTB (HDA_RIRBSTS, RIRB_INT_MASK);
    // clear global interrupt status
    hda_OUTL (HDA_INTSTS, HDA_INT_CTRL_EN | HDA_INT_ALL_STREAM);
    // enable global controller interrupts CIE and GIE -  I do not use AC97/HDA interrupts in INCHINIT or JUDAS so skip this part of initialization
    // hda_OUTL ( HDA_INTCTL, hda_INL ( HDA_INTCTL) | HDA_INT_CTRL_EN | HDA_INT_GLOBAL_EN);

    // reset the position buffer (program the position buffer when starting playback)
    hda_OUTL (HDA_DPLBASE, 0);
    hda_OUTL (HDA_DPUBASE, 0);

    // hardware init mixer for first detected codec (bits 0, 1, 2, etc)
    for (i = 0; i < HDA_MAX_CODECS; i++) {
        if (audio_pci.codec_mask & (1 << i)) {
            audio_pci.codec_index = i;
            if (hda_mixer_init ()) break;
        }
    }
    return TRUE;
}

/**************************
 HDA mixer init functions
***************************/
// initialize the HDA mixer
static unsigned int hda_mixer_init (void)
{
    unsigned int i;
    unsigned short nid;
    unsigned long hda_codec_vender_id;

    // get vender and device IDs then save it to our audio_pci structure
    hda_codec_vender_id = hda_param_read (AC_NODE_ROOT, AC_PAR_VENDOR_ID);
    if (hda_codec_vender_id == 0) hda_codec_vender_id = hda_param_read (AC_NODE_ROOT, AC_PAR_VENDOR_ID);
    audio_pci.codec_id1 = (WORD) (hda_codec_vender_id >> 16);      // HDA codec vender ID
    audio_pci.codec_id2 = (WORD) (hda_codec_vender_id & 0xffff);   // HDA codec device ID

    // search for audio function group root node, exit if not found
    hda_search_audio_node ();
    if (!audio_pci.afg_root_nodenum) goto error_exit_mixinit;

    // check number of nodes for AFG, nid is the starting AFG node - can not be 0
    // hda_get_sub_nodes returns -1 if more then max connections defined in header file are present
    audio_pci.afg_num_nodes = hda_get_sub_nodes (audio_pci.afg_root_nodenum, &nid);
    if ((audio_pci.afg_num_nodes <= 0) || !nid) goto error_exit_mixinit;

    // read default capabilities from main audio function group node
    audio_pci.def_amp_out_caps = hda_param_read (audio_pci.afg_root_nodenum, AC_PAR_AMP_OUT_CAP);
    audio_pci.def_amp_in_caps = hda_param_read (audio_pci.afg_root_nodenum, AC_PAR_AMP_IN_CAP);
    
    // allocate memory for all AFG nodes
    if (audio_pci.afg_nodes) locked_free (audio_pci.afg_nodes);
    audio_pci.afg_nodes = (struct hda_node *) locked_malloc (audio_pci.afg_num_nodes * sizeof (struct hda_node));
    if (!audio_pci.afg_nodes) goto error_exit_mixinit;

    // add all AFG nodes to our structure with hda_add_new_node function
    for (i = 0; i < audio_pci.afg_num_nodes; i++, nid++) hda_add_new_node (&audio_pci.afg_nodes[i], nid);

    // determine output path
    if (!hda_parse_output()) goto error_exit_mixinit;

    // determine the supported audio format from dac node - first output node
    if (audio_pci.dac_node[0]){
        audio_pci.supported_formats = audio_pci.dac_node[0]->supported_formats;
        if (!audio_pci.supported_formats) audio_pci.supported_formats = 0xffffffff; // then enter fixed max freq
        // fixed - we only support 48khz with 16bit PCM stereo max anyway
        audio_pci.supported_max_freq = 48000;
        audio_pci.supported_max_bits = 16;
        // audio_pci.supported_max_freq = hda_get_max_freq ();
        // audio_pci.supported_max_bits = hda_get_max_bits ();
    }
    return 1;

    error_exit_mixinit:
    // free nodes memory on error
    if (audio_pci.afg_nodes){
        locked_free (audio_pci.afg_nodes);
        audio_pci.afg_nodes = NULL;
    }
    return 0;
}

/*********************************
HDA functions volume update/unmute
**********************************/
// set the volume or mute the requested amplifier
static void hda_set_vol_mute (unsigned short nid, int ch, int direction, int index, int val)
{
    unsigned int param;

    param  = (ch) ? AC_AMP_SET_RIGHT : AC_AMP_SET_LEFT;
    param |= (direction == HDA_OUTPUT) ? AC_AMP_SET_OUTPUT : AC_AMP_SET_INPUT;
    param |= index << 8;   // index is bits 8 - 11 for set payload
    param |= val;
    hda_codec_write (nid, 0, AC_VERB_SET_AMP_GAIN_MUTE, param);
}

// get the volume or mute status of the requested amplifier
static unsigned int hda_get_vol_mute (unsigned short nid, int ch, int direction, int index)
{
    unsigned int val, param;

    param  = (ch) ? AC_AMP_GET_RIGHT : AC_AMP_GET_LEFT;
    param |= (direction == HDA_OUTPUT) ? AC_AMP_GET_OUTPUT : AC_AMP_GET_INPUT;
    param |= index;   // index is bits 0 - 3 for get payload
    val = hda_codec_read (nid, 0, AC_VERB_GET_AMP_GAIN_MUTE, param);
    return (val & 0xff);   // bits 8 - 31 are ignored as they should be 0
}

// update the requested amplifier
static int hda_codec_amp_update (unsigned short nid, int ch, int direction, int idx, int mask, int val)
{
    val &= mask;
    val |= hda_get_vol_mute (nid, ch, direction, idx) & ~mask;
    hda_set_vol_mute (nid, ch, direction, idx, val);
    return 1;
}

// unmute output and set max volume for the output amplifier
static void hda_unmute_output (struct hda_node *node)
{
    unsigned int val;
    val = (node->amp_out_caps & AC_AMPCAP_NUM_STEPS) >> AC_AMPCAP_NUM_STEPS_SHIFT;
    hda_codec_amp_update (node->nid, 0, HDA_OUTPUT, 0, 0xff, val);
    hda_codec_amp_update (node->nid, 1, HDA_OUTPUT, 0, 0xff, val);
}

// unmute input and set max volume for the input amplifier
static void hda_unmute_input (struct hda_node *node, unsigned int index)
{
    unsigned int val;
    val = (node->amp_in_caps & AC_AMPCAP_NUM_STEPS) >> AC_AMPCAP_NUM_STEPS_SHIFT;
    hda_codec_amp_update (node->nid, 0, HDA_OUTPUT, 0, 0xff, val);
    hda_codec_amp_update (node->nid, 1, HDA_OUTPUT, 0, 0xff, val);
}

/**********************************
HDA  Nodes identification functions
***********************************/
// Subordinate Node Count as in Intel specs
static unsigned int hda_get_sub_nodes (unsigned short nid, unsigned short *start_node)
{
    unsigned int param;   // the parameter value - response is 32bit

    param = hda_param_read (nid, AC_PAR_NODE_COUNT);
    *start_node = (param >> 16) & 0xff;   // starting node number bits 16 - 23
    return (param & 0xff);                // total number of nodes bits 0 - 7
}

// Search audio node from Function Group Types
static void hda_search_audio_node (void)
{
    int i, total_nodes;
    unsigned short nid;   // starting function group node

    // get total function group nodes and save starting function group node
    total_nodes = hda_get_sub_nodes (AC_NODE_ROOT, &nid);

    // bits 0 - 7 of response from parameter read specify the node type
    // search and save audio function group node id
    for (i = 0; i < total_nodes; i++, nid++) {
        if ((hda_param_read (nid, AC_PAR_FUNCTION_TYPE) & 0xff) == AC_GRP_AUDIO_FUNCTION) {
            audio_pci.afg_root_nodenum = nid;
            break;
        }
    }
}

// retrieve connections for the specified node
// 0 if none
// -1 if more than max_conns
// positive on success with array of conn_list filled
static int hda_get_connections (unsigned short nid, unsigned short *conn_list, int max_conns)
{
    unsigned int param;
    unsigned int shift, num_elements, mask;
    int conn_length, conns, i;
    unsigned short prev_nid;
    unsigned short val, n;

    // get Connection List Length for nid
    param = hda_param_read (nid, AC_PAR_CONNLIST_LEN);

    // if long form connections list - each entry has 16 bits (2 entries)
    if (param & BIT7) {
        shift = 16;
        num_elements = 2;

        // mask is 0x7f for short and 0x7fff for long entries
        // the highest bit (7 or 15) specifies whether connection is an independent NID or a range of NIDs
        mask = 0x7fff;

    // if short form connections list - each entry has 8 bits (4 entries)
    } else {
        shift = 8;
        num_elements = 4;

        // mask is 0x7f for short and 0x7fff for long entries
        // the highest bit (7 or 15) specifies whether connection is an independent NID or a range of NIDs
        mask = 0x7f;
    }

    // Connection List Length bits 0 - 6 (exit if no connections)
    conn_length = param & 0x7f;
    if (!conn_length) return 0;

    // if only 1 connection - get it and exit
    if (conn_length == 1) {
        param = hda_codec_read (nid, 0, AC_VERB_GET_CONNECT_LIST, 0);
        conn_list[0] = param & mask;
        return 1;
    }

    // if more connections
    conns = 0;
    prev_nid = 0;
    for (i = 0; i < conn_length; i++) {

        // get 4 entries for short from and 2 entries for long form connections list
        if (i % num_elements == 0) param = hda_codec_read (nid, 0, AC_VERB_GET_CONNECT_LIST, i);

        // current connection
        val = param & mask;

        // if range of NIDs (highest bit set - 7 or 15 for long entries)
        if (param & (mask + 1)) {
            if (!prev_nid || prev_nid >= val) continue;   // ignore for first entry or if previous NID was equal/higher than current
            for (n = prev_nid + 1; n <= val; n++) {       // start at (prev_nid + 1) because prev_nid is already saved as single NID
                if (conns >= max_conns) return -1;
                conn_list[conns++] = n;
            }

        // if single NID (highest bit clear)
        } else {
            if (conns >= max_conns) return -1;
            conn_list[conns++] = val;
        }

        // save this NID as previous NID in case we have a range of NIDs in next entry
        prev_nid = val;

        // pass on to next connection entry
        param >>= shift;
    }
    return conns;
}

// add a new node to our audio structure
static int hda_add_new_node (struct hda_node *node, unsigned short nid)
{
    int nconns;

    node->nid = nid;
    nconns = hda_get_connections (nid, &node->conn_list[0], HDA_MAX_CONNECTIONS);

    // when we have connections
    if (nconns >= 0) {
        node->nconns = nconns;

        // get Audio Widget Capabilities
        node->wid_caps = hda_param_read (nid, AC_PAR_AUDIO_WIDGET_CAP);

        // bits 20 - 23 define the node type
        // AC_WID_AUD_OUT, AC_WID_AUD_IN, AC_WID_AUD_MIX, AC_WID_AUD_SEL,  AC_WID_PIN,  AC_WID_POWER, AC_WID_VOL_KNB, AC_WID_BEEP, AC_WID_VENDOR
        node->type = (node->wid_caps & (BIT20 | BIT21 | BIT22 | BIT23)) >> 20;

        // if pin complex
        if (node->type == AC_WID_PIN){
            // get Pin Capabilities (output capable, input capable, vref, etc)
            node->pin_caps = hda_param_read (node->nid, AC_PAR_PIN_CAP);

            // get Pin Widget Control status (out enable, in enable, vref enable, etc)
            node->pin_ctl = hda_codec_read (node->nid, 0, AC_VERB_GET_PIN_WIDGET_CONTROL, 0);

            // get default config
            node->def_config = hda_codec_read (node->nid, 0, AC_VERB_GET_CONFIG_DEFAULT, 0);
        }

        // if out amplifier present
        if (node->wid_caps & AC_WCAP_OUT_AMP) {
            // If Amp Param Override is a 1, the widget contains its own amplifier parameters. If this bit is a 0,
            // then the Audio Function node must contain default amplifier parameters, and they should be used
            // to define all amplifier parameters (both input and output) in this widget.

            // The “Amplifier Properties” parameters return the parameters for the input or the output amplifier
            // on a node. In the case of a Pin Widget, the terms input and output are relative to the codec itself;
            // for all other widgets, these terms are relative to the node. The amplifier capabilities are indicated by
            // the step size of the amplifier, the number of steps, the offset of the range with respect to 0 dB, and
            // whether the amplifier supports mute.
            if (node->wid_caps & AC_WCAP_AMP_OVRD) node->amp_out_caps = hda_param_read (node->nid, AC_PAR_AMP_OUT_CAP);
            if (!node->amp_out_caps) node->amp_out_caps = audio_pci.def_amp_out_caps;
        }

        // if in amplifier present
        if (node->wid_caps & AC_WCAP_IN_AMP) {
            // If Amp Param Override is a 1, the widget contains its own amplifier parameters. If this bit is a 0,
            // then the Audio Function node must contain default amplifier parameters, and they should be used
            // to define all amplifier parameters (both input and output) in this widget.

            // The “Amplifier Properties” parameters return the parameters for the input or the output amplifier
            // on a node. In the case of a Pin Widget, the terms input and output are relative to the codec itself;
            // for all other widgets, these terms are relative to the node. The amplifier capabilities are indicated by
            // the step size of the amplifier, the number of steps, the offset of the range with respect to 0 dB, and
            // whether the amplifier supports mute.
            if (node->wid_caps & AC_WCAP_AMP_OVRD) node->amp_in_caps = hda_param_read (node->nid, AC_PAR_AMP_IN_CAP);
            if (!node->amp_in_caps) node->amp_in_caps = audio_pci.def_amp_in_caps;
        }

        // read supported PCM formats - return 32bit dword with format data as in HDA specification
        // for AFG, AI converter and AO converter group
        node->supported_formats = hda_param_read (node->nid, AC_PAR_PCM);
    }

    return nconns;
}

// get the node data pointed by NID from our device structure
static struct hda_node *hda_get_node (unsigned short nid)
{
    struct hda_node *node = audio_pci.afg_nodes;
    unsigned int i;

    for (i = 0; i < audio_pci.afg_num_nodes; i++, node++)
        if (node->nid == nid) return node;

    return NULL;
}

/********************************************************************************************
  HDA output nodes - search path
  output path nodes parser written basing on ALSA project with major modifications for JUDAS support
  some more improvements will be made in the near future
 *********************************************************************************************/
// select the input connection of the given node
static int hda_select_input_connection (struct hda_node *node, unsigned int index)
{
    return hda_codec_write (node->nid, 0, AC_VERB_SET_CONNECT_SEL, index);
}

// clear checked nodes flags for different jack types
static void hda_clear_check_flags (void)
{
    struct hda_node *node = audio_pci.afg_nodes;
    unsigned int i;

    for (i = 0; i < audio_pci.afg_num_nodes; i++, node++) node->checked = 0;
}

// Parse output path until we reach an audio output widget.
// returns 0 if not found, 1 if found
static int hda_parse_output_path (struct hda_node *node, int dac_idx)
{
    int i;
    struct hda_node *child;

    // exit if node already checked
    if (node->checked) return 0;

    // mark this node as checked
    node->checked = 1;
    // if we have an Audio Out widget type
    if (node->type == AC_WID_AUD_OUT) {
        // skip DIGITAL OUT node
        if (node->wid_caps & AC_WCAP_DIGITAL) return 0;
        // DAC node is assigned already, just unmute and connect
        if (audio_pci.dac_node[dac_idx]) return node == audio_pci.dac_node[dac_idx];
        // assign DAC node
        audio_pci.dac_node[dac_idx] = node;
        // if out amplifier present save node to pcm_vols
		if((node->wid_caps & AC_WCAP_OUT_AMP) && (audio_pci.pcm_num_vols < HDA_MAX_PCM_VOLS)) {
            audio_pci.pcm_vols[audio_pci.pcm_num_vols].node = node;
            audio_pci.pcm_vols[audio_pci.pcm_num_vols].index = 0;
            audio_pci.pcm_num_vols++;
        }
        // found
        return 1;
    }

    // also parse child nodes for the main node using the same function
    for (i = 0; i < node->nconns; i++) {
        child = hda_get_node (node->conn_list[i]);
        if (!child) continue;
        // child node found, parse it's output
        if (hda_parse_output_path (child, dac_idx)) {
            // found - select the path, unmute both input and output
            if (node->nconns > 1) hda_select_input_connection (node, i);
            hda_unmute_input (node, i);
            hda_unmute_output (node);
            if (audio_pci.dac_node[dac_idx] && (audio_pci.pcm_num_vols < HDA_MAX_PCM_VOLS) && !(audio_pci.dac_node[dac_idx]->wid_caps & AC_WCAP_OUT_AMP)) {
                if((node->wid_caps & AC_WCAP_IN_AMP) || (node->wid_caps & AC_WCAP_OUT_AMP)) {
                    int n = audio_pci.pcm_num_vols;
                    audio_pci.pcm_vols[n].node = node;
                    audio_pci.pcm_vols[n].index = i;
                    audio_pci.pcm_num_vols++;
                }
            }
            return 1;
        }
    }
    return 0;
}

// Look for the output PIN widget with the given jack type
// and parse the output path to that PIN.
// Returns the PIN node when the path to DAC is established.
// -1 to parse first output
static struct hda_node *hda_parse_output_jack (int jack_type)
{
    struct hda_node *node = audio_pci.afg_nodes;
    int err, i;

    // we test each node from our afg_nodes structure which contains all nodes connected to root AFG node
    for (i = 0 ; i < audio_pci.afg_num_nodes; i++, node++) {
        // pin widget node ?
        if (node->type != AC_WID_PIN) continue;
        // output capable ?
        if (!(node->pin_caps & AC_PINCAP_OUT)) continue;
        // unconnected ?
        if (defconfig_port_conn (node) == AC_JACK_PORT_NONE) continue;
        // parse defined outputs
		if (jack_type >= 0) {
            if (jack_type != defconfig_type (node)) continue;
            // if SPDIF skip
            if (node->wid_caps & AC_WCAP_DIGITAL) continue;
        // else parse 1st output
		} else {
            // output as default ?
            if (!(node->pin_ctl & AC_PINCTL_OUT_EN)) continue;
        }
        // clear checked nodes flags for different jack types
        hda_clear_check_flags ();
        
		// try primary dac
		err = hda_parse_output_path (node, 0);
        
		// try secondary dac
		if (!err && audio_pci.out_pin_node[0]) {
            err = hda_parse_output_path (node, 1);
        }
        if (err) {
            // unmute the PIN output
            hda_unmute_output (node);
            // set PIN-Out enable
            hda_codec_write (node->nid, 0, AC_VERB_SET_PIN_WIDGET_CONTROL, AC_PINCTL_OUT_EN |
               ((node->pin_caps & AC_PINCAP_HP_DRV) ? AC_PINCTL_HP_EN : 0));
            return node;
        }
    }
    return NULL;
}

static int hda_parse_output (void)
{
    struct hda_node *node;

    // Look for the output PIN widget. First, look for the line-out pin
    node = hda_parse_output_jack (AC_JACK_LINE_OUT);

    // Found, remember the PIN node
    if (node) audio_pci.out_pin_node[0] = node;
    // If no line-out is found, try speaker out
    else {
        node = hda_parse_output_jack (AC_JACK_SPEAKER);
        if (node) audio_pci.out_pin_node[0] = node;
    }

    // Look for the HP-out pin
    node = hda_parse_output_jack (AC_JACK_HP_OUT);
    // If we have no line-out/speaker pin then select hp-out as default output
    // Otherwaise select hp-out as secondary output
    if (node) {
        if (!audio_pci.out_pin_node[0]) audio_pci.out_pin_node[0] = node;
        else audio_pci.out_pin_node[1] = node;
    }

    // When no line-out or HP pins found choose the first output pin
    if (!audio_pci.out_pin_node[0]) {
        audio_pci.out_pin_node[0] = hda_parse_output_jack (-1); // parse 1st output
        if (!audio_pci.out_pin_node[0]) {
            #ifdef HDA_DEBUG
            logmessage ("Error : no proper output path found\n");
            #endif
            return 0;
        }
    }

    return 1;
}


/*****************************************************************************

 END OF CODE FALLING UNDER THE GNU/GPL LICENSE

*****************************************************************************/




/*****************************************************************************
        General stuff
******************************************************************************/

int initmixer(void)
{
        int v, s, sv;

        /*
         * If this is the first time we are initializing, we must allocate the
         * lookup tables, clipbuffer & zladdbuffer and lock them as well.
         * Volume table needs to be calculated only once.
         *
         */
        if (mixer_firsttime)
        {
                int *volptr;
                CHANNEL *chptr;
                int buffersize;

                judas_cliptable = locked_malloc(65536 * sizeof(short));
                if (!judas_cliptable)
                {
                        return 0;
                }

                if (judas_device == DEV_FILE) {
                  buffersize = (int) filewriterbuffersize * 8;
                } else if (judas_device == DEV_AC97 || judas_device == DEV_HDA) {
                  buffersize = 48000 / PER_SECOND * 8;
                } else buffersize = 44100 / PER_SECOND * 8;
                judas_clipbuffer = locked_malloc(buffersize);
                if (!judas_clipbuffer)
                {
                        locked_free(judas_cliptable);
                        return 0;
                }
                judas_zladdbuffer = locked_malloc(buffersize);
                if (!judas_zladdbuffer)
                {
                        return 0;
                }
                judas_volumetable = locked_malloc(256 * 256 * sizeof(int) + 1024);
                if (!judas_volumetable)
                {
                        locked_free(judas_cliptable);
                        locked_free(judas_clipbuffer);
                        locked_free(judas_zladdbuffer);
                        return 0;
                }
                /*
                                            * Adjust the volumetable to begin on a 1024 byte boundary;
                                            * the mixing routines need this!
                                           */
                judas_volumetable = (int *)((((unsigned)judas_volumetable) + 1023) & 0xfffffc00);
                volptr = &judas_volumetable[0];
                /*
                                            * Note: although there is an optimized routine for zero volume,
                                            * we need the zero volume table because in stereo mixing the
                                            * other channel's volume could be zero.
                                            */
                for (v = 0; v < 256; v++)
                {
                        for (s = 0; s < 256; s++)
                        {
                                sv = s;
                                if (sv > 127) sv -= 256;
                                sv *= v;
                                sv >>= (16 - SIGNIFICANT_BITS_16);
                                *volptr = sv;
                                volptr++;
                        }
                }
                /*
                                             * The mixing routines need the address shifted, and since we
                                             * don't need the pointer anymore...
                                             */
                judas_volumetable = (int *)((unsigned)judas_volumetable >> 2);

                chptr = &judas_channel[0];

                /*
                                            * Init all channels (no sound played, no sample, mastervolume 64)
                                            *
                                            */
                for (s = CHANNELS; s > 0; s--)
                {
                        chptr->voicemode = VM_OFF;
                        chptr->sample = NULL;
                        chptr->mastervol = 64;
                        chptr++;
                }
                mixer_firsttime = 0;
        }

        if (judas_mixmode & SIXTEENBIT)
        {
                short *clipptr = &judas_cliptable[0];

                for (s = 0; s < 65536; s++)
                {
                        sv = s;
                        if (sv > 32767) sv -= 65536;
                        sv <<= (16 - SIGNIFICANT_BITS_16);
                        if (sv < -32768) sv = -32768;
                        if (sv > 32767) sv = 32767;
                        *clipptr = sv;
                        clipptr++;
                }
        }
        else
        {
                unsigned char *clipptr = (unsigned char *)&judas_cliptable[0];

                for (s = 0; s < 65536; s++)
                {
                        int sv = s;
                        if (sv > 32767) sv = s - 65536;
                        sv <<= (16 - SIGNIFICANT_BITS_16);
                        if (sv < -32768) sv = -32768;
                        if (sv > 32767) sv = 32767;
                        *clipptr = (sv >> 8) + 128;
                        clipptr++;
                }
        }
        return 1;
}

static int judas_lock(void)
{
        if (judas_locked) return 1;
        if (!judas_memlock(&judas_code_lock_start, (int)&judas_code_lock_end - (int)&judas_code_lock_start)) return 0;
        if (!judas_memlock(&judas_device, sizeof judas_device)) return 0;
        if (!judas_memlock(&judas_mixroutine, sizeof judas_mixroutine)) return 0;
        if (!judas_memlock(&judas_mixersys, sizeof judas_mixersys)) return 0;
        if (!judas_memlock(&judas_initialized, sizeof judas_initialized)) return 0;
        if (!judas_memlock(&judas_ds, sizeof judas_ds)) return 0;
        if (!judas_memlock(&judas_irq, sizeof judas_irq)) return 0;
        if (!judas_memlock(&judas_int, sizeof judas_int)) return 0;
        if (!judas_memlock(&judas_dma, sizeof judas_dma)) return 0;
        if (!judas_memlock(&judas_port, sizeof judas_port)) return 0;
        if (!judas_memlock(&judas_irqcount, sizeof judas_irqcount)) return 0;
        if (!judas_memlock(&dsp_version, sizeof dsp_version)) return 0;
        if (!judas_memlock(&judas_mixrate, sizeof judas_mixrate)) return 0;
        if (!judas_memlock(&judas_mixmode, sizeof judas_mixmode)) return 0;
        if (!judas_memlock(&judas_bufferlength, sizeof judas_bufferlength)) return 0;
        if (!judas_memlock(&judas_buffermask, sizeof judas_buffermask)) return 0;
        if (!judas_memlock(&judas_clipbuffer, sizeof judas_clipbuffer)) return 0;
        if (!judas_memlock(&judas_zladdbuffer, sizeof judas_zladdbuffer)) return 0;
        if (!judas_memlock(&judas_cliptable, sizeof judas_cliptable)) return 0;
        if (!judas_memlock(&judas_volumetable, sizeof judas_volumetable)) return 0;
        if (!judas_memlock(&judas_mixpos, sizeof judas_mixpos)) return 0;
        if (!judas_memlock(&judas_samplesize, sizeof judas_samplesize)) return 0;
        if (!judas_memlock(&judas_player, sizeof judas_player)) return 0;
        if (!judas_memlock(&judas_bpmcount, sizeof judas_bpmcount)) return 0;
        if (!judas_memlock(&judas_bpmtempo, sizeof judas_bpmtempo)) return 0;
        if (!judas_memlock(&judas_channel, sizeof judas_channel * CHANNELS)) return 0;
        if (!judas_memlock(&judas_zerolevell, sizeof judas_zerolevell)) return 0;
        if (!judas_memlock(&judas_zerolevelr, sizeof judas_zerolevelr)) return 0;
        if (!judas_memlock(&judas_clipped, sizeof judas_clipped)) return 0;
        if (!judas_memlock(&filewriterbuffer, sizeof filewriterbuffer)) return 0;
        if (!judas_memlock(&filewriterbuffersize, sizeof filewriterbuffersize)) return 0;
        judas_locked = 1;
        return 1;
}

