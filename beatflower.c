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

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <xmms/plugin.h>
#include <xmms/configfile.h>
#include <gtk/gtk.h>

#include "beatflower.h"
#include "beatflower_renderer.h"

/************************************* Constants ******************************************/

#define M_2PI 6.2831853071795864769252867665590058

/* uh.. endian shit */
#define RED_MASK   0xff0000
#define GREEN_MASK 0x00ff00
#define BLUE_MASK  0x0000ff
#define RED_SHIFT   16
#define GREEN_SHIFT  8
#define BLUE_SHIFT   0

/************************************* Variables ****************************************/

config_t     beatflower_config;
//config_t     beatflower_newconfig;
pthread_mutex_t beatflower_status_mutex  = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t beatflower_data_mutex    = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t beatflower_config_mutex  = PTHREAD_MUTEX_INITIALIZER;
bool         beatflower_config_loaded = FALSE;
/*static bool         reinit        = FALSE;*/
bool         beatflower_playing;
bool         beatflower_finished;            /* some status variables... */
bool         beatflower_reset;
int16_t       beatflower_pcm_data[2][512];    /* 2 channel pcm and freq data */
int16_t       beatflower_freq_data[2][256];

beatflower_state_t beatflower;

/********************************* Functions *****************************************/

void         config_set_defaults(config_t *beatflower_config);


void config_set_defaults(config_t *cfg)
{
  g_message("%s:", __PRETTY_FUNCTION__);

  cfg->width  = 320;
  cfg->height = 320;
  cfg->fullscreen = FALSE;
  cfg->color_mode = COLOR_3_GRADIENT;
  cfg->color1 = 0x800000;
  cfg->color2 = 0xff0000;
  cfg->color3 = 0xffff00;
  cfg->draw_mode = DRAW_CIRCLE;
  cfg->samples_mode = SAMPLES_512;
  cfg->amplification_mode = AMP_FULL;
  cfg->offset_mode = OFFSET_NULL;
  cfg->blur = TRUE;
  cfg->decay = 2;
  cfg->factor = 0.95;
  cfg->angle = 1.5;
  cfg->zoombeat = FALSE;
  cfg->rotatebeat = FALSE;
}


int beatflower_scope_amplification(int value)
{
  register int v = value;

  if(beatflower.amp_mode == AMP_HALF)
  {
    return v / 2;
  }

  if(beatflower.amp_mode == AMP_DOUBLE)
  {
    return v * 2;
  }

  return v;
}

int beatflower_scope_offset(int value)
{
  register int v = beatflower_scope_amplification(value);

  v += beatflower.offset_mode;

  if(v >  32767)
  {
    return  32767;
  }

  if(v < -32768)
  {
    return -32768;
  }

  return v;
}

void find_color(int16_t data[2][256])
{
  uint32_t value = 0;
  int32_t x;
  int32_t count = 0;
  int32_t i;

  for(i = 0; i < 64; i++)
  {
    x = (data[0][i] + data[1][i]) >> 1;

    if(x > 0)
    {
      count++;
      value += x;
    }
  }

  if(count)
  {
    value /= (count + 10);

    if(value > 511)
    {
      value = 511;
    }

    beatflower.color = beatflower.color_table[value];
  }

  else
  {
    beatflower.color = beatflower.color_table[0];
  }
}

bool beatflower_check_finished()
{
  bool ret;

  pthread_mutex_lock(&beatflower_status_mutex);

  ret = beatflower_finished;

  if(beatflower_reset)
  {
    beatflower_reset = FALSE;
    beatflower_renderer_sdl.init();
  }

  pthread_mutex_unlock(&beatflower_status_mutex);

  return ret;
}

 bool beatflower_check_playing()
{
  bool ret;

  pthread_mutex_lock(&beatflower_status_mutex);
  ret = beatflower_playing;
  pthread_mutex_unlock(&beatflower_status_mutex);

  return ret;
}


