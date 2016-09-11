/*
 * Judas Player. This program plays MODs, XMs and S3Ms and shows how to set up
 * the signal handlers to ensure proper JUDAS shutdown even in the case of user
 * break.
 *
 * It uses the timer routines in TIMER.C & TIMERASM.ASM. These don't
 * "officially" belong to JUDAS but use them if you need them!
 *
 * Note: New features of Judas are automagically included in the new versions
 *       of JP, so they are not documented here.
 * V2.04 Corrected special keys leaving shit to keyboard buffer -bug.
 * V2.06 Rewrote a lot of things...
 * V2.07 Support for 48000 Hz sample rate (AC97 only).
 * V2.08 Now should really work with all ICH4 AC97 integrated codecs.
 * V2.09 SIS7012 should now work too
 * V2.09b ICH4+ support corrected
 * V2.09c dev version
 * V2.09d corrected dev version with full WATCOM C++ & DJGPP support
 * V2.09e all ICH4+ AC97 devices now use memory mapped IO if available
 * V2.09f WAV library builder added to JUDAS
 * V2.10a HDA codecs support added
 * V2.10b judas_setwavlib function added
 * V2.10d HDA codecs support update for intel 5, 6, 7 series test by RayeR
 */

#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <signal.h>
#include <conio.h>
#include <ctype.h>
#include <process.h>
#include <string.h>
#include <math.h>
#ifdef __DJGPP__
#include <unistd.h>
#include <go32.h>
#include <dpmi.h>
#include <sys/nearptr.h>
/* declare flushall */
#include <unistd.h>
void flushall (void)
{
  /* Simulate `flushall' by only flushing standard streams  */
  fflush (stdout);
  fsync (fileno (stdout));
  fflush (stderr);
  fsync (fileno (stderr));
}
#else
/* WATCOM C++ graphics library  & define djgpp base as absolute zero */
#define __djgpp_conventional_base 0
#include <graph.h>
#endif
#include "judas.h"
#include "timer.h"

/* For direct video memory access. (Does this work at all with PMODE?) */
#define SCREEN_AREA 0xb800
#define SCREEN_LIN_ADDR ((SCREEN_AREA) << 4)
#define SCREEN_SIZE 80*25

#define VUMETERBOTTOM 17
#define VUMETERXSIZE 80
#define VUMETERLEFT 0
#define VUMETERTOP 0
#define VUMETERLINES (VUMETERBOTTOM-VUMETERTOP+1)


/* only for testing purposes */
extern long hda_getbufpos (void);


/* Functions */
void buildmainscreen(void);
void updatemainscreen(void);
int finishfilename(char *filename);
void wavizefilename(char *filename);
int main(int argc, char **argv);
static void handle_int(int a);

/* Variables */
int filewriter = 0;
int mixer = QUALITYMIXER;
int filetype = 0;
int interpolation = 1;
int songrounds = 1;
int channels = 80;
int vumeterwidth = 1;
int vumeterson = 1;
int mainscreenon = 1;
char filename[254];
int mastervolume = 50;
int oldmastervolume;
char oldjudas_clipped;

const char *const extensiontable[] =
{
  ".xm",
  ".mod",
  ".s3m",
  NULL
};

/*
 * Bright and dim versions of the 16 textmode colors.
 */
const char hilite[] = { 0,9,10,11,12,13,14,15,7,11,14,15,14,15,15,15 };
const char lolite[] = { 0,1,2,8,4,5,8,8,8,1,2,3,4,5,6,7 };

#ifdef __WATCOMC__
void jdsgotoxy(char x, char y);
/*
 * Moves cursor to selected position. (Does this work at all with PMODE?)
 */
extern void jdsgotoxy(char x, char y);
#pragma aux jdsgotoxy = \
 "push bp"              \
 "xor bh, bh"           \
 "mov ah,2"             \
 "int 10h"              \
 "pop bp"               \
 parm   [dl] [dh]       \
 modify [ah bh];
#else
void jdsgotoxy(char x, char y)
{
  gotoxy(x+1, y+1);
}
#endif
 
 
 
/*
 * Builds the player screen.
 */
void buildmainscreen(void)
{
  #ifdef __DJGPP__
  /* use conio.h for DJGPP */
  textmode(C80);
  #else
  /* WATCOM C++ */
  _setvideomode(_TEXTC80);
  #endif
  jdsgotoxy(0,VUMETERBOTTOM+1);
  printf("ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ JUDAS V2.10d player ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ"
         "[ESC] Quit, [D] DOS shell, [<Ä>] Rewind/forward, [+-] Mastervol:%3d\n", mastervolume);
  if (!filewriter) {
    printf("%s, port %X IRQ %d DMA %d\n",
     judas_devname[judas_device], judas_port, judas_irq, judas_dma);
  } else {
    printf("Writing WAV: %s\n", filename);
  }
  printf("%s %d Hz, %s, %s IP\n",
   judas_mixmodename[judas_mixmode], judas_mixrate,
   judas_mixername[mixer], judas_ipmodename[interpolation]);

  switch(filetype)
  {
          case 1:
          channels = judas_getxmchannels();
          printf("XM name: %s\n", judas_getxmname());
          printf("Channels: %d\n", channels);
          break;

          case 2:
          channels = judas_getmodchannels();
          printf("MOD name: %s\n", judas_getmodname());
          printf("Channels: %d\n", channels);
          break;

          case 3:
          channels = judas_gets3mchannels();
          printf("S3M name: %s\n", judas_gets3mname());
          printf("Channels: %d\n", channels);
          break;
  }
  vumeterwidth = VUMETERXSIZE/channels;
}


/*
 * Updates the player screen.
 */
void updatemainscreen(void)
{
  if (oldmastervolume != mastervolume) {
    jdsgotoxy(64,VUMETERBOTTOM+2);
    printf("%3d",mastervolume);
    oldmastervolume = mastervolume;
  }
  flushall();
  if (oldjudas_clipped != judas_clipped) {
    jdsgotoxy(68, VUMETERBOTTOM+2);
    if (judas_clipped) printf("Clipped:%3d%%", 1+(int)400*judas_clipped/256); else printf("            ");
    oldjudas_clipped = judas_clipped;
  }
  flushall();

/*
  jdsgotoxy(0,24);  
  // only for testing purposes
  hda_getbufpos();
  flushall();
*/

  jdsgotoxy(0,24);
  switch(filetype)
  {
          case 1:
          printf("Pos:%3d Line:%3d Tick:%3d", judas_getxmpos(), judas_getxmline(), judas_getxmtick());
          break;

          case 2:
          printf("Pos:%3d Line:%3d Tick:%3d", judas_getmodpos(), judas_getmodline(), judas_getmodtick());
          break;

          case 3:
          printf("Pos:%3d Line:%3d Tick:%3d", judas_gets3mpos(), judas_gets3mline(), judas_gets3mtick());
          break;
  }
  flushall();

  /* Draw vumeters... */
  if (vumeterson) {
    short *screenmem = (short *)((unsigned char *)SCREEN_LIN_ADDR + __djgpp_conventional_base);
    int count;
    for (count = 0; count < channels; count++)
    {
      short *ptr = screenmem + 80*VUMETERTOP + count*vumeterwidth;
      float floatvu = sqrt(judas_getvumeter(count));
      char color = (((unsigned int)judas_channel[count].end / 15) % 15) + 1;
      int halves = floatvu *(VUMETERLINES*2+1);
      int zeros;
      int vu = halves/2;
      halves -= vu*2;
      zeros = VUMETERLINES-vu-halves;

      while (zeros)
      {
        int fuck = vumeterwidth;
        while (fuck)
        {
          *ptr = 0;
          ptr++;
          fuck--;
        }
        ptr += (80-vumeterwidth);
        zeros--;
      }
      if (halves)
      {
        int fuck = vumeterwidth-1;
        if (fuck) {
          *ptr = 220+256*hilite[(unsigned char)color];
          ptr++;
          fuck--;
        }
        while (fuck)
        {
          *ptr = 220+256*color;
          ptr++;
          fuck--;
        }
        *ptr = 220+256*lolite[(unsigned char)color];
        ptr++;
        ptr += (80-vumeterwidth);
      }
      while (vu)
      {
        int fuck = vumeterwidth-1;
        if (fuck) {
          *ptr = 219+256*hilite[(unsigned char)color];
          ptr++;
          fuck--;
        }
        while (fuck)
        {
          *ptr = 219+256*color;
          ptr++;
          fuck--;
        }
        *ptr = 219+256*lolite[(unsigned char)color];
        ptr++;
        ptr += (80-vumeterwidth);
        vu--;
      }
    }
  }
}


/*
 * Tries some extensions. Returns 1 if finds the file, otherwise 0.
 */
int finishfilename(char *filename)
{
  /* Maybe the filename is already right... If, return 1. */
  {
    int handle = judas_open(filename);
    if (handle != -1)
    {
      judas_close(handle);
      return 1;
    }
  }
  {
    int pos = strlen(filename)-1;
    int pointpos = pos+1;
    /* Is there already an extension in the filename? */
    {
      while (filename[pos] != ':' &&
             filename[pos] != '\\' &&
             filename[pos] != '/' &&
             pos >= 0)
      {
        if (filename[pos] == '.')
        {
          /* No match! */
          return 0;
        }
        pos--;
      }
      /* Try with different extensions */
      {
        int count;

        for (count = 0; extensiontable[count]; count++) {
          int handle;
          strcat(filename, extensiontable[count]);
          handle = judas_open(filename);
          if (handle != -1) {
            judas_close(handle);
            return 1;
          }
          filename[pointpos] = 0;
        }
        /* No match! */
        return 0;
      }
    }
  }
}

/*
 * Removes path from the filename and changes extension to ".wav".
 */
void wavizefilename(char *filename)
{
  int pos = strlen(filename)-1;
  while (filename[pos] != ':' &&
         filename[pos] != '\\' &&
         filename[pos] != '/' &&
         pos >= 0)
  {
    if (filename[pos] == '.')
    {
      filename[pos] = 0;
      goto FOUNDEXTPOS;
    }
    pos--;
  }
  pos++;
  if (pos == strlen(filename)-1) {
    strcpy(filename, "output.wav");
    return;
  }
  FOUNDEXTPOS:
  strcat(filename, ".wav");
  pos = strlen(filename)-1;
  while (filename[pos] != ':' &&
         filename[pos] != '\\' &&
         filename[pos] != '/' &&
         pos >= 0)
  {
    pos--;
  }
  strcpy(filename, &filename[pos+1]);
}

/*
 * The player skeleton!
 */
int main(int argc, char **argv)
{
        int mixrate = 48000;
        int mixmode = SIXTEENBIT | STEREO;
        int wavhandle = 0;

        /* Set signal handlers */
        signal(SIGINT, handle_int);
        #if defined(SIGBREAK)
        signal(SIGBREAK, handle_int);
        #endif

        #ifdef __DJGPP__
        if (__djgpp_nearptr_enable() == 0) {
               printf("ERROR: Couldn't enable near pointers for DJGPP!\n");
               return 1;
        }
        /* Trick: Avoid re-setting DS selector limit on each memory allocation call */
	    __djgpp_selector_limit = 0xffffffff;
        #endif

        /* Print some shit */
        printf("ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ\n"
               "JUDAS V2.10d player - plays XMs+MODs+S3Ms\n"
               "ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ\n");

        /* Check we have enough commandline parameters */
        if (argc == 1)
        {
               printf("Voluntary stuff marked with <>\n"
                      "\n"
                      "Usage:   jp musicfile <options>\n"
                      "Options: -rxxx   Mixrate, in Hz\n"
                      "         -vxxx   Mastervolume, 0-255, default: 50\n"
                      "         -m      Output mono\n"
                      "         -8      Output 8bit\n"
                      "         -i      Simpler interpolation\n"
                      "         -f      Fast mixer, default: Quality mixer\n"
                      "         -l<xxx> Loop song, play song xxx times, default: once.\n"
                      "                 If no xxx given or xxx zero, will play forever.\n"
                      "         -w      WAV writer, outputs audio into a WAV file named\n"
                      "                 after the musicfile, in the current directory.\n"
                      "         -d      Disable player screen\n"
                      "         -dv     Disable vumeters\n");
               return 1;
        }

        /* Parse options */
        if (argc > 2)
        {
                int count = 2;
                while (count < argc)
                {
                        char *ptr = argv[count];
                        if ((ptr[0] == '-') || (ptr[0] == '/'))
                        {
                                switch(tolower(ptr[1]))
                                {
                                        case 'r':
                                        sscanf(&ptr[2], "%d", &mixrate);
                                        break;

                                        case 'v':
                                        sscanf(&ptr[2], "%d", &mastervolume);
                                        if (mastervolume < 0) mastervolume = 0;
                                        if (mastervolume > 255) mastervolume = 255;
                                        break;

                                        case 'm':
                                        mixmode &= SIXTEENBIT;
                                        break;

                                        case '8':
                                        mixmode &= STEREO;
                                        break;

                                        case 'i':
                                        interpolation = 0;
                                        break;

                                        case 'f':
                                        mixer = FASTMIXER;
                                        break;

                                        case 'l':
                                        songrounds = 0;
                                        sscanf(&ptr[2], "%d", &songrounds);
                                        break;

                                        case 'w':
                                        filewriter = 1;
                                        judascfg_device = DEV_FILE;
                                        break;

                                        case 'd':
                                        if (tolower(ptr[2]) == 'v') vumeterson = 0;
                                        if (!ptr[2]) mainscreenon = 0;
                                        break;
                                }
                        }
                        count++;
                }
        }

        /* Get and finish filename */
        strcpy(filename, argv[1]);
        if (!finishfilename(filename))
        {
          printf("ERROR: Couldn't find musicfile!\n");
          return 1;
        }
        printf("LOADING FILE: %s\n",filename);
        flushall();

        /* Set uninit functions to be called at exit */
        atexit(judas_uninit);

        /* Autoconfigure by using enviroment */
        if (!filewriter) {
          judas_config();
        }

        /* Try to init */
        if (!judas_init(mixrate, mixer, mixmode, interpolation)) goto ERROR;

        /* Exit if in no sound mode */
        if (judas_device == DEV_NOSOUND)
        {
          printf("ERROR: Sound card not detected!\n");
          return 1;
        }

        /* Set mastervolume of channels */
        judas_setmusicmastervolume(CHANNELS, mastervolume);

        /*
         * Try to load, first XM, then MOD, then S3M
         */
        filetype = 1;
        judas_loadxm(filename);
        if (judas_error == JUDAS_OK) goto PLAYIT;
        if (judas_error != JUDAS_WRONG_FORMAT) goto ERROR;
        filetype = 2;
        judas_loadmod(filename);
        if (judas_error == JUDAS_OK) goto PLAYIT;
        if (judas_error != JUDAS_WRONG_FORMAT) goto ERROR;
        filetype = 3;
        judas_loads3m(filename);
        if (judas_error == JUDAS_OK) goto PLAYIT;
        if (judas_error != JUDAS_WRONG_FORMAT) goto ERROR;

        ERROR:
        printf("JUDAS ERROR: %s\n", judas_errortext[judas_error]);
        return 1;

        PLAYIT:
        printf("PLAYING ");
        flushall();
        switch(filetype)
        {
                case 1:
                printf("XM\n");
                judas_playxm(songrounds);
                break;

                case 2:
                printf("MOD\n");
                judas_playmod(songrounds);
                break;

                case 3:
                printf("S3M\n");
                judas_plays3m(songrounds);
                break;
        }

        /* Hook timer to update sound */
        if (!filewriter) {
          timer_init(0x4300, judas_update);
          atexit(timer_uninit);
        } else {
          wavizefilename(filename);
          wavhandle = judas_wavwriter_open(filename);
          if (wavhandle == -1) {
            printf("WAV WRITER ERROR: Error creating the WAV file!");
            return 1;
          };
        }

        if (mainscreenon) {
          buildmainscreen();
          updatemainscreen();
        } else {
          printf("[ESC] Quit, [D] DOS shell, [<Ä>] Rewind/forward, [+-] Mastervol\n");
          flushall();
        }

        for (;;)
        {
                char c;

                while (!kbhit() && judas_songisplaying())
                {
                        if (filewriter) {
                          if (judas_wavwriter_writesome(wavhandle) == -1) {
                            #ifdef __DJGPP__
                            /* use conio.h for DJGPP */
                            flushall();
                            if (mainscreenon) textmode(LASTMODE);
                            #else
                            /* WATCOM C++ */
                            flushall();
                            if (mainscreenon) _setvideomode(_DEFAULTMODE);
                            #endif
                            printf("WAV WRITER ERROR: Error writing the WAV file!");
                            return 1;
                          }
                        }
                        if (mainscreenon) updatemainscreen();
                 }

                if (kbhit()) c = toupper(getch()); else c = 27;
                switch (c) {
                    case 77:
                    {
                      switch(filetype) {
                        case 1:
                        judas_forwardxm();
                        break;

                        case 2:
                        judas_forwardmod();
                        break;

                        case 3:
                        judas_forwards3m();
                        break;
                      }
                    }
                    break;
                    case 75:
                    {
                      switch(filetype) {
                        case 1:
                        judas_rewindxm();
                        break;

                        case 2:
                        judas_rewindmod();
                        break;

                        case 3:
                        judas_rewinds3m();
                        break;
                      }
                    }
                    break;
                    case '+':
                    {
                      if (mastervolume < 20) mastervolume++; else mastervolume += 5;
                      if (mastervolume > 255) mastervolume = 255;
                      judas_setmusicmastervolume(CHANNELS, mastervolume);
                      judas_clipped = 0;
                    }
                    break;
                    case '-':
                    {
                      if (mastervolume < 21) mastervolume--; else mastervolume -= 5;
                      if (mastervolume < 0) mastervolume = 0;
                      judas_setmusicmastervolume(CHANNELS, mastervolume);
                      judas_clipped = 0;
                    }
                    break;
                    case 'D':
                    {
                        #ifdef __DJGPP__
                        /* use conio.h for DJGPP */
                        flushall();
                        if (mainscreenon) textmode(LASTMODE);
                        #else
                        /* WATCOM C++ */
                        flushall();
                        if (mainscreenon) _setvideomode(_DEFAULTMODE);
                        #endif
                        printf("Type \"exit\" to return to JP...");
                        #ifdef __DJGPP__
                        flushall();
                        #else
                        flushall();
                        #endif
                        spawnl(P_WAIT, getenv("COMSPEC"), NULL);
                        if (!filewriter) if (!judas_init(mixrate, mixer, mixmode, interpolation)) goto ERROR;
                        if (mainscreenon) buildmainscreen();
                    }
                    break;
                    case 27:
                    {
                        if (c == 0) getch();
                        switch(filetype)
                        {
                                /*
                                 * Freeing isn't necessary but we do it just
                                 * to make sure it doesn't crash (page fault
                                 * will occur when freeing memory if heap has
                                 * been corrupted by the player!)
                                 */
                                case 1:
                                judas_freexm();
                                break;

                                case 2:
                                judas_freemod();
                                break;

                                case 3:
                                judas_frees3m();
                                break;
                        }
                        goto BYEBYE;
                    }
                    break;
                }
        }
        BYEBYE:
        #ifdef __DJGPP__
        /* use conio.h for DJGPP */
        flushall();
        if (mainscreenon) textmode(LASTMODE);
        #else
        /* WATCOM C++ */
        flushall();
        if (mainscreenon) _setvideomode(_DEFAULTMODE);
        #endif
        if (filewriter) {
          if (judas_wavwriter_close(wavhandle) == -1) {
            printf("WAV WRITER ERROR: Error finishing the WAV file!");
            return 1;
          };
        }
        printf("Bye! Thank you for using Judas Sound System :)\n");
        if (judas_clipped) printf("Try playing the same song with mastervol %d to avoid clipping", (int)((float)100*mastervolume/(1+(float)400*judas_clipped/256)));
        return 0;
}

static void handle_int(int a)
{
        exit(0); /* Atexit functions will be called! */
}
