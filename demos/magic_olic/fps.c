/***************************************************************************
 *   Copyright (C) 2007 by Jarik   *
 *   n/a   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "SDL.h"
#include "fps.h"

Uint32 FPS;
int current_fps=60;
void *my_callback_param;
Uint32 my_callbackfunc (Uint32 interval, void *param);
SDL_TimerID my_timer_id;
extern char str[128];
extern char str_flag;

Uint32 my_callbackfunc (Uint32 interval, void *param)
{
  sprintf (str,"Magic Olik       FPS: %d ", FPS);
//  SDL_WM_SetCaption(str,NULL);
  str_flag=1;
  current_fps=FPS;
  FPS = 0;
  return interval;
}

void
init_fps (void)
{
  FPS = 0;
  my_timer_id = SDL_AddTimer (1000, my_callbackfunc, my_callback_param);
}

void
frame_count (void)
{
  FPS++;
}

int get_fps(void)
{
    return current_fps;
}
