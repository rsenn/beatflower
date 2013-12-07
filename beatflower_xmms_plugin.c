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
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; see the file COPYING.
 If not, write to the Free Software Foundation,
 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

 $Id: beatflower.c,v 1.4 2004/05/18 23:42:16 smoli Exp $ */

#include <string.h>
#include <gtk/gtk.h>
#include <xmms/plugin.h>
#include <xmms/configfile.h>

#include "beatflower.h"
#include "beatflower_xmms.h"

static void beatflower_xmms_init();
static void beatflower_xmms_cleanup();
static void beatflower_xmms_about();
static void beatflower_xmms_playback_start();
static void beatflower_xmms_playback_stop();
static void beatflower_xmms_render_pcm(short data[2][512]);
static void beatflower_xmms_render_freq(short data[2][256]);
static void beatflower_xmms_about();

static VisPlugin beatflower =
{
  NULL,
  NULL,
  0, /* session id, initialized by xmms */
  "beatflower "VERSION,
  2,
  2,
  beatflower_xmms_init,
  beatflower_xmms_cleanup,
  beatflower_xmms_about,
  beatflower_xmms_settings,
  NULL, /* disable */
  beatflower_xmms_playback_start,
  beatflower_xmms_playback_stop,
  beatflower_xmms_render_pcm,
  beatflower_xmms_render_freq
};

/* the only non-static thing.... */
VisPlugin *get_vplugin_info()
{
  return &beatflower;
}


/****************************** XMMS callbacks ********************************/

/* initialize values and start the beatflower beatflower_thread */
void beatflower_xmms_init()
{
  //pthread_attr_t attr;

  g_message("%s:", __PRETTY_FUNCTION__);

  //srand(time(NULL));

  SDL_LockMutex(beatflower_config_mutex);

  if(!beatflower_config_loaded)
  {
    beatflower_xmms_config_load(&beatflower_config);
  }

  SDL_UnlockMutex(beatflower_config_mutex);


  beatflower_finished = FALSE;
  beatflower_playing = FALSE;
  beatflower_reset = FALSE;

  beatflower_thread = SDL_CreateThread((void *)beatflower_thread_function, NULL);
}

void beatflower_xmms_cleanup()
{
  g_message("%s:", __PRETTY_FUNCTION__);

  SDL_LockMutex(beatflower_status_mutex);
  beatflower_finished = TRUE;
  beatflower_playing = FALSE;
  SDL_UnlockMutex(beatflower_status_mutex);

  if(beatflower_thread)
  {
    SDL_WaitThread(beatflower_thread, NULL);
  }
}

void beatflower_xmms_playback_start()
{
  g_message("%s:", __PRETTY_FUNCTION__);

  SDL_LockMutex(beatflower_status_mutex);
  beatflower_playing = TRUE;
  SDL_UnlockMutex(beatflower_status_mutex);
}

void beatflower_xmms_playback_stop()
{
  g_message("%s:", __PRETTY_FUNCTION__);

  SDL_LockMutex(beatflower_status_mutex);
  beatflower_playing = FALSE;
  SDL_UnlockMutex(beatflower_status_mutex);
}

void beatflower_xmms_render_pcm(short data[2][512])
{
  SDL_LockMutex(beatflower_data_mutex);
  memcpy(beatflower_pcm_data, data, 1024);
  SDL_UnlockMutex(beatflower_data_mutex);
}

void beatflower_xmms_render_freq(short data[2][256])
{
  SDL_LockMutex(beatflower_data_mutex);
  memcpy(beatflower_freq_data, data, 512);
  SDL_UnlockMutex(beatflower_data_mutex);
}

void beatflower_xmms_about()
{
  g_message("%s:", __PRETTY_FUNCTION__);

}


