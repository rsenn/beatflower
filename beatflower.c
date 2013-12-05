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
static bool         config_loaded = FALSE;
/*static bool         reinit        = FALSE;*/
static bool         playing;
static bool         finished;            /* some status variables... */
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
static void         beatflower_init();
static void         beatflower_cleanup();
static void         beatflower_about();
static void         beatflower_configure();
static void         beatflower_playback_start();
static void         beatflower_playback_stop();
static void         beatflower_render_pcm(short data[2][512]);
static void         beatflower_render_freq(short data[2][256]);
static void        *beatflower_thread(void *blah);
static bool         check_finished();
static void         beatflower_about();

/* the only non-static thing.... */
VisPlugin *get_vplugin_info()
{
  return &beatflower;
}

static VisPlugin beatflower =  {
  NULL,
  NULL,
  0, /* session id, initialized by xmms */
  "beatflower "VERSION,
  2,
  2,
  beatflower_init,
  beatflower_cleanup,
  beatflower_about,
  beatflower_configure,
  NULL, /* disable */
  beatflower_playback_start,
  beatflower_playback_stop,
  beatflower_render_pcm,
  beatflower_render_freq
};

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
  
  config_loaded = TRUE;
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
  
  ret = finished;
  
  if(reset)
    {
      reset = FALSE;      
      init_engine();
    }
  
  pthread_mutex_unlock(&status_mutex);
    
  return ret;
}

/****************************** XMMS callbacks ********************************/

/* initialize values and start the beatflower thread */
void beatflower_init()
{
  pthread_attr_t attr;
  srand(time(NULL));
  pthread_mutex_lock(&config_mutex);
  if(!config_loaded)
    config_load(&config);
  pthread_mutex_unlock(&config_mutex);
  finished = FALSE;
  playing  = FALSE;
  reset    = FALSE;
  pthread_attr_init(&attr);
  pthread_create(&thread, NULL, beatflower_thread, &attr);
}

void beatflower_cleanup()
{
  pthread_mutex_lock(&status_mutex);
  finished = TRUE;
  playing  = FALSE;
  pthread_mutex_unlock(&status_mutex);
}

void beatflower_playback_start()
{
  pthread_mutex_lock(&status_mutex);
  playing = TRUE;
  pthread_mutex_unlock(&status_mutex);
}

void beatflower_playback_stop()
{
  pthread_mutex_lock(&status_mutex);
  playing = FALSE;
  pthread_mutex_unlock(&status_mutex);
}

void beatflower_render_pcm(short data[2][512])
{
  pthread_mutex_lock(&data_mutex);
  memcpy(pcm_data, data, 1024);
  pthread_mutex_unlock(&data_mutex);
}

void beatflower_render_freq(short data[2][256])
{
  pthread_mutex_lock(&data_mutex);
  memcpy(freq_data, data, 512);
  pthread_mutex_unlock(&data_mutex);
}

static bool check_playing()
{
  bool ret;
  
  pthread_mutex_lock(&status_mutex);
  ret = playing;
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
  if(playing) reset = TRUE;
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

void beatflower_configure()
{
  GtkWidget *conf;
  GtkWidget *vbox1;
  GtkWidget *vbox2;
  GtkWidget *notebook;
  GtkWidget *fullscreen_box;
  GtkWidget *fullscreen_checkbutton;
  GtkWidget *resolution_table;
  GtkWidget *width_label;
  GtkObject *width_spinbutton_adj;
  GtkWidget *width_spinbutton;
  GtkWidget *height_label;
  GtkObject *height_spinbutton_adj;
  GtkWidget *height_spinbutton;
  GtkWidget *renderer_label;
  GtkWidget *scope_box;
  GtkWidget *color_frame;
  GtkWidget *color_box;
  GtkWidget *mode_box;
  GtkWidget *mode_label;
  GtkWidget *mode_option;
  GtkWidget *mode_option_menu;
  GtkWidget *glade_menuitem;
  GtkWidget *color_table;
  GtkWidget *color1_label;
  GtkWidget *color2_label;
  GtkWidget *color3_label;
/*  GtkWidget *color3;
  GtkWidget *color2;
  GtkWidget *color1;*/
  GtkWidget *amplitude_frame;
  GtkWidget *amplitude_table;
  GtkWidget *draw_label;
  GtkWidget *draw_option;
  GtkWidget *draw_option_menu;
  GtkWidget *samples_label;
  GtkWidget *samples_option;
  GtkWidget *samples_option_menu;
  GtkWidget *amplification_label;
  GtkWidget *amplification_option;
  GtkWidget *amplification_option_menu;
  GtkWidget *offset_label;
  GtkWidget *offset_option;
  GtkWidget *offset_option_menu;
  GtkWidget *scope_label;
  GtkWidget *blur_box;
  GtkWidget *blur_checkbutton;
  GtkWidget *decay_box;
  GtkWidget *decay_label;
  GtkObject *decay_spinbutton_adj;
  GtkWidget *decay_spinbutton;
  GtkWidget *special_box;
  GtkWidget *zoom_frame;
  GtkWidget *zoom_box;
  GtkWidget *zoom_checkbutton;
  GtkWidget *zoom_rate_box;
  GtkWidget *zoom_rate_label;
  GtkObject *factor_spinbutton_adj;
  GtkWidget *factor_spinbutton;
  GtkWidget *rotate_frame;
  GtkWidget *rotate_box;
  GtkWidget *rotate_checkbutton;
  GtkWidget *angle_box;
  GtkWidget *angle_label;
  GtkObject *angle_spinbutton_adj;
  GtkWidget *angle_spinbutton;
  GtkWidget *blur_label;
  GtkWidget *hbuttonbox1;
  GtkWidget *ok_button;
  GtkWidget *apply_button;
  GtkWidget *cancel_button;
  
  pthread_mutex_lock(&config_mutex);
  if(!config_loaded)
    config_load(&config);
  newconfig = config;
  pthread_mutex_unlock(&config_mutex);

    conf = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data (GTK_OBJECT (conf), "conf", conf);
  gtk_window_set_title (GTK_WINDOW (conf), "conf");
  
  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox1);
  gtk_object_set_data_full (GTK_OBJECT (conf), "vbox1", vbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (conf), vbox1);
  
  notebook = gtk_notebook_new ();
  gtk_widget_ref (notebook);
  gtk_object_set_data_full (GTK_OBJECT (conf), "notebook", notebook,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (notebook);
  gtk_box_pack_start (GTK_BOX (vbox1), notebook, TRUE, TRUE, 0);
  
  fullscreen_box = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (fullscreen_box);
  gtk_object_set_data_full (GTK_OBJECT (conf), "fullscreen_box", fullscreen_box,

                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fullscreen_box);
  gtk_container_add (GTK_CONTAINER (notebook), fullscreen_box);
  
  fullscreen_checkbutton = gtk_check_button_new_with_label ("Fullscreen");
  gtk_widget_ref (fullscreen_checkbutton);
  gtk_object_set_data_full (GTK_OBJECT (conf), "fullscreen_checkbutton", fullscreen_checkbutton,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fullscreen_checkbutton);
  gtk_box_pack_start (GTK_BOX (fullscreen_box), fullscreen_checkbutton, FALSE, FALSE, 0);
  
  resolution_table = gtk_table_new (2, 2, FALSE);
  gtk_widget_ref (resolution_table);
  gtk_object_set_data_full (GTK_OBJECT (conf), "resolution_table", resolution_table,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (resolution_table);
  gtk_box_pack_start (GTK_BOX (fullscreen_box), resolution_table, TRUE, TRUE, 0);
  
  width_label = gtk_label_new ("Width");
  gtk_widget_ref (width_label);
  gtk_object_set_data_full (GTK_OBJECT (conf), "width_label", width_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (width_label);
  gtk_table_attach (GTK_TABLE (resolution_table), width_label, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 4, 4);
  gtk_misc_set_alignment (GTK_MISC (width_label), 0, 0.5);
  
  width_spinbutton_adj = gtk_adjustment_new (320, 0, 1600, 1, 10, 10);
  width_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (width_spinbutton_adj), 1, 0);
  gtk_widget_ref (width_spinbutton);
  gtk_object_set_data_full (GTK_OBJECT (conf), "width_spinbutton", width_spinbutton,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (width_spinbutton);
  gtk_table_attach (GTK_TABLE (resolution_table), width_spinbutton, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 4, 4);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (width_spinbutton), TRUE);
  
  height_label = gtk_label_new ("Height");
  gtk_widget_ref (height_label);
  gtk_object_set_data_full (GTK_OBJECT (conf), "height_label", height_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (height_label);
  gtk_table_attach (GTK_TABLE (resolution_table), height_label, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 4, 4);
  gtk_misc_set_alignment (GTK_MISC (height_label), 0, 0.5);

  height_spinbutton_adj = gtk_adjustment_new (320, 0, 1400, 1, 10, 10);
  height_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (height_spinbutton_adj), 1, 0);
  gtk_widget_ref (height_spinbutton);
  gtk_object_set_data_full (GTK_OBJECT (conf), "height_spinbutton", height_spinbutton,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (height_spinbutton);
  gtk_table_attach (GTK_TABLE (resolution_table), height_spinbutton, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 4, 4);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (height_spinbutton), TRUE);

  renderer_label = gtk_label_new ("Renderer");
  gtk_widget_ref (renderer_label);
  gtk_object_set_data_full (GTK_OBJECT (conf), "renderer_label", renderer_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (renderer_label);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 0), renderer_label);

  scope_box = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (scope_box);
  gtk_object_set_data_full (GTK_OBJECT (conf), "scope_box", scope_box,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scope_box);
  gtk_container_add (GTK_CONTAINER (notebook), scope_box);

  color_frame = gtk_frame_new ("Color");
  gtk_widget_ref (color_frame);
  gtk_object_set_data_full (GTK_OBJECT (conf), "color_frame", color_frame,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (color_frame);
  gtk_box_pack_start (GTK_BOX (scope_box), color_frame, TRUE, TRUE, 4);

  color_box = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (color_box);
  gtk_object_set_data_full (GTK_OBJECT (conf), "color_box", color_box,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (color_box);
  gtk_container_add (GTK_CONTAINER (color_frame), color_box);

  mode_box = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (mode_box);
  gtk_object_set_data_full (GTK_OBJECT (conf), "mode_box", mode_box,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (mode_box);
  gtk_box_pack_start (GTK_BOX (color_box), mode_box, FALSE, FALSE, 4);

  mode_label = gtk_label_new ("Mode");
  gtk_widget_ref (mode_label);
  gtk_object_set_data_full (GTK_OBJECT (conf), "mode_label", mode_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (mode_label);
  gtk_box_pack_start (GTK_BOX (mode_box), mode_label, FALSE, FALSE, 4);

  mode_option = gtk_option_menu_new ();
  gtk_widget_ref (mode_option);
  gtk_object_set_data_full (GTK_OBJECT (conf), "mode_option", mode_option,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_show (mode_option);
  gtk_box_pack_start (GTK_BOX (mode_box), mode_option, FALSE, FALSE, 4);
  mode_option_menu = gtk_menu_new ();
  glade_menuitem = gtk_menu_item_new_with_label ("2 Color Gradient");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (mode_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("3 Color Gradient");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (mode_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("Random");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (mode_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("From Frequencies");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (mode_option_menu), glade_menuitem);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (mode_option), mode_option_menu);

  color_table = gtk_table_new (3, 2, FALSE);
  gtk_widget_ref (color_table);
  gtk_object_set_data_full (GTK_OBJECT (conf), "color_table", color_table,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (color_table);
  gtk_box_pack_start (GTK_BOX (color_box), color_table, TRUE, TRUE, 0);

  color1_label = gtk_label_new ("Color 1");
  gtk_widget_ref (color1_label);
  gtk_object_set_data_full (GTK_OBJECT (conf), "color1_label", color1_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (color1_label);
  gtk_table_attach (GTK_TABLE (color_table), color1_label, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_misc_set_alignment (GTK_MISC (color1_label), 0, 0.5);

  color2_label = gtk_label_new ("Color 2 ");
  gtk_widget_ref (color2_label);
  gtk_object_set_data_full (GTK_OBJECT (conf), "color2_label", color2_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (color2_label);
  gtk_table_attach (GTK_TABLE (color_table), color2_label, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 3, 3);
  gtk_misc_set_alignment (GTK_MISC (color2_label), 0, 0.5);

  color3_label = gtk_label_new ("Color 3");
  gtk_widget_ref (color3_label);
  gtk_object_set_data_full (GTK_OBJECT (conf), "color3_label", color3_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (color3_label);
  gtk_table_attach (GTK_TABLE (color_table), color3_label, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 4, 4);
  gtk_misc_set_alignment (GTK_MISC (color3_label), 0, 0.5);

/*  color3 = gnome_color_picker_new ();
  gtk_widget_ref (color3);
  gtk_object_set_data_full (GTK_OBJECT (conf), "color3", color3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (color3);
  gtk_table_attach (GTK_TABLE (color_table), color3, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 4, 4);

  color2 = gnome_color_picker_new ();
  gtk_widget_ref (color2);
  gtk_object_set_data_full (GTK_OBJECT (conf), "color2", color2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (color2);
  gtk_table_attach (GTK_TABLE (color_table), color2, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 4, 4);

  color1 = gnome_color_picker_new ();
  gtk_widget_ref (color1);
  gtk_object_set_data_full (GTK_OBJECT (conf), "color1", color1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (color1);
  gtk_table_attach (GTK_TABLE (color_table), color1, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 4, 4);
*/
  amplitude_frame = gtk_frame_new ("Amplitude");
  gtk_widget_ref (amplitude_frame);
  gtk_object_set_data_full (GTK_OBJECT (conf), "amplitude_frame", amplitude_frame,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (amplitude_frame);
  gtk_box_pack_start (GTK_BOX (scope_box), amplitude_frame, TRUE, TRUE, 0);

  amplitude_table = gtk_table_new (4, 2, FALSE);
  gtk_widget_ref (amplitude_table);
  gtk_object_set_data_full (GTK_OBJECT (conf), "amplitude_table", amplitude_table,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (amplitude_table);
  gtk_container_add (GTK_CONTAINER (amplitude_frame), amplitude_table);

  draw_label = gtk_label_new ("Draw Mode");
  gtk_widget_ref (draw_label);
  gtk_object_set_data_full (GTK_OBJECT (conf), "draw_label", draw_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (draw_label);
  gtk_table_attach (GTK_TABLE (amplitude_table), draw_label, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 4, 4);
  gtk_misc_set_alignment (GTK_MISC (draw_label), 1, 0);

  draw_option = gtk_option_menu_new ();
  gtk_widget_ref (draw_option);
  gtk_object_set_data_full (GTK_OBJECT (conf), "draw_option", draw_option,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (draw_option);
  gtk_table_attach (GTK_TABLE (amplitude_table), draw_option, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 4, 4);
  draw_option_menu = gtk_menu_new ();
  glade_menuitem = gtk_menu_item_new_with_label ("Dots");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (draw_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("Balls");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (draw_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("Lines");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (draw_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("Lines from Center");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (draw_option_menu), glade_menuitem);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (draw_option), draw_option_menu);

  samples_label = gtk_label_new ("Samples");
  gtk_widget_ref (samples_label);
  gtk_object_set_data_full (GTK_OBJECT (conf), "samples_label", samples_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (samples_label);
  gtk_table_attach (GTK_TABLE (amplitude_table), samples_label, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 4, 4);
  gtk_misc_set_alignment (GTK_MISC (samples_label), 1, 0.5);

  samples_option = gtk_option_menu_new ();
  gtk_widget_ref (samples_option);
  gtk_object_set_data_full (GTK_OBJECT (conf), "samples_option", samples_option,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (samples_option);
  gtk_table_attach (GTK_TABLE (amplitude_table), samples_option, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 4, 4);
  samples_option_menu = gtk_menu_new ();
  glade_menuitem = gtk_menu_item_new_with_label ("32");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (samples_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("64");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (samples_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("128");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (samples_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("256");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (samples_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("512");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (samples_option_menu), glade_menuitem);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (samples_option), samples_option_menu);

  amplification_label = gtk_label_new ("Amplification");
  gtk_widget_ref (amplification_label);
  gtk_object_set_data_full (GTK_OBJECT (conf), "amplification_label", amplification_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (amplification_label);
  gtk_table_attach (GTK_TABLE (amplitude_table), amplification_label, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 4, 4);
  gtk_misc_set_alignment (GTK_MISC (amplification_label), 1, 0.5);

  amplification_option = gtk_option_menu_new ();
  gtk_widget_ref (amplification_option);
  gtk_object_set_data_full (GTK_OBJECT (conf), "amplification_option", amplification_option,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (amplification_option);
  gtk_table_attach (GTK_TABLE (amplitude_table), amplification_option, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 4, 4);
  amplification_option_menu = gtk_menu_new ();
  glade_menuitem = gtk_menu_item_new_with_label ("0.5");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (amplification_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("1");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (amplification_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("2");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (amplification_option_menu), glade_menuitem);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (amplification_option), amplification_option_menu);

  offset_label = gtk_label_new ("Offset");
  gtk_widget_ref (offset_label);
  gtk_object_set_data_full (GTK_OBJECT (conf), "offset_label", offset_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (offset_label);
  gtk_table_attach (GTK_TABLE (amplitude_table), offset_label, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 4, 4);
  gtk_misc_set_alignment (GTK_MISC (offset_label), 1, 0.5);

  offset_option = gtk_option_menu_new ();
  gtk_widget_ref (offset_option);
  gtk_object_set_data_full (GTK_OBJECT (conf), "offset_option", offset_option,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (offset_option);
  gtk_table_attach (GTK_TABLE (amplitude_table), offset_option, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 4, 4);
  offset_option_menu = gtk_menu_new ();
  glade_menuitem = gtk_menu_item_new_with_label ("-32768");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (offset_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("+/-0");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (offset_option_menu), glade_menuitem);
  glade_menuitem = gtk_menu_item_new_with_label ("+32768");
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (offset_option_menu), glade_menuitem);
  gtk_option_menu_set_menu (GTK_OPTION_MENU (offset_option), offset_option_menu);

  scope_label = gtk_label_new ("Scope");
  gtk_widget_ref (scope_label);
  gtk_object_set_data_full (GTK_OBJECT (conf), "scope_label", scope_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scope_label);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK 
(notebook), 1), scope_label);

  blur_box = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (blur_box);
  gtk_object_set_data_full (GTK_OBJECT (conf), "blur_box", blur_box,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (blur_box);
  gtk_container_add (GTK_CONTAINER (notebook), blur_box);

  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox2);
  gtk_object_set_data_full (GTK_OBJECT (conf), "vbox2", vbox2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox2);
  gtk_box_pack_start (GTK_BOX (blur_box), vbox2, TRUE, TRUE, 0);

  blur_checkbutton = gtk_check_button_new_with_label ("Enable Blur");
  gtk_widget_ref (blur_checkbutton);
  gtk_object_set_data_full (GTK_OBJECT (conf), "blur_checkbutton", blur_checkbutton,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (blur_checkbutton);
  gtk_box_pack_start (GTK_BOX (vbox2), blur_checkbutton, FALSE, FALSE, 4);

  decay_box = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (decay_box);
  gtk_object_set_data_full (GTK_OBJECT (conf), "decay_box", decay_box,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (decay_box);
  gtk_box_pack_start (GTK_BOX (vbox2), decay_box, TRUE, TRUE, 0);

  decay_label = gtk_label_new ("Decay Rate");
  gtk_widget_ref (decay_label);
  gtk_object_set_data_full (GTK_OBJECT (conf), "decay_label", decay_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (decay_label);
  gtk_box_pack_start (GTK_BOX (decay_box), decay_label, FALSE, FALSE, 4);

  decay_spinbutton_adj = gtk_adjustment_new (1, 0, 100, 1, 10, 10);
  decay_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (decay_spinbutton_adj), 1, 0);
  gtk_widget_ref (decay_spinbutton);
  gtk_object_set_data_full (GTK_OBJECT (conf), "decay_spinbutton", decay_spinbutton,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_box_pack_start (GTK_BOX (decay_box), decay_spinbutton, TRUE, TRUE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (decay_spinbutton), TRUE);

  special_box = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (special_box);
  gtk_object_set_data_full (GTK_OBJECT (conf), "special_box", special_box,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (special_box);
  gtk_box_pack_start (GTK_BOX (blur_box), special_box, TRUE, TRUE, 0);

  zoom_frame = gtk_frame_new ("Zoom");
  gtk_widget_ref (zoom_frame);
  gtk_object_set_data_full (GTK_OBJECT (conf), "zoom_frame", zoom_frame,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (zoom_frame);
  gtk_box_pack_start (GTK_BOX (special_box), zoom_frame, TRUE, TRUE, 4);

  zoom_box = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (zoom_box);
  gtk_object_set_data_full (GTK_OBJECT (conf), "zoom_box", zoom_box,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (zoom_box);
  gtk_container_add (GTK_CONTAINER (zoom_frame), zoom_box);

  zoom_checkbutton = gtk_check_button_new_with_label ("Zoom in/out by beat");
  gtk_widget_ref (zoom_checkbutton);
  gtk_object_set_data_full (GTK_OBJECT (conf), "zoom_checkbutton", zoom_checkbutton,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (zoom_checkbutton);
  gtk_box_pack_start (GTK_BOX (zoom_box), zoom_checkbutton, FALSE, FALSE, 4);

  zoom_rate_box = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (zoom_rate_box);
  gtk_object_set_data_full (GTK_OBJECT (conf), "zoom_rate_box", zoom_rate_box,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (zoom_rate_box);
  gtk_box_pack_start (GTK_BOX (zoom_box), zoom_rate_box, TRUE, TRUE, 0);

  zoom_rate_label = gtk_label_new ("Rate");
  gtk_widget_ref (zoom_rate_label);
  gtk_object_set_data_full (GTK_OBJECT (conf), "zoom_rate_label", zoom_rate_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (zoom_rate_label);
  gtk_box_pack_start (GTK_BOX (zoom_rate_box), zoom_rate_label, FALSE, FALSE, 4);

  factor_spinbutton_adj = gtk_adjustment_new (1, 0, 100, 1, 10, 10);
  factor_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (factor_spinbutton_adj), 1, 3);
  gtk_widget_ref (factor_spinbutton);
 gtk_object_set_data_full (GTK_OBJECT (conf), "factor_spinbutton", factor_spinbutton,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (factor_spinbutton);
  gtk_box_pack_start (GTK_BOX (zoom_rate_box), factor_spinbutton, TRUE, TRUE, 4);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (factor_spinbutton), TRUE);
  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (factor_spinbutton), GTK_UPDATE_IF_VALID);

  rotate_frame = gtk_frame_new ("Rotate");
  gtk_widget_ref (rotate_frame);
  gtk_object_set_data_full (GTK_OBJECT (conf), "rotate_frame", rotate_frame,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (rotate_frame);
  gtk_box_pack_start (GTK_BOX (special_box), rotate_frame, TRUE, TRUE, 4);

  rotate_box = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (rotate_box);
  gtk_object_set_data_full (GTK_OBJECT (conf), "rotate_box", rotate_box,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (rotate_box);
  gtk_container_add (GTK_CONTAINER (rotate_frame), rotate_box);

  rotate_checkbutton = gtk_check_button_new_with_label ("Rotate left/right by beat");
  gtk_widget_ref (rotate_checkbutton);
  gtk_object_set_data_full (GTK_OBJECT (conf), "rotate_checkbutton", rotate_checkbutton,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (rotate_checkbutton);
  gtk_box_pack_start (GTK_BOX (rotate_box), rotate_checkbutton, FALSE, FALSE, 4);

  angle_box = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (angle_box);
  gtk_object_set_data_full (GTK_OBJECT (conf), "angle_box", angle_box,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (angle_box);
  gtk_box_pack_start (GTK_BOX (rotate_box), angle_box, TRUE, TRUE, 0);

  angle_label = gtk_label_new ("Angle");
  gtk_widget_ref (angle_label);
  gtk_object_set_data_full (GTK_OBJECT (conf), "angle_label", angle_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (angle_label);
  gtk_box_pack_start (GTK_BOX (angle_box), angle_label, FALSE, FALSE, 4);

  angle_spinbutton_adj = gtk_adjustment_new (1, -180, 180, 1, 10, 10);
  angle_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (angle_spinbutton_adj), 1, 3);
  gtk_widget_ref (angle_spinbutton);
  gtk_object_set_data_full (GTK_OBJECT (conf), "angle_spinbutton", angle_spinbutton,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (angle_spinbutton);
  gtk_box_pack_start (GTK_BOX (angle_box), angle_spinbutton, TRUE, TRUE, 4);

  blur_label = gtk_label_new ("Blur");
  gtk_widget_ref (blur_label);
  gtk_object_set_data_full (GTK_OBJECT (conf), "blur_label", blur_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (blur_label);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK 
(notebook), 2), blur_label);

  hbuttonbox1 = gtk_hbutton_box_new ();
  gtk_widget_ref (hbuttonbox1);
  gtk_object_set_data_full (GTK_OBJECT (conf), "hbuttonbox1", hbuttonbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbuttonbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox1, TRUE, TRUE, 0);

  ok_button = gtk_button_new_with_label ("Ok");
  gtk_widget_ref (ok_button);
  gtk_object_set_data_full (GTK_OBJECT (conf), "ok_button", ok_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (ok_button);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), ok_button);
  GTK_WIDGET_SET_FLAGS (ok_button, GTK_CAN_DEFAULT);

  apply_button = gtk_button_new_with_label ("Apply");
  gtk_widget_ref (apply_button);
  gtk_object_set_data_full (GTK_OBJECT (conf), "apply_button", apply_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (apply_button);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), apply_button);
  GTK_WIDGET_SET_FLAGS (apply_button, GTK_CAN_DEFAULT);

  cancel_button = gtk_button_new_with_label ("Cancel");
  gtk_widget_ref (cancel_button);
  gtk_object_set_data_full (GTK_OBJECT (conf), "cancel_button", cancel_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (cancel_button);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), cancel_button);
  GTK_WIDGET_SET_FLAGS (cancel_button, GTK_CAN_DEFAULT);

  gtk_signal_connect (GTK_OBJECT (width_spinbutton), "changed",
                      GTK_SIGNAL_FUNC (on_width_spinbutton_changed),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (height_spinbutton), "changed",
                      GTK_SIGNAL_FUNC (on_height_spinbutton_changed),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (mode_option), "clicked",
                      GTK_SIGNAL_FUNC (on_mode_option_selected),
                      NULL);
/*  gtk_signal_connect (GTK_OBJECT (color3), "color_set",
                      GTK_SIGNAL_FUNC (on_color3_color_set),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (color2), "color_set",
                      GTK_SIGNAL_FUNC (on_color2_color_set),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (color1), "color_set",
                      GTK_SIGNAL_FUNC (on_color1_color_set),
                      NULL);*/
  
  gtk_widget_show((GtkWidget *)conf);
}
 
void beatflower_about()
{
  
}
