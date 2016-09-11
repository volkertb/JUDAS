/*
 * JUDAS raw sample routines
 */

#include <stdlib.h>
#include <stdio.h>
#include <mem.h>
#include "judas.h"

SAMPLE *judas_loadrawsample(char *name, int repeat, int end, unsigned char voicemode);

SAMPLE *judas_loadrawsample(char *name, int repeat, int end, unsigned char voicemode)
{
        int length;
        int handle;
        SAMPLE *smp;

        /* Don't waste memory if Nosound */
        judas_error = JUDAS_OK;
        if (judas_device == DEV_NOSOUND)
        {
                return &fakesample;
        }

        judas_error = JUDAS_OPEN_ERROR;
        handle = judas_open(name);
        if (handle == -1) return NULL;
        length = judas_seek(handle, 0, SEEK_END);
        if (length == -1)
        {
                judas_close(handle);
                return NULL;
        }
        judas_seek(handle, 0, SEEK_SET);
        smp = judas_allocsample(length);
        if (!smp)
        {
                judas_close(handle);
                return NULL;
        }
        if (end == 0) end = length;
        if (end > length) end = length;
        if (repeat > length - 1) repeat = length - 1;
        judas_error = JUDAS_READ_ERROR;
        if (judas_read(handle, smp->start, length) != length)
        {
                judas_freesample(smp);
                judas_close(handle);
                return NULL;
        }
        judas_close(handle);
        smp->repeat = smp->start + repeat;
        smp->end = smp->start + end;
        smp->voicemode = voicemode | VM_ON;
        judas_ipcorrect(smp);
        judas_error = JUDAS_OK;
        return smp;
}
