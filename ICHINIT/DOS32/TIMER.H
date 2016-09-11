
/*****************************************************************************/
/* globals for the timer handler functions (TIMER.ASM)                       */
/*****************************************************************************/

#define PITFrequency 1193181
extern DWORD          flip2_TimerErrorcode;   /* dword - timer error code */
extern BOOL           FLIP2API flip2_StartTimer (void *tproc,int freq);
extern BOOL           FLIP2API flip2_StopTimer (void *tproc);
