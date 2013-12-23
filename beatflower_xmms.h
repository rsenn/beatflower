/* BeatFlower - a XMMS Visualization Plug-In

   Copyright (C) 2002, Roman Senn.
   Email: <smoli@paranoya.ch>
   Project Homepage: <http://www.blah.ch/beatflower>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.
   If not, write to the Free Software Foundation,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   $Id: beatflower.c,v 1.4 2004/05/18 23:42:16 smoli Exp $ */

#ifndef BEATFLOWER_XMMS_H__
#define BEATFLOWER_XMMS_H__ 1

#include <gtk/gtk.h>

extern GtkWidget *beatflower_xmms_settings_win;

void beatflower_xmms_settings(void);

void beatflower_xmms_config_load(beatflower_config_t *cfg);
void beatflower_xmms_config_save(beatflower_config_t *cfg);

#endif // BEATFLOWER_XMMS_H__ 1
