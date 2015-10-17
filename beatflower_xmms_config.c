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

#include <xmms/configfile.h>

#include "beatflower.h"
#include "beatflower_xmms.h"

void 
beatflower_xmms_config_load(beatflower_config_t *cfg)
{
  ConfigFile *f;
  gchar *filename;

//  g_message("%s:", __PRETTY_FUNCTION__);

  filename = g_strconcat(g_get_home_dir(), "/.xmms/config", NULL);

  if(!(f = xmms_cfg_open_file(filename)))   // && !(f = xmms_cfg_new()))
  {
    g_warning("Could not open XMMS beatflower config '%s', settings defaults...", filename);
    beatflower_config_default(cfg);
    // beatflower_xmms_config_save(cfg);
  }

  else
  {
    xmms_cfg_read_boolean(f, PACKAGE, "fullscreen", &cfg->fullscreen);
    xmms_cfg_read_int(f, PACKAGE, "width", &cfg->width);
    xmms_cfg_read_int(f, PACKAGE, "height", &cfg->height);
    xmms_cfg_read_int(f, PACKAGE, "colormode", (int *)&cfg->color_mode);
    xmms_cfg_read_int(f, PACKAGE, "color1", &cfg->color1);
    xmms_cfg_read_int(f, PACKAGE, "color2", &cfg->color2);
    xmms_cfg_read_int(f, PACKAGE, "color3", &cfg->color3);
    xmms_cfg_read_int(f, PACKAGE, "drawmode", (int *)&cfg->draw_mode);
    xmms_cfg_read_int(f, PACKAGE, "samplesmode", (int *)&cfg->samples_mode);
    xmms_cfg_read_int(f, PACKAGE, "ampmode", (int *)&cfg->amplification_mode);
    xmms_cfg_read_int(f, PACKAGE, "offsetmode", (int *)&cfg->offset_mode);
    xmms_cfg_read_int(f, PACKAGE, "decay", &cfg->decay);
    xmms_cfg_read_double(f, PACKAGE, "factor", &cfg->factor);
    xmms_cfg_read_double(f, PACKAGE, "angle", &cfg->angle);
    xmms_cfg_read_boolean(f, PACKAGE, "blur", &cfg->blur);
    xmms_cfg_read_boolean(f, PACKAGE, "zoombeat", &cfg->zoombeat);
    xmms_cfg_read_boolean(f, PACKAGE, "rotatebeat", &cfg->rotatebeat);

    xmms_cfg_free(f);
  }

  g_free(filename);

  beatflower_config_loaded = TRUE;
}

void 
beatflower_xmms_config_save(beatflower_config_t *cfg)
{
  ConfigFile *f;

  gchar *filename;
//  g_message("%s:", __PRETTY_FUNCTION__);

  filename = g_strconcat(g_get_home_dir(), "/.xmms/config", NULL);

  if(!(f = xmms_cfg_open_file(filename)))
  {
    f = xmms_cfg_new();
  }

  xmms_cfg_write_boolean(f, PACKAGE, "fullscreen", cfg->fullscreen);
  xmms_cfg_write_int(f, PACKAGE, "width", cfg->width);
  xmms_cfg_write_int(f, PACKAGE, "height", cfg->height);
  xmms_cfg_write_int(f, PACKAGE, "colormode", cfg->color_mode);
  xmms_cfg_write_int(f, PACKAGE, "color1", cfg->color1);
  xmms_cfg_write_int(f, PACKAGE, "color2", cfg->color2);
  xmms_cfg_write_int(f, PACKAGE, "color3", cfg->color3);
  xmms_cfg_write_int(f, PACKAGE, "drawmode", (int)cfg->draw_mode);
  xmms_cfg_write_int(f, PACKAGE, "samplesmode", (int)cfg->samples_mode);
  xmms_cfg_write_int(f, PACKAGE, "ampmode", (int)cfg->amplification_mode);
  xmms_cfg_write_int(f, PACKAGE, "offsetmode", (int)cfg->offset_mode);
  xmms_cfg_write_boolean(f, PACKAGE, "blur", cfg->blur);
  xmms_cfg_write_int(f, PACKAGE, "decay", cfg->decay);
  xmms_cfg_write_double(f, PACKAGE, "factor", cfg->factor);
  xmms_cfg_write_double(f, PACKAGE, "angle", cfg->angle);
  xmms_cfg_write_boolean(f, PACKAGE, "zoombeat", cfg->zoombeat);
  xmms_cfg_write_boolean(f, PACKAGE, "rotatebeat", cfg->rotatebeat);

  g_message("Writing XMMS beatflower config to '%s' ...", filename);

  xmms_cfg_write_file(f, filename);
  xmms_cfg_free(f);

  g_free(filename);

/*  if(beatflower_xmms_settings_win)
  {
    gtk_widget_destroy(beatflower_xmms_settings_win);
    beatflower_xmms_settings_win = NULL;
  }*/
}
