#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "SDL.h"
#include "sound.h"
#include "hsc.h"


int main(int argc, char *argv[])
{
    uint8_t *hsc;
    uint32_t lp;
    hsc=hsc_load("TECHNO.HSC");
    if(hsc==NULL)
    {
        printf("lostship.hsc not found. \n");
        return 3;
    }
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER);
    sound_init();
    start_opl_emulation();

    hsc_play(hsc,1);
    while(hsc_get_status())
    {
        if(lp!=hsc_get_pattpos())
        {
            printf("status - %d; pos - %d; pattpos - %d                       \r",hsc_get_status(),hsc_get_pos(),hsc_get_pattpos());
            lp=hsc_get_pattpos();
            fflush(stdout);
            usleep(100000);
        }
    }
    
    sound_end();
    return 0;
}

