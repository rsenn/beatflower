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
#include <SDL.h>

/************************************* Constants ******************************************/

#define M_2PI 6.2831853071795864769252867665590058

/* uh.. endian shit */
#define RED_MASK   0xff0000
#define GREEN_MASK 0x00ff00
#define BLUE_MASK  0x0000ff
#define RED_SHIFT   16
#define GREEN_SHIFT  8
#define BLUE_SHIFT   0

/*************************************** Types ********************************************/

typedef int bool;

/* configuration structure */
typedef struct config_s {
  bool         fullscreen;
  int          width;
  int          height;
  unsigned int color1;
  unsigned int color2;
  unsigned int color3;
  bool         blur;       /* do blur */
  int          decay;      /* decay value */
  double       factor;     /* zoom factor */
  double       angle;      /* rotation angle */
  bool         zoombeat;   /* zoom by beat */
  bool         rotatebeat; /* rotate by beat */
  enum { COLOR_2_GRADIENT = 0, COLOR_3_GRADIENT = 1, COLOR_RANDOM = 2, COLOR_FREQ  = 3 }                  color_mode; 
  enum { DRAW_DOTS        = 0, DRAW_BALLS       = 1, DRAW_CIRCLE  = 2, DRAW_LINES  = 3 }                  draw_mode;
  enum { SAMPLES_32       = 0, SAMPLES_64       = 1, SAMPLES_128  = 2, SAMPLES_256 = 3, SAMPLES_512 = 4 } samples_mode;
  enum { AMP_HALF         = 0, AMP_FULL         = 1, AMP_DOUBLE   = 2 }                                   amplification_mode;
  enum { OFFSET_MINUS     = 0, OFFSET_NULL      = 1, OFFSET_PLUS  = 2 }                                   offset_mode;
} config_t;

/************************************* Variables ****************************************/

static VisPlugin    beatflower;
static config_t     config;
static config_t     newconfig;
pthread_mutex_t     status_mutex  = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t     data_mutex    = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t     config_mutex  = PTHREAD_MUTEX_INITIALIZER;
static bool         beatflower_config_loaded = FALSE;
/*static bool         reinit        = FALSE;*/
static bool         beatflower_playing;
static bool         beatflower_finished;            /* some status variables... */
static bool         reset;
static pthread_t    thread;       /* thread that does the drawing */
static gint16       pcm_data[2][512];    /* 2 channel pcm and freq data */
static gint16       freq_data[2][256];
static SDL_Surface *screen;
static unsigned int color_table[512];
static unsigned int samples;
static void       (*scope)(short data[512]);
static Sint32       width;
static Sint32       height;
static Sint32       sine_table[512];
static Sint32       cosine_table[512];
static Sint32       radius;
static Uint32       color;
static Uint32       pitch;
static Uint32      *pixels;
static Uint32     **transform_table;
static double       factor;
static double       angle;
static bool         blur_enable;
static Uint32       color_mode;
static Uint32       amp_mode;
static Uint32       offset_mode;
static Uint32       color1;
static Uint32       color2;
static Uint32       color3;
static Uint32       decay;

/********************************* Functions *****************************************/

VisPlugin          *get_vplugin_info();
static void         config_set_defaults(config_t *config);
static void         config_load(config_t *cfg);
static void         config_save(config_t *cfg);

static void         dot_scope(short data[512]);
static void         ball_scope(short data[512]);
static void         circle_scope(short data[512]);
static void         line_scope(short data[512]);
static void         init_engine();
static void         create_color_table();
static void         line(Uint32 x1, Uint32 y1, Uint32 x2, Uint32 y2);
static void         zoom(Sint32 *x, Sint32 *y);
static void         rotate(Sint32 *x, Sint32 *y);
static void         init_transform();
static bool         check_finished();


void config_set_defaults(config_t *config)
{
  config->width  = 320;
  config->height = 320;
  config->fullscreen = FALSE;
  config->color_mode = COLOR_3_GRADIENT;
  config->color1 = 0x800000;
  config->color2 = 0xff0000;
  config->color3 = 0xffff00;
  config->draw_mode = DRAW_CIRCLE;
  config->samples_mode = SAMPLES_512;
  config->amplification_mode = AMP_FULL;
  config->offset_mode = OFFSET_NULL;
  config->blur = TRUE;
  config->decay = 2;
  config->factor = 0.95;
  config->angle = 1.5;
  config->zoombeat = FALSE;
  config->rotatebeat = FALSE;
}

void config_load(config_t *cfg)
{
  ConfigFile *f;
  char name[512];
  
  sprintf(name, "%s%s", g_get_home_dir(), "/.xmms/config");
  
  if(!(f = xmms_cfg_open_file(name)))
    config_set_defaults(cfg);
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
      xmms_cfg_read_boolean(f, PACKAGE, "blur", &cfg->blur);
      xmms_cfg_read_boolean(f, PACKAGE, "decay", &cfg->decay);
      xmms_cfg_read_double(f, PACKAGE, "factor", &cfg->factor);
      xmms_cfg_read_double(f, PACKAGE, "angle", &cfg->angle);
      xmms_cfg_read_boolean(f, PACKAGE, "zoombeat", &cfg->zoombeat);
      xmms_cfg_read_boolean(f, PACKAGE, "rotatebeat", &cfg->rotatebeat);
    }
  
  beatflower_config_loaded = TRUE;
}

void config_save(config_t *cfg)
{
  ConfigFile *f;
  char name[512];
  
  sprintf(name, "%s%s", g_get_home_dir(), "/.xmms/config");
  
  if(!(f = xmms_cfg_open_file(name)))
    f = xmms_cfg_new();
  
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
  xmms_cfg_write_boolean(f, PACKAGE, "decay", cfg->decay);
  xmms_cfg_write_double(f, PACKAGE, "factor", cfg->factor);
  xmms_cfg_write_double(f, PACKAGE, "angle", cfg->angle);
  xmms_cfg_write_boolean(f, PACKAGE, "zoombeat", cfg->zoombeat);
  xmms_cfg_write_boolean(f, PACKAGE, "rotatebeat", cfg->rotatebeat);

  xmms_cfg_write_file(f, name);
  xmms_cfg_free(f);
}

static void create_color_table()
{
  if(color_mode == COLOR_2_GRADIENT)
    {
      register int red, green, blue;
      register int dr, dg, db;
      int i;
      
      red   = (color1 >> RED_SHIFT)   & 0xff;
      green = (color1 >> GREEN_SHIFT) & 0xff;
      blue  = (color1 >> BLUE_SHIFT)  & 0xff;
      
      dr = ((color2 >> RED_SHIFT)   & 0xff) - red;
      dg = ((color2 >> GREEN_SHIFT) & 0xff) - green;
      db = ((color2 >> BLUE_SHIFT)  & 0xff) - blue;
      
      for(i = 0; i < 512; i++)
        color_table[i] = ((((red   + ((i * dr) >> 9)) << RED_SHIFT)   & RED_MASK) |
                          (((green + ((i * dg) >> 9)) << GREEN_SHIFT) & GREEN_MASK) |
                          (((blue  + ((i * db) >> 9)) << BLUE_SHIFT)  & BLUE_MASK));
    }
  else if(color_mode == COLOR_3_GRADIENT)
    {
      register int red, green, blue;
      register int dr, dg, db;
      int i;
      
      red   = (color1 >> RED_SHIFT)   & 0xff;
      green = (color1 >> GREEN_SHIFT) & 0xff;
      blue  = (color1 >> BLUE_SHIFT)  & 0xff;
      
      dr = ((color2 >> RED_SHIFT)   & 0xff) - red;
      dg = ((color2 >> GREEN_SHIFT) & 0xff) - green;
      db = ((color2 >> BLUE_SHIFT)  & 0xff) - blue;
      
      for(i = 0; i < 256; i++)
        color_table[i] = ((((red   + ((i * dr) >> 8)) << RED_SHIFT)   & RED_MASK) |
                          (((green + ((i * dg) >> 8)) << GREEN_SHIFT) & GREEN_MASK) |
                          (((blue  + ((i * db) >> 8)) << BLUE_SHIFT)  & BLUE_MASK));
      
      red   = (color2 >> RED_SHIFT)   & 0xff;
      green = (color2 >> GREEN_SHIFT) & 0xff;
      blue  = (color2 >> BLUE_SHIFT)  & 0xff;
      
      dr = ((color3 >> RED_SHIFT)   & 0xff) - red;
      dg = ((color3 >> GREEN_SHIFT) & 0xff) - green;
      db = ((color3 >> BLUE_SHIFT)  & 0xff) - blue;
      
      for(i = 0; i < 256; i++)
        color_table[i + 256] = ((((red   + ((i * dr) >> 8)) << RED_SHIFT)   & RED_MASK) |
                                (((green + ((i * dg) >> 8)) << GREEN_SHIFT) & GREEN_MASK) |
                                (((blue  + ((i * db) >> 8)) << BLUE_SHIFT)  & BLUE_MASK));
    }
}

/* draw a line from [x1|y1] to [x2,y2]

   when x1 = x2 and y1 = y2, nothing is drawed
*/
__inline__ static void line(Uint32 x1, Uint32 y1,
                            Uint32 x2, Uint32 y2)
{
  register Sint32 dx;
  register Sint32 dy;
  register Sint32 x;
  register Sint32 y;
  register Sint32 d;
  register char *p = (char *)pixels;
  register Sint32 xinc = 4;
  register Sint32 yinc = pitch;
  register Sint32 temp;

/*  if(x1 == x2 && y1 == y2) return;*/
  
  p += (x1 * xinc) + (y1 * yinc);

  /* swap so line can be calculated from left-top to right-bottom */
  if(y1 > y2)
    {
      temp = y2; y2 = y1; y1 = temp;
      yinc = -yinc;
    }
  if(x1 > x2)
    {
      temp = x2; x2 = x1; x1 = temp;
      xinc = -xinc;
    }
      
  /* caculate horizontal and vertical delta */
  dx = x2 - x1;
  dy = y2 - y1;
  
/*  if(dx == 1 && dy == 1)
    {
      *(Uint32 *)pixels = color;
      return;
    }*/
  
  /* swap all x and y stuff when dy > dx */
  if(dy > dx)
    {
      temp = yinc; yinc = xinc; xinc = temp;
      temp = y1; y1 = x1; x1 = temp;
      temp = y2; y2 = x2; x2 = temp;
      temp = dx; dx = dy; dy = temp;
    }

  /* the algorithm */
  y = y1;
  d = 2 * dy - dx;

  for(x = x1; x < x2; x++)
    {
      if(d < 0)
        d += 2 * dy;
      else
        {
          d += 2 * (dy - dx);
          y++;
          p += yinc;
        }
      p += xinc;
      *(Uint32 *)p = color;
    }
}

static void create_sine_tables()
{
  Sint32    i;
  double angle = 0;

  for(i = 0; i < samples; i++)
    {
      angle += M_2PI / samples;
      sine_table[i]   = sin(angle) * (radius - 1);
      cosine_table[i] = cos(angle) * (radius - 1);
    }
}


/* initialize the beatflower engine */
__inline__ static void init_engine()
{
  pthread_mutex_lock(&config_mutex);

  screen = SDL_SetVideoMode(config.width, config.height, 32, SDL_HWSURFACE);
  
  pixels = screen->pixels;
  pitch  = screen->pitch;
  width  = screen->w;
  height = screen->h;
  radius = (width > height ? height >> 1 : width >> 1);

  samples = (config.samples_mode == SAMPLES_32  ? 32  :
            (config.samples_mode == SAMPLES_64  ? 64  :
            (config.samples_mode == SAMPLES_128 ? 128 :
            (config.samples_mode == SAMPLES_256 ? 256 : 512))));
  
  scope = (config.draw_mode == DRAW_DOTS   ? &dot_scope    :
          (config.draw_mode == DRAW_BALLS  ? &ball_scope   :
          (config.draw_mode == DRAW_CIRCLE ? &circle_scope : &line_scope)));
  
  factor = config.factor;
  angle = config.angle * M_2PI / 360;
  blur_enable = config.blur;
  color_mode = config.color_mode;
  amp_mode = config.amplification_mode;
  offset_mode = (config.offset_mode == OFFSET_MINUS ? -32768 :
                (config.offset_mode == OFFSET_PLUS  ?  32768 : 0));
  color1 = config.color1;
  color2 = config.color2;
  color3 = config.color3;
  decay = config.decay;
  
  create_sine_tables();
  init_transform();
  create_color_table();
  
  pthread_mutex_unlock(&config_mutex);
}
  
__inline__ static Uint32 average(Uint32 c1, Uint32 c2,
                                 Uint32 c3, Uint32 c4)
{
  register Uint32 red, green, blue;
  
  red   = (((c1 >> RED_SHIFT)   & 0xff) + ((c2 >> RED_SHIFT)   & 0xff) +
           ((c3 >> RED_SHIFT)   & 0xff) + ((c4 >> RED_SHIFT)   & 0xff));
  green = (((c1 >> GREEN_SHIFT) & 0xff) + ((c2 >> GREEN_SHIFT) & 0xff) +
           ((c3 >> GREEN_SHIFT) & 0xff) + ((c4 >> GREEN_SHIFT) & 0xff));
  blue  = (((c1 >> BLUE_SHIFT)  & 0xff) + ((c2 >> BLUE_SHIFT)  & 0xff) +
           ((c3 >> BLUE_SHIFT)  & 0xff) + ((c4 >> BLUE_SHIFT)  & 0xff));
  
  if(red >= decay)   red -= decay;
  if(green >= decay) green -= decay;
  if(blue >= decay)  blue -= decay;
  
  red >>= 2;
  green >>= 2;
  blue >>= 2;
  
  return (red << RED_SHIFT | green << GREEN_SHIFT | blue << BLUE_SHIFT);
}

__inline__ static void zoom(register Sint32 *x, register Sint32 *y)
{
  Sint32 newx, newy;
  double bx, by;
  
  newx = *x - (width >> 1);
  newy = *y - (height >> 1);
  bx = newx * factor;
  by = newy * factor;
  newx = bx;
  newy = by;
  newx += width >> 1;
  newy += height >> 1;
  
  if(newx >= width || newx < 0 || newy >= height || newy < 0)
    {
      newx = 0;
      newy = 0;
    }
  
  if(newx < 0 || newx >= width || newy < 0 || newy > height)
    {
      newx = width >> 1;
      newy = height >> 1;
    }
  
  *x = newx;
  *y = newy;
}

__inline__ static void rotate(register Sint32 *x, register Sint32 *y)
{
  Sint32 newx, newy;
  double bx, by;
  
  newx = *x - (width >> 1);
  newy = *y - (height >> 1);
  bx = newx * cos(angle) + newy * sin(angle);
  by = newy * cos(angle) - newx * sin(angle);
  newx = bx;
  newy = by;
  newx += width >> 1;
  newy += height >> 1;
  
  if(newx >= width || newx < 0 || newy >= height || newy < 0)
    {
      newx = 0;
      newy = 0;
    }
  
  if(newx < 0 || newx >= width || newy < 0 || newy > height)
    {
      newx = width >> 1;
      newy = height >> 1;
    }
  
  *x = newx;
  *y = newy;
}

static void init_transform()
{
  Sint32 x, y, i, tx, ty;
  
  transform_table = malloc(height * width * sizeof(Uint32 *) * 4);
  
  i = 0;
  
  for(y = 1; y < height - 1; y++)
    for(x = 1; x < width - 1; x++)
      {
        tx = x + 1; ty = y;
        zoom(&tx, &ty);
        rotate(&tx, &ty);
        transform_table[i++] = pixels + (pitch >> 2) * ty + tx;
        tx = x - 1; ty = y;
        zoom(&tx, &ty);
        rotate(&tx, &ty);
        transform_table[i++] = pixels + (pitch >> 2) * ty + tx;
        tx = x; ty = y + 1;
        zoom(&tx, &ty);
        rotate(&tx, &ty);
        transform_table[i++] = pixels + (pitch >> 2) * ty + tx;
        tx = x; ty = y - 1;
        zoom(&tx, &ty);
        rotate(&tx, &ty);
        transform_table[i++] = pixels + (pitch >> 2) * ty + tx;
      }
}

static void blur()
{
  register Sint32 x, y, i;
  Uint32 *new = malloc(pitch * height);
  
  i = 0;
  
  for(y = 1; y < height - 1; y++)
    for(x = 1; x < width - 1; x++)
      {
        if((y == (height >> 1) || y == (height >> 1) + 1) &&
           (x == (width >> 1) || x == (width >> 1) + 1))
          new[y * (pitch >> 2) + x] = 0;
        else
          new[y * (pitch >> 2) + x] = average(*transform_table[i + 3],
                                              *transform_table[i + 2],
                                              *transform_table[i + 1],
                                              *transform_table[i]);
        i += 4;
      }
  
  memcpy(pixels, new, pitch * height);
  free(new);
}

static void black()
{
  memset(pixels, 0, pitch * height);
}

static __inline__ int amplification(int value)
{
  register int v = value;
  
  if(amp_mode == AMP_HALF)
    return v / 2;
  
  if(amp_mode == AMP_DOUBLE)
    return v * 2;
    
  return v;
}

static __inline__ int offset(int value)
{
  register int v = amplification(value);
  
  v += offset_mode;
  
  if(v >  32767) return  32767;
  if(v < -32768) return -32768;
  
  return v;
}


static void circle_scope(short data[512])
{
  register Sint32 thisx, thisy;
  register Sint32 nextx, nexty;
  Sint32 firstx, firsty;
  Sint32 i, idx, inc = 512 / samples;  
  
  
  firstx = thisx = ((sine_table[0] * (offset(data[0]) + 32768)) >> 16) + (width >> 1);
  firsty = thisy = ((cosine_table[0] * (offset(data[0]) + 32768)) >> 16) + (height >> 1);

  for(i = 1, idx = 0; i < samples; i++, idx += inc)
    {
      nextx = ((sine_table[i] * (offset(data[idx]) + 32768)) >> 16) + (width >> 1);
      nexty = ((cosine_table[i] * (offset(data[idx]) + 32768)) >> 16) + (height >> 1);

      line(thisx, thisy, nextx, nexty);

      thisx = nextx;
      thisy = nexty;
    }
  
  line(thisx, thisy, firstx, firsty);
}

static void line_scope(short data[512])
{
  register Sint32 x, y;
  register Sint32 i, idx, inc = 512 / samples;
  
  for(i = 0, idx = 0; i < samples; i++, idx += inc)
    {
      x = ((sine_table[i] * (offset(data[idx]) + 32768)) >> 16) + (width >> 1);
      y = ((cosine_table[i] * (offset(data[idx]) + 32768)) >> 16) + (height >> 1);
      
      line(width >> 1, height >> 1, x, y);
    }
}

static void dot_scope(short data[512])
{
  register Sint32 x, y;
  register Sint32 i, idx, inc = 512 / samples;
  
  for(i = 0, idx = 0; i < samples; i++, idx += inc)
    {
      x = ((sine_table[i] * (offset(data[i]) + 32768)) >> 16) + (width >> 1);
      y = ((cosine_table[i] * (offset(data[i]) + 32768)) >> 16) + (height >> 1);
      
      pixels[y * (pitch >> 2) + x] = color;
    }
}

static void ball_scope(short data[512])
{
  register Sint32 x, y;
  register Sint32 i, idx, inc = 512 / samples;
  
  for(i = 0, idx = 0; i < samples; i++, idx += inc)
    {
      x = ((sine_table[i] * (offset(data[idx]) + 32768)) >> 16) + (width >> 1);
      y = ((cosine_table[i] * (offset(data[idx]) + 32768)) >> 16) + (height >> 1);
      
      pixels[y * (pitch >> 2) + x + 1] = color;
      pixels[y * (pitch >> 2) + x - 1] = color;
      pixels[(y + 1) * (pitch >> 2) + x] = color;
      pixels[(y - 1) * (pitch >> 2) + x] = color;
    }
}

void find_color(Sint16 data[2][256])
{
  Uint32 value = 0;
  Sint32 x;
  Sint32 count = 0;
  Sint32 i;
  
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
        value = 511;
      color = color_table[value];
    }
  else
    color = color_table[0];
}

static bool check_finished()
{
  bool ret;
  
  pthread_mutex_lock(&status_mutex);
  
  ret = beatflower_finished;
  
  if(reset)
    {
      reset = FALSE;      
      init_engine();
    }
  
  pthread_mutex_unlock(&status_mutex);
    
  return ret;
}

static bool check_playing()
{
  bool ret;
  
  pthread_mutex_lock(&status_mutex);
  ret = beatflower_playing;
  pthread_mutex_unlock(&status_mutex);
  
  return ret;
}


void *beatflower_thread(void *blah)
{ 
  while(!check_playing())
    {
      if(check_finished())
        return NULL;
      
      SDL_Delay(10);
    }

  init_engine();

  while(!check_finished())
    {      
      pthread_mutex_lock(&data_mutex);
      find_color(freq_data);
      scope(pcm_data[0]);
      pthread_mutex_unlock(&data_mutex);

      if(check_playing())
        SDL_Flip(screen);      
      
      if(blur_enable)
        blur();
      
      if(!blur_enable)
        black();        
    }
  
  return NULL;
}

void on_fullscreen_checkbutton_clicked(GtkCheckButton *checkbutton)
{
  newconfig.fullscreen = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton));
}

void on_width_spinbutton_changed(GtkSpinButton *spinbutton)
{
  int value;
  
  value = gtk_spin_button_get_value_as_int(spinbutton);
  
  if(value > 160 && value < 1600)
    {
      newconfig.width = value;
    }
}
  
void on_height_spinbutton_changed(GtkSpinButton *spinbutton)
{
  int value;
  
  value = gtk_spin_button_get_value_as_int(spinbutton);
  
  if(value > 160 && value < 1400)
    {
      newconfig.height = value;
    }
}

/*void on_color1_color_set(GnomeColorPicker *gnomecolorpicker,
                         guint             arg1,
                         guint             arg2,
                         guint             arg3,
                         guint             arg4)
{  
  unsigned char red, green, blue;
  
  gnome_color_picker_get_i8(gnomecolorpicker, &red, &green, &blue, NULL);
  newconfig.color1 = (red << RED_SHIFT) | (green << GREEN_SHIFT) | (blue << BLUE_SHIFT);
}

void on_color2_color_set(GnomeColorPicker *gnomecolorpicker,
                         guint             arg1,
                         guint             arg2,
                         guint             arg3,
                         guint             arg4)
{  
  unsigned char red, green, blue;
  
  gnome_color_picker_get_i8(gnomecolorpicker, &red, &green, &blue, NULL);
  newconfig.color2 = (red << RED_SHIFT) | (green << GREEN_SHIFT) | (blue << BLUE_SHIFT);
}

void on_color3_color_set(GnomeColorPicker *gnomecolorpicker,
                         guint             arg1,
                         guint             arg2,
                         guint             arg3,
                         guint             arg4)
{  
  unsigned char red, green, blue;
  
  gnome_color_picker_get_i8(gnomecolorpicker, &red, &green, &blue, NULL);
  newconfig.color3 = (red << RED_SHIFT) | (green << GREEN_SHIFT) | (blue << BLUE_SHIFT);
}*/

void on_mode_option_selected(GtkMenuShell *shell)
{
  GtkWidget *active;
  gint32     index;
  
  active = gtk_menu_get_active(GTK_MENU(shell));
  index = g_list_index(shell->children, active);
  newconfig.color_mode = index;
}   
  
void on_draw_option_selected(GtkMenuShell *shell)
{
  GtkWidget *active;
  gint32     index;
  
  active = gtk_menu_get_active(GTK_MENU(shell));
  index = g_list_index(shell->children, active);  
  newconfig.draw_mode = index;
}   
  
void on_samples_option_selected(GtkMenuShell *shell)
{
  GtkWidget *active;
  gint32     index;
  
  active = gtk_menu_get_active(GTK_MENU (shell));
  index = g_list_index(shell->children, active);  
  newconfig.samples_mode = index;
}   
  
void on_amplification_option_selected(GtkMenuShell *shell)
{
  GtkWidget *active;
  gint32     index;
  
  active = gtk_menu_get_active (GTK_MENU (shell));
  index = g_list_index (shell->children, active);  
  newconfig.amplification_mode = index;
}   
  
void on_offset_option_selected(GtkMenuShell *shell)
{
  GtkWidget *active;
  gint32     index;
  
  active = gtk_menu_get_active (GTK_MENU (shell));
  index = g_list_index (shell->children, active);  
  newconfig.offset_mode = index;
}   

void on_blur_checkbutton_clicked(GtkCheckButton *checkbutton)
{
  newconfig.blur = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton));
}

void on_apply_button_clicked(GtkButton *button)
{
  config_save(&newconfig);

  pthread_mutex_lock(&config_mutex);
  config = newconfig;
  pthread_mutex_unlock(&config_mutex);
  
  pthread_mutex_lock(&status_mutex);
  if(beatflower_playing) reset = TRUE;
  pthread_mutex_unlock(&status_mutex);
}

void on_cancel_button_clicked(GtkButton *button)
{
}

void on_ok_button_clicked(GtkButton *button)
{
  on_apply_button_clicked(button);
  on_cancel_button_clicked(button);
}

void on_decay_spinbutton_changed(GtkSpinButton *spinbutton)
{  
  int value;
  
  value = gtk_spin_button_get_value_as_int(spinbutton);
  
  if(value > 0)
    {
      newconfig.decay = value;
    }
}

void on_zoom_checkbutton_clicked(GtkCheckButton *checkbutton)
{
  newconfig.zoombeat = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton));
}

void on_factor_spinbutton_changed(GtkSpinButton *spinbutton)
{  
  double value;
  
  value = gtk_spin_button_get_value_as_float(spinbutton);
  
  if(value > 0)
    {
      newconfig.factor = value;
    }
}

void on_rotate_checkbutton_clicked(GtkCheckButton *checkbutton)
{
  newconfig.rotatebeat = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton));
}

void on_angle_spinbutton_changed(GtkSpinButton *spinbutton)
{  
  double value;
  
  value = gtk_spin_button_get_value_as_float(spinbutton);
  
  if(value <= 180 && value >= -180)
    {
      newconfig.angle = value;
    }
}

