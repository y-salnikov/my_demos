#include <SDL.h>
#include "sound.h"

int main(void)
{
//Initialize SDL 
    if( SDL_Init( SDL_INIT_AUDIO ) < 0 ) 
    { 
	printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() ); 
    }
    sound_init();
    
    sound_done();
 //Quit SDL subsystems 
    SDL_Quit();
    return 0;
}
