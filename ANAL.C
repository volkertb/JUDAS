/*
 * ANAL INVADERS megagame!
 *
 * Shows how to use music & sound effects together, make a sound setup menu
 * and how to pan the sfx according to the "location" the sound comes from.
 *
 * Like JP, this sets up the signal handlers & uses IRQ0 to update the sound.
 *
 * Channels used:
 * 0-3  Music
 * 4    Menumove & menuselect effect
 * 5-6  Fart at the exit
 * 7    Player shot effect
 * 8    Enemy shot effect
 * 9    Explosion effect
 *
 * This code is shitty!
 *
 * In V2.01  the player bounds checking was corrected!
 *    V2.03  a custom keyboard handler was added!
 *    V2.04y mixer changing is supported and ESC moves bacwards in menus
 *    V2.06y support for more volume levels was added
 */

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <mem.h>
#include <dos.h>
#ifdef __DJGPP__
#include <unistd.h>
#include <go32.h>
#include <dpmi.h>
#include <sys/nearptr.h>
#else
#define __djgpp_conventional_base 0
#endif
#include "judas.h"
#include "timer.h"
#include "kbd.h"

#define MAX_OBJECTS 250
#define MAX_STARS 50

#define OT_NONE 0
#define OT_PLAYER 1
#define OT_ENEMY 2
#define OT_PLAYERSHOT 3
#define OT_ENEMYSHOT 4
#define OT_EXPLOSION 5

typedef struct
{
        int x;
        int y;
        int sx;
        int sy;
        int time;
        int type;
} OBJECT;

typedef struct
{
        int x;
        int y;
        int sx;
} STAR;

unsigned mixratetable[] =
{
        5513,
        8269,
        11025,
        16538,
        22050,
        33075,
        44100
};

char *onofftext[] =
{
        "OFF",
        "ON",
};

char *mainmenutext[] =
{
        "Start The Massacre",
        "Setup Sound",
        "Quit To The Realms Of MS-DOS"
};

unsigned char colortable[] =
{
        0, 8, 5, 9, 7, 14, 15
};

unsigned char expshapetable[] =
{
         176, 176, 176, 177, 177, 177, 178, 219
};

unsigned char expcolortable[] =
{
        4, 4, 6, 6, 12, 12, 14, 15
};

char *scrbuffer;
char textbuffer[80];
int mixrate;
int mixer;
int interpolation;
int musicvol;
int sfxvol;
int mainmenuchoice = 0;
int soundmenuchoice = 0;

int score = 0;
int hiscore = 0;
int lives = 0;
int diff = 0;

SAMPLE *fart;
SAMPLE *explosion;
SAMPLE *plrshoot;
SAMPLE *menumove;
SAMPLE *menuselect;
OBJECT object[MAX_OBJECTS];
STAR star[MAX_STARS];

int main(void);
void mainmenu(void);
void playgame(void);
void initstars(void);
void movestars(void);
void drawstars(void);
void initobjects(void);
void drawobjects(void);
void initplayer(void);
void moveplayer(void);
void moveobjects(void);
void spawnenemy(void);
void checkplayershots(void);
void checkenemies(void);
void createexplosion(int x, int y, int pieces);
int scalepanning(int x);
void soundmenu(void);
void resetmode(void);
void loadconfig(void);
void saveconfig(void);
void fliptob8000(void);
void clearscreen(void);
void printtext(int x, int y, int color, const char *text);
void printtextc(int y, int color, const char *text);
static void handle_int(int a);

int main(void)
{
        mixrate = 4;
        mixer = FASTMIXER;
        interpolation = 0;
        musicvol = 50;
        sfxvol = 50;


        #ifdef __DJGPP__
        if (__djgpp_nearptr_enable() == 0) {
               printf("ERROR: Couldn't enable near pointers for DJGPP!\n");
               return 1;
        }
        /* Trick: Avoid re-setting DS selector limit on each memory allocation call */
            __djgpp_selector_limit = 0xffffffff;
        #endif

        atexit(judas_uninit);
        atexit(timer_uninit);
        atexit(kbd_uninit);
        signal(SIGINT, handle_int);
        #if defined(SIGBREAK)
        signal(SIGBREAK, handle_int);
        #endif

        srand(666 + *( (unsigned *) ((unsigned char *)0x46c + __djgpp_conventional_base) ));
        resetmode();
        scrbuffer = malloc(80 * 25 * 2);

        if (!scrbuffer)
        {
                printf("ERROR: No Memory For Virtual Screen!");
                return 666;
        }

        loadconfig();

        judas_config();
        if (!judas_init(mixratetable[mixrate], mixer, SIXTEENBIT | STEREO, interpolation))
        {
                printf("JUDAS ERROR: %s\n", judas_errortext[judas_error]);
                return 666;
        }

        kbd_init();
        timer_init(0x4280, judas_update);

        judas_setmusicmastervolume(4, musicvol);
        judas_setsfxmastervolume(4, sfxvol);

        judas_loadxm("tune1.xm");
        judas_playxm(0);

        fart = judas_loadwav("fart.wav");
        explosion = judas_loadwav("explsion.wav");
        plrshoot = judas_loadwav("plrshoot.wav");
        menumove = judas_loadwav("menumove.wav");
        menuselect = judas_loadwav("menusel.wav");
        mainmenu();

        judas_playsample(fart, 5, 22050, 64*256, LEFT);
        judas_playsample(fart, 6, 22050, 64*256, RIGHT);
        sleep(1);

        saveconfig();
        resetmode();
        return 0;
}

void resetmode(void)
{
        union REGS glenregs;

        glenregs.w.ax = 0x0003;
        int386(0x10, &glenregs, &glenregs);
}

void mainmenu(void)
{
        for(;;)
        {
                int count;
                int key;
                clearscreen();
                printtextc(6, 15, "-+* ANAL INVADERS And The DEFENDERS OF THE RECTUM *+-");
                printtextc(8, 14, "Megagame by Cadaver!");
                sprintf(textbuffer, "LAST SCORE:%06d  HISCORE:%06d", score, hiscore);
                printtextc(11, 10, textbuffer);
                for (count = 0; count < 3; count++)
                {
                        int color = 7;
                        if (mainmenuchoice == count) color = 15;

                        printtextc(15 + count, color, mainmenutext[count]);
                }
                fliptob8000();
                do
                {
                        key = kbd_getkey();
                } while (!key);
                switch(key)
                {
                        case KEY_ESC:
                        judas_playsample(menumove, 4, 22050, 64*256, MIDDLE);
                        mainmenuchoice = 3;

                        case KEY_UP:
                        judas_playsample(menumove, 4, 22050, 64*256, MIDDLE);
                        mainmenuchoice--;
                        if (mainmenuchoice < 0) mainmenuchoice = 2;
                        break;

                        case KEY_DOWN:
                        judas_playsample(menumove, 4, 22050, 64*256, MIDDLE);
                        mainmenuchoice++;
                        if (mainmenuchoice > 2) mainmenuchoice = 0;
                        break;

                        case KEY_SPACE:
                        case KEY_ENTER:
                        judas_playsample(menuselect, 4, 22050, 64*256, MIDDLE);
                        switch(mainmenuchoice)
                        {
                                case 0:
                                judas_loadxm("tune2.xm");
                                judas_playxm(0);
                                playgame();
                                judas_loadxm("tune1.xm");
                                judas_playxm(0);
                                break;

                                case 1:
                                soundmenu();
                                break;

                                case 2:
                                goto EXITTODOS;
                        }
                        break;
                }
        }
        EXITTODOS:
        return;
}

void soundmenu(void)
{
        int soundmenuchoice = 0;

        for(;;)
        {
                int count;
                int key;
                clearscreen();
                printtextc(6, 15, "-+* ANAL INVADERS And The DEFENDERS OF THE RECTUM *+-");
                printtextc(8, 14, "Megagame by Cadaver!");
                sprintf(textbuffer, "LAST SCORE:%06d  HISCORE:%06d", score, hiscore);
                printtextc(11, 10, textbuffer);
                for (count = 0; count < 6; count++)
                {
                        int color = 7;
                        if (soundmenuchoice == count) color = 15;

                        switch(count)
                        {
                                case 0:
                                sprintf(textbuffer, "Mixrate: %d", mixratetable[mixrate]);
                                break;

                                case 1:
                                sprintf(textbuffer, "Interpolation: %s", judas_ipmodename[interpolation]);
                                break;

                                case 2:
                                sprintf(textbuffer, "Music Volume: %d", musicvol);
                                break;

                                case 3:
                                sprintf(textbuffer, "Sfx Volume: %d", sfxvol);
                                break;

                                case 4:
                                sprintf(textbuffer, "Mixer: %s", judas_mixername[mixer]);
                                break;

                                case 5:
                                sprintf(textbuffer, "Exit To Previous Menu");
                                break;
                        }
                        printtextc(14 + count, color, textbuffer);
                }
                fliptob8000();
                do
                {
                        key = kbd_getkey();
                } while (!key);
                switch(key)
                {
                        case KEY_ESC:
                        judas_playsample(menuselect, 4, 22050, 64*256, MIDDLE);
                        goto EXITTOMAIN;

                        case KEY_UP:
                        judas_playsample(menumove, 4, 22050, 64*256, MIDDLE);
                        soundmenuchoice--;
                        if (soundmenuchoice < 0) soundmenuchoice = 5;
                        break;

                        case KEY_DOWN:
                        judas_playsample(menumove, 4, 22050, 64*256, MIDDLE);
                        soundmenuchoice++;
                        if (soundmenuchoice > 5) soundmenuchoice = 0;
                        break;

                        case KEY_LEFT:
                        judas_playsample(menumove, 4, 22050, 64*256, MIDDLE);
                        switch(soundmenuchoice)
                        {
                                case 0:
                                mixrate--;
                                if (mixrate < 0) mixrate = 0;
                                judas_init(mixratetable[mixrate], mixer, SIXTEENBIT | STEREO, interpolation);
                                break;

                                case 1:
                                interpolation = 0;
                                judas_init(mixratetable[mixrate], mixer, SIXTEENBIT | STEREO, interpolation);
                                break;

                                case 2:
                                if (musicvol > 10) {
                                        musicvol -= 10;
                                } else musicvol -= 1;
                                if (musicvol < 0) musicvol = 0;
                                judas_setmusicmastervolume(4, musicvol);
                                break;

                                case 3:
                                if (sfxvol > 10) {
                                        sfxvol -= 10;
                                } else sfxvol -= 1;
                                if (sfxvol < 0) sfxvol = 0;
                                judas_setsfxmastervolume(4, sfxvol);
                                break;

                                case 4:
                                mixer = FASTMIXER;
                                judas_init(mixratetable[mixrate], mixer,  SIXTEENBIT | STEREO, interpolation);
                                break;
                        }
                        break;

                        case KEY_RIGHT:
                        judas_playsample(menumove, 4, 22050, 64*256, MIDDLE);
                        switch(soundmenuchoice)
                        {
                                case 0:
                                mixrate++;
                                if (mixrate > 6) mixrate = 6;
                                judas_init(mixratetable[mixrate], mixer, SIXTEENBIT | STEREO, interpolation);
                                break;

                                case 1:
                                interpolation = 1;
                                judas_init(mixratetable[mixrate], mixer, SIXTEENBIT | STEREO, interpolation);
                                break;

                                case 2:
                                if (musicvol >= 10) {
                                        musicvol += 10;
                                } else musicvol += 1;
                                if (musicvol > 100) musicvol = 100;
                                judas_setmusicmastervolume(4, musicvol);
                                break;

                                case 3:
                                if (sfxvol >= 10) {
                                        sfxvol += 10;
                                } else sfxvol += 1;
                                if (sfxvol > 100) sfxvol = 100;
                                judas_setsfxmastervolume(4, sfxvol);
                                break;

                                case 4:
                                mixer = QUALITYMIXER;
                                judas_init(mixratetable[mixrate], mixer,  SIXTEENBIT | STEREO, interpolation);
                                break;
                        }
                        break;

                        case KEY_SPACE:
                        case KEY_ENTER:
                        switch(soundmenuchoice)
                        {
                                case 5:
                                judas_playsample(menuselect, 4, 22050, 64*256, MIDDLE);
                                goto EXITTOMAIN;
                        }
                        break;
                }
        }
        EXITTOMAIN:
        return;
}

void playgame(void)
{
        int delay = 0;
        int deathcounter = 0;
        int nextlife = 10000;

        diff = 0;
        lives = 3;
        score = 0;
        initstars();
        initobjects();
        initplayer();
        timer_count = 0;

        for (;;)
        {
                int speed;
                while (!timer_count)
                {
                }
                speed = timer_count;
                if (speed > 8) speed = 8;
                timer_count = 0;
                while (speed)
                {
                        movestars();
                        moveplayer();
                        moveobjects();
                        spawnenemy();
                        checkplayershots();
                        checkenemies();
                        if (score > hiscore) hiscore = score;
                        if (score >= nextlife)
                        {
                                judas_playsample(menuselect, 4, 22050, 64*256, MIDDLE);
                                lives++;
                                nextlife += 10000;
                        }
                        delay++;
                        if (delay == 1500)
                        {
                                delay = 0;
                                diff++;
                                if (diff > 50) diff = 50;
                        }
                        speed--;
                        if (!object[0].type)
                        {
                                deathcounter++;
                                if ((!lives) && (!(rand() % 7)))
                                {
                                        int x = rand() % 640;
                                        judas_playsample(explosion, 9, 22050, 64*256, scalepanning(x));
                                        createexplosion(x, rand() % 200, 10 + rand() % 20);
                                }
                                if (deathcounter >= 280)
                                {
                                        deathcounter = 0;
                                        if (lives)
                                        {
                                                initobjects();
                                                initplayer();
                                        }
                                        else
                                        {
                                                return;
                                        }
                                }
                        }
                }
                clearscreen();
                drawstars();
                drawobjects();
                sprintf(textbuffer, "SCORE:%06d", score);
                printtext(1, 23, 10, textbuffer);
                sprintf(textbuffer, "LIVES:%02d", lives);
                printtext(36, 23, 10, textbuffer);
                sprintf(textbuffer, "HISCORE:%06d", hiscore);
                printtext(65, 23, 10, textbuffer);
                if (!lives)
                {
                        printtextc(10, 15, "G A M E  O V E R");
                        printtextc(12, 12, "The Anal Invaders infiltrate Earth!");
                        printtextc(13, 12, "Everyone will suffer from hemorrhoids!");
                }
                fliptob8000();
                if (checkkey(KEY_ESC)) break;
        }
}

void initobjects(void)
{
        int count;
        OBJECT *optr = &object[1];

        for (count = MAX_OBJECTS - 1; count; count--)
        {
                optr->type = OT_NONE;
                optr++;
        }
}

void initplayer(void)
{
        OBJECT *optr = &object[0];
        optr->type = OT_PLAYER;
        optr->x = 32;
        optr->y = 100;
        optr->time = 0;
}

void moveplayer(void)
{
        OBJECT *optr = &object[0];
        if (optr->type != OT_PLAYER) return;
        if (optr->time) optr->time--;

        if (kt[KEY_UP])
        {
                optr->y -= 2;
                if (optr->y < 0) optr->y = 0;
        }

        if (kt[KEY_DOWN])
        {
                optr->y += 2;
                if (optr->y > 199) optr->y = 199;
        }

        if (kt[KEY_LEFT])
        {
                optr->x -= 4;
                if (optr->x < 0) optr->x = 0;
        }

        if (kt[KEY_RIGHT])
        {
                optr->x += 4;
                if (optr->x > 639) optr->x = 639;
        }

        if (kt[KEY_SPACE])
        {
                if (!optr->time)
                {
                        OBJECT *fptr = &object[1];
                        int f;
                        for (f = MAX_OBJECTS - 1; f; f--)
                        {
                                if (!fptr->type)
                                {
                                        judas_playsample(plrshoot, 7, 22050, 64*256, scalepanning(optr->x));
                                        optr->time = 12;
                                        fptr->type = OT_PLAYERSHOT;
                                        fptr->x = optr->x;
                                        fptr->y = optr->y;
                                        fptr->sx = 8;
                                        break;
                                }
                                fptr++;
                        }
                }
        }
}

void moveobjects(void)
{
        int count;
        OBJECT *optr = &object[1];

        for (count = MAX_OBJECTS - 1; count; count--)
        {
                switch(optr->type)
                {
                        case OT_NONE:
                        break;

                        case OT_PLAYERSHOT:
                        optr->x += optr->sx;
                        if (optr->x >= 640) optr->type = OT_NONE;
                        break;

                        case OT_ENEMYSHOT:
                        optr->x += optr->sx;
                        optr->y += optr->sy;
                        optr->time--;
                        if (optr->time <= 0) optr->type = OT_NONE;
                        break;

                        case OT_EXPLOSION:
                        optr->x += optr->sx;
                        optr->y += optr->sy;
                        optr->time--;
                        if (optr->time & 1) optr->sy++;
                        if (optr->time <= 0) optr->type = OT_NONE;
                        break;

                        case OT_ENEMY:
                        optr->x += optr->sx;
                        optr->y -= optr->sy;
                        optr->time--;
                        if (optr->time <= 0)
                        {
                                optr->sx = (rand() % 7) - 5;
                                optr->sy = (rand() % 5) - 2;
                                optr->time = (rand() % 20) + 5;
                                if ((rand() % 60) < diff)
                                {
                                        OBJECT *fptr = &object[1];
                                        int f;
                                        for (f = MAX_OBJECTS - 1; f; f--)
                                        {
                                                if (!fptr->type)
                                                {
                                                        judas_playsample(fart, 8, 44100, 40*256, scalepanning(optr->x));
                                                        fptr->type = OT_ENEMYSHOT;
                                                        fptr->x = optr->x;
                                                        fptr->y = optr->y;
                                                        fptr->sx = (object[0].x - optr->x) / 70;
                                                        fptr->sy = (object[0].y - optr->y) / 70;
                                                        fptr->time = 80;
                                                        break;
                                                }
                                                fptr++;
                                        }
                                }
                        }
                        if ((optr->x >= 680) || (optr->x < -40) || (optr->y >= 240) || (optr->y < -40))
                                optr->type = OT_NONE;
                        break;
                }
                optr++;
        }
}

void checkplayershots(void)
{
        int count;
        OBJECT *optr = &object[1];

        for (count = MAX_OBJECTS - 1; count; count--)
        {
                if (optr->type == OT_PLAYERSHOT)
                {
                        OBJECT *tptr = &object[1];
                        int tcount;
                        for (tcount = MAX_OBJECTS - 1; tcount; tcount--)
                        {
                                if (tptr->type == OT_ENEMY)
                                {
                                        if ((abs(optr->x - tptr->x) < 8) && (abs(optr->y - tptr->y) < 8))
                                        {
                                                judas_playsample(explosion, 9, 22050, 64*256, scalepanning(optr->x));
                                                optr->type = OT_NONE;
                                                tptr->type = OT_NONE;
                                                score += 100;
                                                createexplosion(optr->x, optr->y, 10);
                                                break;
                                        }
                                }
                                tptr++;
                        }
                }
                optr++;
        }
}

void checkenemies(void)
{
        int count;
        OBJECT *optr = &object[1];

        if (!object[0].type) return;
        for (count = MAX_OBJECTS - 1; count; count--)
        {
                if ((optr->type == OT_ENEMYSHOT) || (optr->type == OT_ENEMY))
                {
                        if ((abs(optr->x - object[0].x) < 8) && (abs(optr->y - object[0].y) < 8))
                        {
                                judas_playsample(explosion, 9, 22050, 64*256, scalepanning(optr->x));
                                lives--;
                                optr->type = OT_NONE;
                                object[0].type = OT_NONE;
                                createexplosion(optr->x, optr->y, 30);
                                break;
                        }
                }
                optr++;
        }
}

void createexplosion(int x, int y, int pieces)
{
        int count;
        OBJECT *optr = &object[1];

        for (count = MAX_OBJECTS - 1; count; count--)
        {
                if (!optr->type)
                {
                        optr->type = OT_EXPLOSION;
                        optr->x = x;
                        optr->y = y;
                        optr->sx = (rand() % 9) - 4;
                        optr->sy = -(rand() % 9);
                        optr->time = (rand() % 64) + 64;
                        pieces--;
                        if (!pieces) break;
                }
                optr++;
        }
}

void drawobjects(void)
{
        int count;
        OBJECT *optr = &object[0];

        for (count = MAX_OBJECTS; count; count--)
        {
                if ((optr->type) && (optr->x >= 0) && (optr->y >= 0) && (optr->x < 640) && (optr->y < 200))
                {
                        unsigned char *dptr = scrbuffer + (((optr->x >> 3) + (optr->y >> 3) * 80) << 1);
                        switch(optr->type)
                        {
                                case OT_PLAYER:
                                dptr[0] = 16;
                                dptr[1] = 10;
                                break;

                                case OT_PLAYERSHOT:
                                dptr[0] = 7;
                                dptr[1] = 15;
                                break;

                                case OT_ENEMYSHOT:
                                dptr[0] = 15;
                                dptr[1] = (rand() % 15) + 1;
                                break;

                                case OT_ENEMY:
                                dptr[0] = 2;
                                dptr[1] = 2;
                                break;

                                case OT_EXPLOSION:
                                dptr[0] = expshapetable[optr->time / 16];
                                dptr[1] = expcolortable[optr->time / 16];
                                break;
                        }
                }
                optr++;
        }
}

void spawnenemy(void)
{
        int count;
        OBJECT *optr = &object[1];

        if ((rand() % 666) < (650 - diff * 3)) return;
        for (count = MAX_OBJECTS - 1; count; count--)
        {
                if (!optr->type)
                {
                        optr->type = OT_ENEMY;
                        optr->x = 640;
                        optr->y = rand() % 200;
                        optr->sx = (rand() % 3) - 4;
                        optr->sy = (rand() % 5) - 2;
                        optr->time = (rand() % 20) + 5;
                        break;
                }
                optr++;
        }
}


void initstars(void)
{
        int count;
        STAR *sptr = &star[0];

        for (count = MAX_STARS; count; count--)
        {
                sptr->x = rand() % 640;
                sptr->y = rand() % 200;
                sptr->sx = (rand() % 6) + 1;
                sptr++;
        }
}

void movestars(void)
{
        int count;
        STAR *sptr = &star[0];

        for (count = MAX_STARS; count; count--)
        {
                sptr->x -= sptr->sx;
                if (sptr->x < 0)
                {
                        sptr->x = 640;
                        sptr->y = rand() % 200;
                        sptr->sx = (rand() % 6) + 1;
                }
                sptr++;
        }
}

void drawstars(void)
{
        int count;
        STAR *sptr = &star[0];

        for (count = MAX_STARS; count; count--)
        {
                if ((sptr->x >= 0) && (sptr->y >= 0) && (sptr->x < 640) && (sptr->y < 200))
                {
                        unsigned char *dptr = scrbuffer + (((sptr->x >> 3) + (sptr->y >> 3) * 80) << 1);
                        dptr[0] = 250;
                        dptr[1] = colortable[sptr->sx];
                }
                sptr++;
        }
}

/*
 * Scales the x-coordinates used in the game (0-639) on the 0-255 panning
 * scale.
 */
int scalepanning(int x)
{
        int panning = (x * 4) / 10;
        if (panning < 0) panning = 0;
        if (panning > 255) panning = 255;
        return panning;
}

void loadconfig(void)
{
        FILE *file = fopen("anal.cfg", "rt");
        if (!file) return;

        fscanf(file, "%d %d %d %d %d", &mixrate, &mixer, &interpolation, &musicvol, &sfxvol);
}

void saveconfig(void)
{
        FILE *file = fopen("anal.cfg", "wt");
        if (!file) return;

        fprintf(file, "%d %d %d %d %d", mixrate, mixer, interpolation, musicvol, sfxvol);
}

void clearscreen(void)
{
        memset(scrbuffer, 0, 80 * 25 * 2);
}

void printtext(int x, int y, int color, const char *text)
{
        unsigned char *dptr = scrbuffer + ((x + y * 80) << 1);

        while (*text)
        {
                *dptr++ = *text++;
                *dptr++ = color;
        }
}

void printtextc(int y, int color, const char *text)
{
        int x = (80 - strlen(text)) / 2;
        unsigned char *dptr = scrbuffer + ((x + y * 80) << 1);

        while (*text)
        {
                *dptr++ = *text++;
                *dptr++ = color;
        }
}

void fliptob8000(void)
{
        memcpy((char *)0xb8000 + __djgpp_conventional_base, scrbuffer, 80 * 25 * 2);
}

static void handle_int(int a)
{
        resetmode(); /* To prevent user panic when no cursor is seen! */
        exit(0); /* Atexit functions will be called! */
}
