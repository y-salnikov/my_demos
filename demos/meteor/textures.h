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

void texture_init (void);
void endfire (void);
Uint8 *bufer (void);
Uint8 *bufer2 (void);
Uint8 *bufer3 (void);
void view_bufer_no_sdl (Uint8 * dst_buf, Uint8 * src_buf);
//void anim_stop (void);
//void anim_start (void);
void fire_update(void);

void bind_fire_texture(void);
void update_texture(void);
void bind_bkg_texture(void);
void update_bkg_texture(void);
void bind_meteor_texture(void);
void meteor_update_texture(void);
void spectr_texture_update(void);

