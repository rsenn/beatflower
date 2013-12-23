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

static VisPlugin beatflower_plugin =
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
VisPlugin *
get_vplugin_info(void)
{
  return &beatflower_plugin;
}


/****************************** XMMS callbacks ********************************/
static void beatflower_xmms_log(const char *format, ...)
{
  va_list args;
  va_start(args, format);

  g_logv(G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE, format, args);
  va_end(args);
}

/* initialize values and start the beatflower beatflower_thread */
void
beatflower_xmms_init(void)
{
  g_message("%s:", __PRETTY_FUNCTION__);

  pthread_mutex_lock(&beatflower_config_mutex);

  if(!beatflower_config_loaded)
    beatflower_xmms_config_load(&beatflower_config);

  pthread_mutex_unlock(&beatflower_config_mutex);

  beatflower_log = &beatflower_xmms_log;

  beatflower_start();
}

void
beatflower_xmms_cleanup(void)
{
  pthread_mutex_lock(&beatflower_status_mutex);
  beatflower_finished = TRUE;
  beatflower_playing = FALSE;
  pthread_mutex_unlock(&beatflower_status_mutex);

  if(beatflower_thread)
    pthread_join(beatflower_thread, NULL);
}

void
beatflower_xmms_playback_start(void)
{
  pthread_mutex_lock(&beatflower_status_mutex);
  beatflower_playing = TRUE;
  pthread_mutex_unlock(&beatflower_status_mutex);
}

void
beatflower_xmms_playback_stop(void)
{
  pthread_mutex_lock(&beatflower_status_mutex);
  beatflower_playing = FALSE;
  pthread_mutex_unlock(&beatflower_status_mutex);
}

void 
beatflower_xmms_render_pcm(short data[2][512])
{
  pthread_mutex_lock(&beatflower_data_mutex);
  memcpy(beatflower_pcm_data, data, 1024);
  pthread_mutex_unlock(&beatflower_data_mutex);
}

void 
beatflower_xmms_render_freq(short data[2][256])
{
  pthread_mutex_lock(&beatflower_data_mutex);
  memcpy(beatflower_freq_data, data, 512);
  pthread_mutex_unlock(&beatflower_data_mutex);
}

void
beatflower_xmms_about(void)
{
}


