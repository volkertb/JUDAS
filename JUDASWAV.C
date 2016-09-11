/*
 * JUDAS WAV handling
 */

#include <stdlib.h>
#include <stdio.h>
#include <mem.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include "judas.h"

#ifdef __DJGPP__
#include <sys/stat.h> /* for mode definitions */ 
#include <unistd.h>   /* compatibility mode */
#define HANDLE_PRAGMA_PACK_PUSH_POP 1
#endif

#define MAXFILES 100  /* 100 max wav files per 1 library */

extern char *filewriterbuffer;
extern int filewriterbuffersize;
extern unsigned char judas_initialized;
extern void safemixer(void *address, int length);

SAMPLE *judas_loadwav(char *name);
void judas_setwavlib(char *name);


#pragma pack(push,1)
typedef struct
{
        char rifftext[4];
        unsigned totallength;
        char wavetext[4];
        char formattext[4];
        unsigned formatlength;
        unsigned short format;
        unsigned short channels;
        unsigned freq;
        unsigned avgbytes;
        unsigned short blockalign;
        unsigned short bits;
        char datatext[4];
        unsigned datalength;
} WAV_HEADER;

typedef struct
{
        unsigned offset;       // offset in library for this wav entry
        unsigned size;         // size of this wav entry
        char filename[64];     // wav filename
} WAV_ENTRY;

typedef struct
{
        char logo[4];          // JDSL format sign (header has max 100 wav entries)
        unsigned filescount;   // number of wav files in library
        unsigned encoding;     // encoding method
        WAV_ENTRY files[MAXFILES];  // single wav entry
} LIB_HEADER;
#pragma pack(pop)

static LIB_HEADER libheader;
static char libname[260] = {0};


/*
 * Opens a file for wav writing.
 * Returns nonnegative file handle if successful, -1 on error
 */
int judas_wavwriter_open(char *name)
{
  WAV_HEADER header;
  int handle;
  
  if (!judas_initialized) return -1;
  header.totallength = 0; // Will be determined after writing the pcm data
  header.datalength = 0; // Will be determined after writing the pcm data
  memcpy(header.rifftext, "RIFF", 4);
  memcpy(header.wavetext, "WAVE", 4);
  memcpy(header.formattext, "fmt ", 4);
  memcpy(header.datatext, "data", 4);
  header.formatlength = 16;
  header.format = 1;
  header.freq = judas_mixrate;
  switch (judas_mixmode) {
    case (MONO | EIGHTBIT):
      header.channels = 1;
      header.avgbytes = header.freq;
      header.blockalign = 1;
      header.bits = 8;
    break;
    case (MONO | SIXTEENBIT):
      header.channels = 1;
      header.avgbytes = header.freq * 2;
      header.blockalign = 2;
      header.bits = 16;
    break;
    case (STEREO | EIGHTBIT):
      header.channels = 2;
      header.avgbytes = header.freq * 2;
      header.blockalign = 2;
      header.bits = 8;
    break;
    case (STEREO | SIXTEENBIT):
      header.channels = 2;
      header.avgbytes = header.freq * 4;
      header.blockalign = 4;
      header.bits = 16;
    break;
  }

  handle = open(name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE);
  if (handle == -1) {
    return handle;
  }
  if (write(handle, &header, sizeof(WAV_HEADER)) < sizeof(WAV_HEADER)) {
    close(handle);
    return -1;
  }

  if (filewriterbuffer == NULL) {
    if (!(filewriterbuffer = locked_malloc(filewriterbuffersize))) return -1;
  }

  return handle;
}

/*
 * Wav writer. Writes one bufferlength of mixed data into the wav file.
 * Returns nonnegative file handle if successful, -1 on error
 */
int judas_wavwriter_writesome(int handle)
{
  if (!judas_initialized) return -1;
  if (handle == -1) return -1;

  safemixer(filewriterbuffer, filewriterbuffersize);
  if (write(handle, filewriterbuffer, filewriterbuffersize) < filewriterbuffersize) {
    close(handle);
    return -1;
  }
  return handle;
}

/*
 * Finishes wav writing and closes the wav writer file.
 * Returns 0 on success, -1 on error
 */
int judas_wavwriter_close(int handle)
{
  int totallength;
  int datalength;

  if (filewriterbuffer) locked_free(filewriterbuffer);

  totallength = filelength(handle);
  datalength = totallength - sizeof(WAV_HEADER);
  totallength -= 8;
  if (datalength < 0) {
    close(handle);
    return -1;
  }
  if (lseek(handle, 4, SEEK_SET) == -1) {
    close(handle);
    return -1;
  };
  if (write(handle, &totallength, 4) == -1) {
    close(handle);
    return -1;
  }
  if (lseek(handle, sizeof(WAV_HEADER)-4, SEEK_SET) == -1) {
    close(handle);
    return -1;
  };
  if (write(handle, &datalength, 4) == -1) {
    close(handle);
    return -1;
  }
  return close(handle);
}

SAMPLE *judas_loadwav(char *name)
{
        int length;
        int reallength;
        int handle;
        SAMPLE *smp;
        WAV_HEADER header;
        int fcount;

        /* Don't waste memory if Nosound */
        judas_error = JUDAS_OK;
        if (judas_device == DEV_NOSOUND)
        {
                return &fakesample;
        }

        /*
         * Try to open
         */
        judas_error = JUDAS_OPEN_ERROR;

        if (!libname[0]) strcpy (libname, "jdswav.lib");
		handle = judas_open(libname);
        if (handle != -1) {
                /* library open success */
                judas_error = JUDAS_READ_ERROR;
                if (judas_read (handle, &libheader, sizeof(LIB_HEADER)) != sizeof(LIB_HEADER)) {
                        judas_close (handle);
                        return NULL;
                }
        
                /* check JDSL library logo */
                if ((libheader.logo[0] != 'J') || (libheader.logo[1] != 'D') || (libheader.logo[2] != 'S')) {
                        judas_close (handle);
                        return NULL;
                }

                /* ni compare files and set library file pointer on success */
                for (fcount = 0; fcount < libheader.filescount; fcount++) {
                        if (strnicmp(name, (char *)&libheader.files[fcount].filename, sizeof(name)) == 0) {
                            if (judas_seek(handle, libheader.files[fcount].offset, SEEK_SET) == -1) {
                                    judas_close (handle);
                                    return NULL;
                            }
                            break;   // success - wav entry was found
                        }
                }        

                /* close library handle if wav file not found in library */
                if (fcount == libheader.filescount) {
                        judas_close (handle);
                }
        }
        
        /* try to load as single wav file if wav entry not found in library */
        if (handle == -1) {
                handle = judas_open(name);
                if (handle == -1) return NULL;
        }

        /*
        * Read identification
        */
        judas_error = JUDAS_READ_ERROR;
        if (judas_read(handle, &header, 12) != 12)
        {
                judas_close(handle);
                return NULL;
        }
        judas_error = JUDAS_WRONG_FORMAT;
        if (memcmp("RIFF", header.rifftext, 4))
        {
                judas_close(handle);
                return NULL;
        }
        if (memcmp("WAVE", header.wavetext, 4))
        {
                judas_close(handle);
                return NULL;
        }
        /*
         * Search for the FORMAT chunk
         */
        for (;;)
        {
                judas_error = JUDAS_READ_ERROR;
                if (judas_read(handle, &header.formattext, 8) != 8)
                {
                        judas_close(handle);
                        return NULL;
                }
                if (!memcmp("fmt ", &header.formattext, 4)) break;
                if (judas_seek(handle, header.formatlength, SEEK_CUR) == -1)
                {
                        judas_close(handle);
                        return NULL;
                }
        }
        /*
         * Read in the FORMAT chunk
         */
        if (judas_read(handle, &header.format, 16) != 16)
        {
                judas_close(handle);
                return NULL;
        }
        /*
         * Skip data if the format chunk was bigger than what we use
         */
        if (judas_seek(handle, header.formatlength - 16, SEEK_CUR) == -1)
        {
                judas_close(handle);
                return NULL;
        }
        /*
         * Check for correct format
         */
        judas_error = JUDAS_WRONG_FORMAT;
        if (header.format != 1)
        {
                judas_close(handle);
                return NULL;
        }
        /*
         * Search for the DATA chunk
         */
        for (;;)
        {
                judas_error = JUDAS_READ_ERROR;
                if (judas_read(handle, &header.datatext, 8) != 8)
                {
                        judas_close(handle);
                        return NULL;
                }
                if (!memcmp("data", &header.datatext, 4)) break;
                if (judas_seek(handle, header.datalength, SEEK_CUR) == -1)
                {
                        judas_close(handle);
                        return NULL;
                }
        }
        /*
         * Allocate sample, load audio data, do processing (unsigned->signed,
         * stereo->mono)
         */
        length = header.datalength;
        reallength = length;
        if (header.channels == 2) reallength >>= 1;
        smp = judas_allocsample(reallength);
        if (!smp)
        {
                judas_close(handle);
                return NULL;
        }
        if (header.channels == 2)
        {
                if (header.bits == 16)
                {
                        unsigned count = length >> 2;
                        short *buffer;
                        short *src;
                        short *dest;

                        judas_error = JUDAS_OUT_OF_MEMORY;
                        buffer = malloc(length);
                        if (!buffer)
                        {
                                judas_freesample(smp);
                                judas_close(handle);
                                return NULL;
                        }
                        judas_error = JUDAS_READ_ERROR;
                        if (judas_read(handle, buffer, length) != length)
                        {
                                free(buffer);
                                judas_freesample(smp);
                                judas_close(handle);
                                return NULL;
                        }
                        src = buffer;
                        dest = (short *)smp->start;
                        while (count--)
                        {
                                int average = (src[0] + src[1]) / 2;
                                *dest = average;
                                src += 2;
                                dest++;
                        }
                        free(buffer);
                        smp->repeat = smp->start;
                        smp->end = smp->start + reallength;
                        smp->voicemode = VM_ON | VM_16BIT;
                }
                else
                {
                        unsigned count = length >> 1;
                        unsigned char *buffer;
                        unsigned char *src;
                        signed char *dest;

                        judas_error = JUDAS_OUT_OF_MEMORY;
                        buffer = malloc(length);
                        if (!buffer)
                        {
                                judas_freesample(smp);
                                judas_close(handle);
                                return NULL;
                        }
                        judas_error = JUDAS_READ_ERROR;
                        if (judas_read(handle, buffer, length) != length)
                        {
                                free(buffer);
                                judas_freesample(smp);
                                judas_close(handle);
                                return NULL;
                        }
                        src = buffer;
                        dest = (signed char *)smp->start;
                        while (count--)
                        {
                                int average = (src[0] + src[1] - 0x100) / 2;
                                *dest = average;
                                src += 2;
                                dest++;
                        }
                        free(buffer);
                        smp->repeat = smp->start;
                        smp->end = smp->start + reallength;
                        smp->voicemode = VM_ON;
                }
        }
        else
        {
                if (header.bits == 16)
                {
                        judas_error = JUDAS_READ_ERROR;
                        if (judas_read(handle, smp->start, length) != length)
                        {
                                judas_freesample(smp);
                                judas_close(handle);
                                return NULL;
                        }
                        smp->repeat = smp->start;
                        smp->end = smp->start + length;
                        smp->voicemode = VM_ON | VM_16BIT;
                }
                else
                {
                        unsigned count = length;
                        char *src = smp->start;

                        judas_error = JUDAS_READ_ERROR;
                        if (judas_read(handle, smp->start, length) != length)
                        {
                                judas_freesample(smp);
                                judas_close(handle);
                                return NULL;
                        }
                        while (count--)
                        {
                                *src += 0x80;
                                src++;
                        }
                        smp->repeat = smp->start;
                        smp->end = smp->start + length;
                        smp->voicemode = VM_ON;
                }
        }
        judas_ipcorrect(smp);
        judas_error = JUDAS_OK;
        judas_close(handle);
        return smp;
}

void judas_setwavlib(char *name)
{
    strcpy (libname, name);
}

