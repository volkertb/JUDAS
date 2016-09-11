Build notes by RayeR
********************

This source package was accomodated to use with latest GCC 4.6.x, DJGPP 2.04
Due to GCC optimalization issues since ver. 4.0.1 I modified global
optimization flag from -O3 to -Os (-O1 and higher produced crashing code).
It would be better to isolate problem function or file and modify the
optimization locally.
To build the package just run !DJGPP.BAT under plain DOS or Windows DOS box.
When using intel HDA be sure to not have set BLASTER environment variable
otherwise it will try initialize SB first.


           █                   ▀▄                        ▄▄            ▄▄▄▄▄
        ▄▄█▌     ▄▄▄    ▀▄▄▄▄    ██▄▄            ▄▄█▀▀▀▀▀▀██        ▄██▀    ▀▀▄
       ▀  ▐█▄  ▀▀█        ▐█ ▀▀ ▐█▀▀▀▀█▄        ▐██        █▌      █▀         █
           ▀██   █▌       ▐█    █▌     ▀█▄      ▐██        ██     ▐█       ▀▄▀
            ▐█▌  █▌        █▌  ▐█        ▀█▄    ▐██        ▐█▌     ██▄
             ██  ▐█        █▌  ▐█▌         ██▄  ▐█▌        ▐█▌      ▀▀██▄▄
             ██  ▐█        ▐█  ▐█▌          ▀█▌ ██▌  ▄▄▄▄▄ ▐█▌          ▀██▄
             ██  ▐█▌       ▐█  ▐██           █▌ ███▀▀▀▀▀▀▀▀███▄           ▀██
            ▐██   █▌       ▐█▌  ██           █  ██          ██ █            ██
   ▄▄       ▐█▌   ██        ██  ▐█▌         █▌ ▐█▌          ██     ▄▄       ▐█▌
  █ ▐█      ██    ██        ██▌  ██        ██  ▐█           ██   ▄▀  ▀       ██
 ▀  █      ▐█▌   ▐█▌       ▄███  ▐█▌    ▄▄██   ██           ██   █▄         ▐█▌
   ▐▌      ██    ██    ▄▄████▀   █████████▀   ▄███▄       ▄███▄   ██       ▄██
   ██▄   ▄██▀   ▐██ ▄███▀▀      ██▀▀▀       ▄▀                 ▀   ██▄   ▄██▀
    ▀████▀▀      ▀███▀        ▄▀                                    ▀████▀▀

           Apocalyptic   Softwaremixing    Soundsystem   Version 2.10d


                      "Black Spider's April 2008 Release"
                   with HDA support under the GNU/GPL license


                                DOCUMENTATION
                                -------------

                                  Contents:

                               1. INTRODUCTION
                               2. FEATURES
                               3. HOW TO USE
                               4. CONFIGURING
                               5. HISTORY
                               6. FINAL WORDS


---------------
1. INTRODUCTION
---------------

This is version 2.10d of JUDAS Apocalyptic Softwaremixing Soundsystem.
System+soundcard code & original (buggy) players by Cadaver.
Most bugfixes, additions and quality mixer by Yehar.
Intel ICH AC97 and HDA codecs support by Black Spider.

JUDAS can be used to create devastating digital virtual music & fx
abominations with the most popular soundcards. It is mainly intended for games,
demos and musicdisks.

JUDAS is exclusively intended to be used with Watcom 32bit flat protected
mode DOS programs, with either DOS4GW or any other complatible DOS extender.
With DOS4GW soundcard IRQs above 7 don't work, though.

Previous versions of JUDAS were blasphemy-ware. This means that:
        * Use at own risk - authors take no responsibility for anything!
        * Documentation & attitude aren't "serious" or "professional", if
          you don't like this then look elsewhere!
        * Contains strictly non-portable DOS code
        * Full sources are included
        * Judas is free, it would be nice though
          if you include the authors in the credits.
        * Kutulu will always be watching you
        * Listen to True Nordic Black Metal while programming, preferably
          Burzum. Or listen to Supreme Teutonic Power Metal :D

This version however (all the HDA codec functions only) is released under the
GNU/GPL license. The full text of this license can be found in the file gpl20.txt
The HDA code was written after analyzing various HDA linux related sources,
the drivers from ALSA project made by Takashi Iwai, the OSS sound system project
and others. Although this code is different and pretty unique some similarities
especially in the HDA nodes access strategy can be found. That is why I decided
to publicly release this version of Judas under the GNU/GPL license.



-----------
2. FEATURES
-----------

Soundcards supported:

        * SOUND BLASTER
          - 8bit sound
          - mono
          - autoinit mode for DSP versions > 2.00
          - mixrate up to 22050 Hz
        * SOUND BLASTER PRO
          - 8bit sound
          - mono/stereo
          - mixrate up to 22050 Hz (stereo) or 44100 Hz (mono)
        * SOUND BLASTER 16 &
          GRAVIS ULTRASOUND
          - 8bit/16bit sound
          - mono/stereo
          - mixrate up to 44100 Hz
        * INTEL ICH AC97 CODEC (and compatibles)
          - 16bit sound
          - stereo
          - mixrate up to 48000 Hz


        ATTENTION:
        ----------
     >> GRAVIS ULTRASOUND support is slightly more complicated than SB
        support. Sound must be transferred to the card's DRAM before it's
        played (it cannot play the DMA buffer directly. GUSMAX could, using
        the codec chip but as a matter of principle it won't be used!) This
        transfer uses a very high speed (650 kHz) which slows down the CPU a
        bit. Using a 16-bit DMA channel helps much.
        GUS support will work at least with Classic & MAX & PnP.


     >> INTEL ICH AC97/HDA CODECS are only supported in pure DOS. Under Windows,
        a SOUND BLASTER compatible device will be detected. If all supported
        devices SB/GUS/AC97/HDA are present (or emulated) in the system,
        the configuration procedure will choose the GRAVIS ULTRASOUND
        (by looking at the ULTRASND enviroment variable) as the best option.
        When only a SOUND BLASTER compatible card and an AC97/HDA codec
        is present, the config procedure will always chose the SOUND BLASTER
        device as default (by looking at the BLASTER enviroment variable).
        THE AC97/HDA code does not make use of any ISA DMA channel and does
        not need any IRQ (the AC97/HDA interrupt number is given for debug
        purposes only). The PCI Bus Master DMA engine is started once at
        the begining (init procedure) and stopped at the uninit procedure.
        From the programmer point of view, the AC97/HDA codec should be
        considered just like an SB comptible music card (it still needs a
        call to the judas update procedure, preferably at 50 - 70Hz to stay
        compatible with other supported devices).

        NOTE : The user may need to run the ichinit.com utility that comes
        with this package in order to unlock or adjust/decrease the AC97
        codec volume under pure DOS. The best way out would be to put
        ichinit.com in autoexec.bat. Should work (although there is
        absolutely no guarantee it will ;-) with AC97 integrated on
        i810/i810E/i810E2/i815/i815E/i815EG/i815EP/i815G/i815P/i820/i820E/
        i830MP/i840/i845/i845E/i845G/i845GL/i850/i850E/i860/440MX/E7500/SIS735
        and some AMD/nVidia chipsets. High Definition Audio codecs do not
        require ICHINIT to work properly. In theory all HDA platforms are
        supported.

        NOTE: AC97 codecs must support VRA (Variable Rate Audio) in order
        to change the sample rate to something else than 48000 Hz.


     >> Judas also has a WAV writer.


Fileformats supported:

        * XMs
        * MODs
        * S3Ms
        * WAVs
        * Raw samples

Software mixer features:

        * 32 channels (can be increased, if you need more!)
        * Unlimited amount of samples (dynamically allocated)
        * Clipping
        * True channel volume meters

       Fast mixer:
        * Optional linear interpolation
        * It's extremely fast! (read SPEED.DOC)

       Quality mixer:
        * 32 bit mixing
        * Click removal system
        * Either cubic (Hermite) or linear interpolation
        * Enough volume levels for even the faintest sounds
        * Makes Judas the best sounding sound system! (Hi Maz!) High-end audio
          freaks: Look at JUDASW.GIF and JUVSCPW.GIF for interpolation test
          results. The plots are spectrum analyses of sampled white noise
          played at a low frequency.

Multitasking compatibility stuff:

        * Developed almost exclusively in a WIN95 dos box, without problems!
        * Doesn't use Virtual DMA or any strange shit, just allocates
          the DMA buffer from conventional memory and everything works fine!
        * Soundcards work under WINDOWS, providing the sound driver doesn't
          fuck up. However, playback can take a LOT more time (especially on a
          GUS) than under pure DOS. Some known problems:
          - Old WIN3.1 GUS driver causes the machine to lock up if a DOS
            program uses the GUS.
          - The WIN95 driver of Avance SB16 clone causes the machine to lock
            up if a DOS program tries to use 16bit DMA.
          - Some WIN3.1 SB drivers might not work.
        * All player/mixer code & sound/music data is locked via the DPMI
          memory locking functions. Feel free to call judas_update() from your
          IRQ0 handler (remember to lock your own IRQ code/data though!)
        * If memory can't be locked (not enough physical memory), the situation
          will be treated as an out-of-memory error.


-------------
3. HOW TO USE
-------------

Here follows an explanation how to compile and link the soundsystem to your
programs. Later on all JUDAS functions are explained.

        * MAKEFILE builds the JUDAS library. Simply run wmake to compile.

        * You'll need NASM to re-compile the mixing routines in JUDASASM.ASM/JASMDJ.ASM.

        * MAKEEX.BAT will compile the JUDAS example programs. By default
          they'll be linked with WDOSX. MAKEJP.BAT and MAKEANAL.BAT compile
          the example programs separately.

        * To use JUDAS in your own programs, you must have the header files
          JUDAS.H, JUDASCFG.H & JUDASERR.H in your work directory. Put the
          following line in your program:

                #include "judas.h"

          When linking, remember to tell the linker to use the JUDAS.LIB file.
          For example:

                wlink F massacre N massacre.exe L judas.lib SYS wdosx

And now to the functions...but first an explanation of JUDAS error handling.
For functions that return a value, 1 is success and 0 is failure (except the
JUDAS file functions, they work like those defined in IO.H, e.g failed file
open returns -1.) If a function returns failure, you can check the reason from
the variable judas_error. Error codes are defined in JUDASERR.H, and there
are also text strings for them, so it's easy to print the error.

GENERAL FUNCTIONS
-----------------

void judas_config(void);

        This reads the soundcard type, port, IRQ & DMA from the enviroment
        variables (BLASTER & ULTRASND.) If none of them is found, it tries
        to detect an Intel ICH AC97 compatible codec by querying the PCI bus.
        If that fails too, it assumes there is no soundcard.
        The sound configuration can also be manually set, it consists of
        these variables (all this shit is defined in JUDAS.H)

                extern unsigned judascfg_device;
                extern unsigned judascfg_port;
                extern unsigned judascfg_irq;
                extern unsigned judascfg_dma1;
                extern unsigned judascfg_dma2; /* SB16 high DMA */

        Device must be DEV_NOSOUND, DEV_SB, DEV_SBPRO, DEV_SB16 or DEV_GUS.
        judascfg_dma2 is only needed for SB16. These variables can be changed
        anytime, for example to let the user try different settings.

        ATTENTION: In NOSOUND mode samples/modules aren't loaded/played, to
        conserve memory. There is no "simulated" playing. This will cause
        problems if you need to synchronize your main program to the music
        (demos!) But, it's not impossible to overcome this. Simply use a
        secondary timing method (for example, a counter increased by IRQ0)
        if in NOSOUND mode.

int judas_init(unsigned mixrate, unsigned mixer, unsigned mixmode, int interpolation);

        Tries to init the soundcard according to the sound configuration.
        If the soundcard doesn't respond, returns failure.
        When calling this function for the first time, it must also reserve
        the DMA buffer & the mixing internal buffers, so it might also fail
        if there's not enough (physical) memory.
        If everything is successful, the sound output is started and you can
        begin to call the judas_update() routine.

        Mixrate is the number of audio samples produced per second. It is auto-
        corrected if soundcard doesn't support a certain mixrate. Remember: CPU
        time usage is directly proportional to the mixrate, as it has to
        produce more audio data at higher rates!

        Mixer specifies the mixer used. You can select either QUALITYMIXER or
        FASTMIXER. If you use quality mixer, you'll get better sound quality,
        but mixing will take much more CPU time. Quality mixer improvements in
        comparison to fast mixer: 32 bit mixing, click removal, smooth volume
        slides, 16 bit interpolation of 8 bit samples, more precise volume,
        cubic interpolation. A simple guideline: If you are making a demo or a
        game and want to show off with the amazingly fast gfx, choose fast
        mixer. If you are making a music disk, choose quality mixer. You can
        also let the user decide! For a sizelimited intro, Judas is a bit too
        bloated.

        Mixmode can be one of these, and is also auto-corrected according to
        soundcard limitations. Stereo sound takes more processing power,
        especially if the channels aren't panned to extreme left/right or
        to middle. 16bitness shouldn't affect the CPU time usage much. Quality
        mixer won't get any faster even if you use mono sound (lazy code).

                MONO | EIGHTBIT
                STEREO | EIGHTBIT
                MONO | SIXTEENBIT
                STEREO | SIXTEENBIT

        With fast mixer, any nonzero value in the interpolation parameter turns
        interpolation on. It will make the sound MUCH better and takes only 50%
        more CPU time than normal mixing. Quality mixer always uses
        interpolation, and the interpolation parameter switches between cubic
        and linear interpolation. Zero makes it linear, any other value cubic.

        Here's a table that shows how the mixer and interpolation modes affect
        CPU time usage when playing a 16-bit test xm on a Pentium/166:

                          IP:  off linear cubic
                quality mixer  -     34%   36%
                   fast mixer  13%   19%   -

        If you want to change soundcard, mixer, mixmode, mixrate or anything,
        just call judas_init() again. There will be one empty DMA buffer
        played, which means a short pause in the sound.

void judas_update(void);

        This will mix new sound data to the DMA buffer. Must be called 20 or
        more times per second. This can be done either in the main program
        or in some interrupt (IRQ0 is good, reprogram the PIT clock to
        something like 50-70 Hz to reduce the amount of work done at a time.
        This means smoother CPU time usage. Send also the end-of-interrupt
        before mixing to allow other interrupts meanwhile, this is especially
        important for Original Sound Blaster & Ultrasound because they need
        as fast as possible response to the sound card IRQ.)

        NOTE: the interval at which you call judas_update() WILL NOT affect
        music tempo, but if you don't call it often enough the sound gets
        choppy as the old DMA buffer contents get played again.

        WEIRD SHIT ALERT: judas_update() has to query either the DMA controller
        or in case of Ultrasound, the soundcard itself, where the sound is
        currently playing. If judas_update() is called too often (many hundred
        times in a second), the hardware gives sometimes wrong positions.
        There is code to compensate that, but in case that you have a Pentium,
        clicks may still occur in the sound. To avoid this, it's best to wait
        for a retrace or do something other time-consuming task in between
        judas_update() calls. And of course, the easiest thing is, as mentioned
        above, to program the timer to update the sound. 70 Hz is an absolutely
        safe calling frequency.

void judas_uninit(void);

        This is the most important function, as it stops the sound and restores
        the soundcard IRQ vector. Make sure this is called at the end of the
        program, even if user presses Ctrl-C or Ctrl-Break. This is done by
        setting up signal handlers:

                int main(int argc, char **argv)
                {
                        /* Set judas_uninit() to be called at exit */
                        atexit(judas_uninit);

                        /* Set signal handlers */
                        signal(SIGINT, handle_int);
                        signal(SIGBREAK, handle_int);

                        judas_config();
                        judas_init();

                        ...
                }

                static void handle_int(int a)
                {
                        exit(0); /* Atexit functions will be called! */
                }

int judas_songisplaying(void);

        Returns 1 if a song is currently being played, otherwise returns 0.
        Use this function if you need to know if the song has reached its end
        and stopped.

int judas_wavwriter_open(char *name);
int judas_wavwriter_writesome(int handle);
int judas_wavwriter_close(int handle);

        WAV writer functions. You can use them to convert your songs into WAVs.
        Instructions: Initialize Judas, load a song, start playing it (do
        not call judas_update() in any phase), call judas_wavwriter_open() with
        the WAV file name as the parameter, the function returns a file handle
        for the opened WAV file, keep calling judas_wavwriter_writesome() with
        the handle as the parameter until the song is finished (indicated by
        judas_songisplaying()), call judas_wavwriter_close() with the handle
        as the parameter to finish the WAV writing process.

        These functions return -1 on error.

SAMPLE & CHANNEL FUNCTIONS
--------------------------

SAMPLE *judas_allocsample(int length);

        This allocates a new sample with the given length of bytes. In normal
        use you don't need to call this but if you want to play long streams
        of audio or want to do some strange synthetic abominations this might
        be for you.

        If you want to play a long audiostream, you must allocate a suitably
        long buffer with this function, stuff data into it, start playing and
        then spy the pos field of the channel structure to know where in the
        buffer to stuff new data. Actually this is a lot like the DMA buffer
        updating. I didn't want to make functions for this because I wouldn't
        need them myself and everyone has their own way to make this.

        judas_allocsample() also allocates the volume profile of the sample.
        The volume profile takes about 0.002x memory compared to the sampledata
        itself. It is used by the realtime volume meter routines.

void judas_ipcorrect(SAMPLE *smp);

        In normal use you really don't need this, but if you have allocated
        a sample with judas_allocsample(), then each time you change the
        contents of your sample data, you must call this function.

        It updates a 3,5K safety buffer directly after the sample data. This is
        needed because of mixing routine optimizations. Ipcorrect originally
        meant interpolation-correction, when the optimized routines in fast
        mixer weren't in use yet.

        It also calculates the volume profile of the sample, for later use by
        the realtime volume meter routines.

        When judas_allocsample() is called, the safety buffer is automatically
        added to the sample length, and enough memory for the volume profile is
        allocated, so you don't have to worry about those more.

        WARNING: You must set the end (and also repeat if you're using looping)
        fields of the sample structure before calling judas_ipcorrect(), or
        it won't do anything!

void judas_freesample(SAMPLE *smp);

        Frees a sample, no matter if it's made by yourself, or loaded with
        some of the functions. You can even free a sample which is running,
        and no crash!

        NOTE: you can't obtain the samplepointers used by the music players.
        This is for everyone's safety. And please don't call this function with
        some random shit as the sample pointer, it'll just fuck up!

void judas_playsample(SAMPLE *smp, unsigned chnum, unsigned frequency,
                      unsigned char volume, unsigned char panning);

        This function has many parameters, but they're really self-explaining.
        Channel numbers go from 0 to CHANNELS - 1. Volume must be in range 0 to
        64*256. Give the sampling frequency of your sample in the frequency
        parameter if you want it to be played at the original frequency.

        IMPORTANT: The music players use channels 0 to MOD_CHANNELS - 1. So
        if you're playing a 8-channel tune, 8 is the lowest free channel.
        Playing a sample on a music channel while music is on won't fuck up
        badly, but you will get strange sounds.

void judas_stopsample(unsigned chnum);

        Stops sound on channel chnum by turning the voicemode to VM_OFF.

void judas_preventdistortion(unsigned active_channels);

        This will set mastervolume on all channels to 256 / active_channels.
        This will totally prevent clipping distortion, but may cause the
        sound to be too quiet. Experiment by yourself.

void judas_setmastervolume(unsigned chnum, unsigned char mastervol);

        Sets mastervolume on channel chnum.

void judas_setmusicmastervolume(unsigned musicchannels, unsigned char mastervol);

        Sets mastervolume on channels 0 to musicchannels - 1.

void judas_setsfxmastervolume(unsigned musicchannels, unsigned char mastervol);

        Sets mastervolume on channels musicchannels to CHANNELS - 1, i.e
        on the channels that are free for sound fx.

SAMPLE *judas_loadrawsample(char *name, int repeat, int end, unsigned char voicemode);

        Loads a raw sample, and sets the repeat, end, and voicemode as you
        wish. Here the repeat and end aren't pointers, but offsets from sample
        start. Set end to 0 and it will be automatically set to the end of the
        sample.

SAMPLE *judas_loadwav(char *name);

        Loads a WAV file. WAVs are always one-shot. Stereo WAVs are converted
        to mono. The function should be able to skip properly the crap-chunks
        added by various Windows audio recording programs. In addition,
        starting from version 2.09f WAV files can also be loaded from
        a library. The default WAV library the system will check is the file
        JDSWAV.LIB. Please check jdswav.zip package for more details on how
        to create such a library.

void judas_setwavlib (char *name);

        Overrides the default library name for WAV files.

float judas_getvumeter(unsigned chnum);

        Estimates and returns the true volume on the specified channel. The
        range is 0..1, but not 1. The values you get don't depend on master
        volume or mixer.


MUSIC LOADING/PLAYING FUNCTIONS
-------------------------------

There are three function sets for XM, MOD and S3M format. Only those you use
will be linked to your program. The function behaviour is same in every set.
Here are the XM functions listed & explained:

int judas_loadxm(char *name);

        Loads an XM file. May fail because of various things, like file not
        found, read error, incorrect format or out-of-memory.

void judas_freexm(void);

        Frees an XM file, if one has been loaded. It isn't really necessary
        to do this, because judas_loadxm() will automatically free the previous
        XM. This is however a good function for testing purposes: if the heap
        has been corrupted by the player/mixer routines it will page-fault!
        (I haven't had a page-fault for many days now...)

void judas_playxm(void);
void judas_stopxm(void);
void judas_forwardxm(void);
void judas_rewindxm(void);
unsigned char judas_getxmpos(void);
unsigned char judas_getxmline(void);
unsigned char judas_getxmtick(void);
unsigned char judas_getxmchannels(void);
char *judas_getxmname(void);

        Should all explain themselves. You cannot hang the machine by calling
        the functions before an XM has been loaded!

JUDAS IO FUNCTIONS
------------------

int judas_open(char *name);
int judas_seek(int handle, int bytes, int whence);
int judas_read(int handle, void *buffer, int size);
void judas_close(int handle);

        These work really like the standard IO functions defined in IO.H. The
        idea of them is following: if you want to implement a datafile system
        (compressed, maybe) you need only to change these routines.

DPMI MEMORY-LOCKING FUNCTIONS
-----------------------------

int judas_memlock(void *start, unsigned size);
int judas_memunlock(void *start, unsigned size);
void *locked_malloc(int size);
void locked_free(void *address);
void *dos_malloc(int size);
void dos_free(void *address);


        These should be self-explanatory. According to the JUDAS "standard",
        judas_memlock() & judas_memunlock() return 1 on success and 0 on
        failure.

        locked_malloc() and locked_free() function just like the normal
        malloc() & free() functions, they'll just lock and unlock the memory
        automatically. locked_malloc() returns NULL either if:
                * there is not enough memory available
                * the memory cannot be locked

        dos_malloc() and dos_free() functions allocate and free low DOS memory

        For multitasking compatibility, you MUST lock all memory (code, data,
        variables) touched by your IRQ routines. If you do this, Kutulu will be
        very very proud of you.

OTHER THINGS
------------

In JUDAS.H there are definitions for the config currently used as well as many
textstring arrays. They can be used to easily print soundcard & audiomode info,
as well as errors. Look at JP.C for a good(?) example.

        /*
         * Device, port, IRQ, DMA, mixrate & mixmode currently in use.
         * Don't change them!
         */
        extern unsigned judas_device;
        extern unsigned judas_port;
        extern unsigned judas_irq;
        extern unsigned judas_dma;
        extern unsigned judas_mixrate;
        extern unsigned char judas_mixmode;

        /*
         * Mixer names
         */
        extern char *judas_mixername[];

        /*
         * Sound device names
         */
        extern char *judas_devname[];

        /*
         * Mixmode names
         */
        extern char *judas_mixmodename[];

        /*
         * Interpolation mode names
         */
        extern char *judas_ipmodename[];

        /*
         * Error code
         */
        extern int judas_error;

        /*
         * Text strings for the error codes
         */
        extern char *judas_errortext[];

        /*
         * Clipping indicator (quality mixer only) indicates the hardest
         * clipping occurred. 0 = has not clipped, 64 = Very faint clipping,
         * 128 = half volume wouldn't clip, 255 = 1/4 volume wouldn't clip
         * Does not give values in range 1..63.
         */
        extern volatile char judas_clipped;

--------------
4. CONFIGURING
--------------

In JUDASCFG.H there are a couple of interesting defines. Look at it for an
explanation & instructions.

----------
5. HISTORY
----------

V2.0   - Original release

V2.01  - Corrected S3M arpeggio (if zero, must use previous infobyte)
       - Corrected XXX-fuckup in soundcard enviroment variables
       - Corrected player bounds checking in Anal Invaders

V2.02  - Removed error checking from sample loading in MODs & S3Ms, to make
         corrupted modules work. In the XM loader I didn't dare to do this,
         because XMs contain various headers in between sampledata, and if
         they're not in their correct position...BIG SHIT HAPPENS!
       - Added a check to MOD loader that loop end must not be beyond
         sample length. This makes those weird shittunes like DALLAS.MOD work
         without crashing.
       - Corrected the spelling of Vintersemestre in the hails section of this
         document.

V2.03  - Added a keyboard handler to Anal Invaders, now it's easier to play!
       - Added support for empty patterns in XMs
       - Corrected the "inaudible-bassline-bug" of HERRA.S3M
       - Corrected volumeslide "both-parameters-nonzero" bug in XMs & MODs

V2.04  - Added handling of mono S3Ms and "panning magic" to S3M loader
       - Added undocumented panning commands to MOD and S3M players, now songs
         by Khyron will be played correctly!
       - Corrected S3M vibrato "lose-old-speed-bug"
       - Removed some checking in S3M loader
       - Corrected S3M frequency limits
       - Corrected S3M portamento up & down to use the same previous value
       - Scaled default pannings by 50% to prevent "earache"
       - Corrected S3M sample offset command to remember old value
       - Corrected XM global volume limits
       - Corrected MOD instrumentnumber/note alone bug
       - XM toneportamento must not change the instrument/samplenumber
         (a very stupid bug!)

V2.04y - Added selectable quality mixer and made required changes to the
         example programs
       - Added simple file writer

V2.05  - Corrected behaviour of retrig command in both XMs & MODs. Now retrig
         in XMs behaves just like in FT2 and in MODs like in Cubic Player
         (confusing!!!) After doing this, Cadaver is on the verge of total mind
         meltdown...
       - In MODs, a notedelay without a note mustn't play anything.
       - There might be 256 patterns in a mod, so mt_patterns variable must
         be bigger than 8 bits.
       - If GUS DMA-transfer cache fills up (some IRQ missed maybe) then cache
         is cleared manually. Sometimes sound stuck under WIN95, hopefully
         this corrects it!
       - Made JPLAY's default volume louder
       - Added some little things to the documentation
       - December has begun, cold frostvoid everywhere...
       - This will probably be the last version of Judas of which you can get
         Cadaver's version also.

V2.06y - Corrected XM & MOD handling of two pattern breaks on the same row.
         The problem still exists in the S3M player, because the channels
         should be sorted somehow accordingly to the default panning, and
         that's just too much shit for us!
       - Corrected XM, MOD & S3M track sine vibrato phase. (Hi Dagoon!)
       - XM & S3M note cut now works!
       - Made song end autodetection & looping control. Now you can set how
         many times the song plays before it stops by itself! Infinite looping
         is also possible.
       - Killed a bug that caused S3M player to sometimes crash at song end.
       - Increased the number of volume levels. (Audible with Quality Mixer
         only!) If you have used the sample playing functions of older versions
         of Judas, and want to use the latest version of Judas in your program,
         just multiply the volumes that go to judas_playsample() by 256.
       - Renamed JPLAY to JP to make it faster to type. Retrain your fingers!
       - Made the clip removing routines detect and handle sudden VM_OFFs! Now
         S3Ms with noteoffs sound smoother.
       - Added channel volume meters. Don't worry! They won't slow down the
         mixing at all when you don't use them!
       - Rewrote the file writer. Now it writes WAV files!
       - Made Judas WATCOM 11.0 compatible. (Structure packing was the problem)
       - Optimized mixing routines a bit.
       - Added quality mixer clipping indicator.
       - Added song rewinding and forwarding.
       - Released at the Abduction'98 demoparty!!!

V2.07a - Added pure DOS support for INTEL ICH0/ICH/ICH2/ICH3 and compatible
         AC97 integrated audio codecs (judasac.h and judasac.inc defines).
       - Added low DOS memory functions dos_malloc() and dos_free()
       - Advanced error tracking and debug functions for AC97 support code
         located in judas.c file.
       - Added a separate ichinit DOS utility for AC97 codecs.
         (Black Spider's February 2003 release).

V2.07y - Improved pure DOS support for INTEL ICH0/ICH/ICH2/ICH3 and compatible
         AC97 integrated audio codecs (judasac.h and judasac.inc defines).
       - Added ICH4 controller support.
       - Example programs now use the WDOSX DOS extender instead of PMODE/W
         (Black Spider's March 2003 release).

V2.08y - Improved pure DOS support for INTEL ICH4 AC97 integrated audio codecs.
         (Black Spider's March 2003 release).

V2.09a - Added support for SIS7012 - now it works :)

V2.09b - Corrected ICH4 and above chips support - now it works :)

V2.09c - Dev version (all tasm code converted to nasm)

V2.09d - Dev version with full support for WATCOM C++ and DJGPP near pointers

V2.09e - All AC97 audio devices now use memory mapped IO if available

V2.09f - WAV library builder added to JUDAS. See JDSWAV.ZIP package for details

v2.10a - High Definition Audio codecs support added - supported up to ICH10 (!)

v2.10b - judas_setwavlib function added

v2.10c - High Definition Audio codecs support update for intel 5 and 6 series test by RayeR

v2.10d - High Definition Audio codecs support update for intel 7 series test by RayeR

--------------
6. FINAL WORDS
--------------

JUDAS was written because Cadaver got bored of digital sound systems which were
full of restrictions (register and you'll get sound fx support!), didn't
interpolate, were made in Pascal or refused to work if they ran out of
Ultrasound memory. It was programmed in the realms of northern icy void
from March 1997 onwards.

Hail from Cadaver to these supreme artists: Impaled Nazarene, Mustan Kuun
Lapset, Vintersemestre, Horna, Troll, Burzum, Immortal, Mercyful Fate.

Hail to all perverts who know us!

email Cadaver: loorni@student.oulu.fi
Cadaver's homepage: http://www.student.oulu.fi/~loorni

email Yehar: ollinie@freenet.hut.fi
Absorb free ambient techno: http://www.sublevel3.org

NOTE: Judas is developed as a sound system, not as a player. So, don't ask us
to program Cubic-Player-like fileselectors or such. It's YOUR job, right? :)

------------------------------------------------------------------------------

NOTE: Judas sound system for DOS is no longer developed. However this
sound engine is now integrated in the BME package, which is available
at Cadaver's web site. The last original 2.06y version of Judas for DOS
was released in 1998.
This version (2.10b - Black Spider's release) only adds AC97/HDA
codecs support for the DOS version of Judas sound system. There are
also a few docs changes. Just enough to explain AC97/HDA behaviour.
Many thanks to Matthias Goeke for providing the ICH5 as well as the HDA
mainboard for testing purposes.

Greetz to all those who release their coding stuff with source code.
Hail to the ones who know me, and of coz to Cadaver and Yehar
for makeing such a good DOS sound engine.

email to Piotr Ulaszewski (aka BSpider) for AC97/HDA support problems:
(don't forget to specify the type of hardware you use :-)
piotrkn22@poczta.onet.pl
http://www.piotrkn22.republika.pl
