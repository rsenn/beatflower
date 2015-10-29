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
#include <SDL/SDL.h>

#include "beatflower.h"
#include "beatflower_renderer.h"

/*************************** Global function prototypes ************************************/
void beatflower_renderer_sdl_init();
void *beatflower_renderer_sdl_thread(void *blah);

/****************************** Renderer callbacks *****************************************/
const beatflower_renderer_t beatflower_renderer_sdl = {
  .init = &beatflower_renderer_sdl_init,
  .thread = &beatflower_renderer_sdl_thread,
};

/*************************** Local function prototypes *************************************/
static void create_color_table(void);
static void line(Uint32 x1, Uint32 y1, Uint32 x2, Uint32 y2);
static void create_sine_tables(void);
static Uint32 average(Uint32 c1, Uint32 c2, Uint32 c3, Uint32 c4);
static void zoom(register Sint32 *x, register Sint32 *y);
static void rotate(register Sint32 *x, register Sint32 *y);
static void init_transform(void);
static void blur(void);
static void black(void);
static void circle_scope(short data[512]);
static void line_scope(short data[512]);
static void dot_scope(short data[512]);
static void ball_scope(short data[512]);

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
static SDL_Surface *screen;

/****************************************************************************************
 * initialize the beatflower engine
 ****************************************************************************************/
void
beatflower_renderer_sdl_init(void)
{
  fprintf(stderr, "%s()\n", __PRETTY_FUNCTION__);
  fflush(stderr);

  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE) < 0)
  {
    beatflower_log("SDL_Init() failed!");
    SDL_Quit();
  }

  else
  {
    beatflower_log("SDL_Init() ok.");
  }

  beatflower_status_mutex = SDL_CreateMutex();
  beatflower_data_mutex = SDL_CreateMutex();
  beatflower_config_mutex = SDL_CreateMutex();

  SDL_LockMutex(beatflower_config_mutex);

 // beatflower_xmms_config_load(&beatflower_config);

  beatflower_log("Initializing beatflower video mode %ux%u ...", beatflower_config.width, beatflower_config.height);

#if SDL_MAJOR_VERSION < 2
  //screen = SDL_SetVideoMode(beatflower_config.width, beatflower_config.height, 32, SDL_SWSURFACE);
  screen = SDL_SetVideoMode(beatflower_config.width, beatflower_config.height, 32, SDL_HWSURFACE);

  beatflower_log("SDL_SetVideoMode = %p (%s)", screen, SDL_GetError());

  //SDL_FillRect(screen, NULL, 0xffffffff);
  SDL_Flip(screen);

  SDL_WM_SetCaption("beatflower v"VERSION, NULL);
#else

  // Initialise libSDL.
  if(SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    beatflower_log("Could not initialize SDL: %s.\n", SDL_GetError());
    return;
  }

  // Create SDL graphics objects.
  SDL_Window * window = SDL_CreateWindow(
                          PACKAGE" v"VERSION,
                          SDL_WINDOWPOS_UNDEFINED,
                          SDL_WINDOWPOS_UNDEFINED,
                          beatflower_config.width, beatflower_config.height,
                          SDL_WINDOW_SHOWN/*|SDL_WINDOW_RESIZABLE*/);

  if(!window)
  {
    g_error("Couldn't create window: %s\n", SDL_GetError());
    return;
    //quit(3);
  }


  screen = SDL_GetWindowSurface(window);
#endif
  beatflower_log("screen = %08xl", (unsigned long) screen);

  beatflower.pixels = screen->pixels;
  beatflower.pitch  = screen->pitch;
  beatflower.width  = screen->w;
  beatflower.height = screen->h;
  beatflower.radius = (beatflower.width > beatflower.height ? beatflower.height >> 1 : beatflower.width >> 1);

  beatflower.samples = (beatflower_config.samples_mode == SAMPLES_32  ? 32  :
             (beatflower_config.samples_mode == SAMPLES_64  ? 64  :
              (beatflower_config.samples_mode == SAMPLES_128 ? 128 :
               (beatflower_config.samples_mode == SAMPLES_256 ? 256 : 512))));

  beatflower.scope = (beatflower_config.draw_mode == DRAW_DOTS   ? &dot_scope    :
           (beatflower_config.draw_mode == DRAW_BALLS  ? &ball_scope   :
            (beatflower_config.draw_mode == DRAW_CIRCLE ? &circle_scope : &line_scope)));

  beatflower.factor = beatflower_config.factor;
  beatflower.angle = beatflower_config.angle * M_2PI / 360;
  beatflower.blur_enable = beatflower_config.blur;
  beatflower.color_mode = beatflower_config.color_mode;
  beatflower.amp_mode = beatflower_config.amplification_mode;
  beatflower.offset_mode = (beatflower_config.offset_mode == OFFSET_MINUS ? -32768 :
                 (beatflower_config.offset_mode == OFFSET_PLUS  ?  32768 : 0));
  beatflower.color1 = beatflower_config.color1;
  beatflower.color2 = beatflower_config.color2;
  beatflower.color3 = beatflower_config.color3;
  beatflower.decay = beatflower_config.decay;

  create_sine_tables();
  init_transform();
  create_color_table();

  SDL_UnlockMutex(beatflower_config_mutex);
}

/****************************************************************************************
 * Rendering thread
 ****************************************************************************************/
void *
beatflower_renderer_sdl_thread(void *blah)
{
  //fprintf(stderr,"%s()\n", __PRETTY_FUNCTION__); fflush(stderr);

//  beatflower_renderer_sdl_init();

  while(!beatflower_check_playing())
  {
    if(beatflower_check_finished())
    {
      return NULL;
    }

    SDL_Delay(10);
  }

  while(!beatflower_check_finished())
  {
    SDL_LockMutex(beatflower_data_mutex);
    beatflower_find_color(beatflower_freq_data);
    beatflower.scope(beatflower_pcm_data[0]);
    SDL_UnlockMutex(beatflower_data_mutex);

    if(beatflower_check_playing())
    {
      static Uint32 ticks;
      Uint32 newticks;
      SDL_Flip(screen);
      newticks = SDL_GetTicks();

      //if(ticks) fprintf(stderr, "fps: %u\n", 1000 / (newticks - ticks));

      ticks = newticks;
    }

    if(beatflower.blur_enable)
    {
      blur();
    }

    if(!beatflower.blur_enable)
    {
      black();
    }
  }

  SDL_DestroyMutex(beatflower_status_mutex);
  SDL_DestroyMutex(beatflower_data_mutex);
  SDL_DestroyMutex(beatflower_config_mutex);

  SDL_Quit();

  return NULL;
}

/********************************* Functions *****************************************/
static void
create_color_table(void)
{
  if(beatflower.color_mode == COLOR_2_GRADIENT)
  {
    register int red, green, blue;
    register int dr, dg, db;
    int i;

    red   = (beatflower.color1 >> RED_SHIFT)   & 0xff;
    green = (beatflower.color1 >> GREEN_SHIFT) & 0xff;
    blue  = (beatflower.color1 >> BLUE_SHIFT)  & 0xff;

    dr = ((beatflower.color2 >> RED_SHIFT)   & 0xff) - red;
    dg = ((beatflower.color2 >> GREEN_SHIFT) & 0xff) - green;
    db = ((beatflower.color2 >> BLUE_SHIFT)  & 0xff) - blue;

    for(i = 0; i < 512; i++)
      beatflower.color_table[i] = ((((red   + ((i * dr) >> 9)) << RED_SHIFT)   & RED_MASK) |
                        (((green + ((i * dg) >> 9)) << GREEN_SHIFT) & GREEN_MASK) |
                        (((blue  + ((i * db) >> 9)) << BLUE_SHIFT)  & BLUE_MASK));
  }

  else if(beatflower.color_mode == COLOR_3_GRADIENT)
  {
    register int red, green, blue;
    register int dr, dg, db;
    int i;

    red   = (beatflower.color1 >> RED_SHIFT)   & 0xff;
    green = (beatflower.color1 >> GREEN_SHIFT) & 0xff;
    blue  = (beatflower.color1 >> BLUE_SHIFT)  & 0xff;

    dr = ((beatflower.color2 >> RED_SHIFT)   & 0xff) - red;
    dg = ((beatflower.color2 >> GREEN_SHIFT) & 0xff) - green;
    db = ((beatflower.color2 >> BLUE_SHIFT)  & 0xff) - blue;

    for(i = 0; i < 256; i++)
      beatflower.color_table[i] = ((((red   + ((i * dr) >> 8)) << RED_SHIFT)   & RED_MASK) |
                        (((green + ((i * dg) >> 8)) << GREEN_SHIFT) & GREEN_MASK) |
                        (((blue  + ((i * db) >> 8)) << BLUE_SHIFT)  & BLUE_MASK));

    red   = (beatflower.color2 >> RED_SHIFT)   & 0xff;
    green = (beatflower.color2 >> GREEN_SHIFT) & 0xff;
    blue  = (beatflower.color2 >> BLUE_SHIFT)  & 0xff;

    dr = ((beatflower.color3 >> RED_SHIFT)   & 0xff) - red;
    dg = ((beatflower.color3 >> GREEN_SHIFT) & 0xff) - green;
    db = ((beatflower.color3 >> BLUE_SHIFT)  & 0xff) - blue;

    for(i = 0; i < 256; i++)
      beatflower.color_table[i + 256] = ((((red   + ((i * dr) >> 8)) << RED_SHIFT)   & RED_MASK) |
                              (((green + ((i * dg) >> 8)) << GREEN_SHIFT) & GREEN_MASK) |
                              (((blue  + ((i * db) >> 8)) << BLUE_SHIFT)  & BLUE_MASK));
  }
}

/* draw a line from [x1|y1] to [x2,y2]

   when x1 = x2 and y1 = y2, nothing is drawed
*/
static void
line(Uint32 x1, Uint32 y1, Uint32 x2, Uint32 y2)
{
  register Sint32 dx;
  register Sint32 dy;
  register Sint32 x;
  register Sint32 y;
  register Sint32 d;
  register char *p = (char *)beatflower.pixels;
  register Sint32 xinc = 4;
  register Sint32 yinc = beatflower.pitch;
  register Sint32 temp;

  /*  if(x1 == x2 && y1 == y2) return;*/

  p += (x1 * xinc) + (y1 * yinc);

  /* swap so line can be calculated from left-top to right-bottom */
  if(y1 > y2)
  {
    temp = y2;
    y2 = y1;
    y1 = temp;
    yinc = -yinc;
  }

  if(x1 > x2)
  {
    temp = x2;
    x2 = x1;
    x1 = temp;
    xinc = -xinc;
  }

  /* caculate horizontal and vertical delta */
  dx = x2 - x1;
  dy = y2 - y1;

  /*  if(dx == 1 && dy == 1)
      {
        *(Uint32 *)beatflower.pixels = beatflower.color;
        return;
      }*/

  /* swap all x and y stuff when dy > dx */
  if(dy > dx)
  {
    temp = yinc;
    yinc = xinc;
    xinc = temp;
    temp = y1;
    y1 = x1;
    x1 = temp;
    temp = y2;
    y2 = x2;
    x2 = temp;
    temp = dx;
    dx = dy;
    dy = temp;
  }

  /* the algorithm */
  y = y1;
  d = 2 * dy - dx;

  for(x = x1; x < x2; x++)
  {
    if(d < 0)
    {
      d += 2 * dy;
    }

    else
    {
      d += 2 * (dy - dx);
      y++;
      p += yinc;
    }

    p += xinc;
    *(Uint32 *)p = beatflower.color;
  }
}

static void
create_sine_tables(void)
{
  Sint32    i;
  double angle = 0;

  for(i = 0; i < beatflower.samples; i++)
  {
    angle += M_2PI / beatflower.samples;
    beatflower.sine_table[i]   = sin(angle) * (beatflower.radius - 1);
    beatflower.cosine_table[i] = cos(angle) * (beatflower.radius - 1);
  }
}

static Uint32
average(Uint32 c1, Uint32 c2, Uint32 c3, Uint32 c4)
{
  register Uint32 red, green, blue;

  red   = (((c1 >> RED_SHIFT)   & 0xff) + ((c2 >> RED_SHIFT)   & 0xff) +
           ((c3 >> RED_SHIFT)   & 0xff) + ((c4 >> RED_SHIFT)   & 0xff));
  green = (((c1 >> GREEN_SHIFT) & 0xff) + ((c2 >> GREEN_SHIFT) & 0xff) +
           ((c3 >> GREEN_SHIFT) & 0xff) + ((c4 >> GREEN_SHIFT) & 0xff));
  blue  = (((c1 >> BLUE_SHIFT)  & 0xff) + ((c2 >> BLUE_SHIFT)  & 0xff) +
           ((c3 >> BLUE_SHIFT)  & 0xff) + ((c4 >> BLUE_SHIFT)  & 0xff));

  if(red >= beatflower.decay)
  {
    red -= beatflower.decay;
  }

  if(green >= beatflower.decay)
  {
    green -= beatflower.decay;
  }

  if(blue >= beatflower.decay)
  {
    blue -= beatflower.decay;
  }

  red >>= 2;
  green >>= 2;
  blue >>= 2;

  return (red << RED_SHIFT | green << GREEN_SHIFT | blue << BLUE_SHIFT);
}

static void
zoom(register Sint32 *x, register Sint32 *y)
{
  Sint32 newx, newy;
  double bx, by;

  newx = *x - (beatflower.width >> 1);
  newy = *y - (beatflower.height >> 1);
  bx = newx * beatflower.factor;
  by = newy * beatflower.factor;
  newx = bx;
  newy = by;
  newx += beatflower.width >> 1;
  newy += beatflower.height >> 1;

  if(newx >= beatflower.width || newx < 0 || newy >= beatflower.height || newy < 0)
  {
    newx = 0;
    newy = 0;
  }

  if(newx < 0 || newx >= beatflower.width || newy < 0 || newy > beatflower.height)
  {
    newx = beatflower.width >> 1;
    newy = beatflower.height >> 1;
  }

  *x = newx;
  *y = newy;
}

static void
rotate(register Sint32 *x, register Sint32 *y)
{
  Sint32 newx, newy;
  double bx, by;

  newx = *x - (beatflower.width >> 1);
  newy = *y - (beatflower.height >> 1);
  bx = newx * cos(beatflower.angle) + newy * sin(beatflower.angle);
  by = newy * cos(beatflower.angle) - newx * sin(beatflower.angle);
  newx = bx;
  newy = by;
  newx += beatflower.width >> 1;
  newy += beatflower.height >> 1;

  if(newx >= beatflower.width || newx < 0 || newy >= beatflower.height || newy < 0)
  {
    newx = 0;
    newy = 0;
  }

  if(newx < 0 || newx >= beatflower.width || newy < 0 || newy > beatflower.height)
  {
    newx = beatflower.width >> 1;
    newy = beatflower.height >> 1;
  }

  *x = newx;
  *y = newy;
}

static void
init_transform(void)
{
  Sint32 x, y, i, tx, ty;

  beatflower.transform_table = malloc(beatflower.height * beatflower.width * sizeof(Uint32 *) * 4);

  i = 0;

  for(y = 1; y < beatflower.height - 1; y++)
    for(x = 1; x < beatflower.width - 1; x++)
    {
      tx = x + 1;
      ty = y;
      zoom(&tx, &ty);
      rotate(&tx, &ty);
      beatflower.transform_table[i++] = beatflower.pixels + (beatflower.pitch >> 2) * ty + tx;
      tx = x - 1;
      ty = y;
      zoom(&tx, &ty);
      rotate(&tx, &ty);
      beatflower.transform_table[i++] = beatflower.pixels + (beatflower.pitch >> 2) * ty + tx;
      tx = x;
      ty = y + 1;
      zoom(&tx, &ty);
      rotate(&tx, &ty);
      beatflower.transform_table[i++] = beatflower.pixels + (beatflower.pitch >> 2) * ty + tx;
      tx = x;
      ty = y - 1;
      zoom(&tx, &ty);
      rotate(&tx, &ty);
      beatflower.transform_table[i++] = beatflower.pixels + (beatflower.pitch >> 2) * ty + tx;
    }
}

static void
blur(void)
{
  register Sint32 x, y, i;
  Uint32 *new = malloc(beatflower.pitch * beatflower.height);

  i = 0;

  for(y = 1; y < beatflower.height - 1; y++)
    for(x = 1; x < beatflower.width - 1; x++)
    {
      if((y == (beatflower.height >> 1) || y == (beatflower.height >> 1) + 1) &&
          (x == (beatflower.width >> 1) || x == (beatflower.width >> 1) + 1))
      {
        new[y * (beatflower.pitch >> 2) + x] = 0;
      }

      else
        new[y * (beatflower.pitch >> 2) + x] = average(*beatflower.transform_table[i + 3],
                                            *beatflower.transform_table[i + 2],
                                            *beatflower.transform_table[i + 1],
                                            *beatflower.transform_table[i]);

      i += 4;
    }

  memcpy(beatflower.pixels, new, beatflower.pitch * beatflower.height);
  free(new);
}

static void
black(void)
{
  memset(beatflower.pixels, 0, beatflower.pitch * beatflower.height);
}

static void
circle_scope(short data[512])
{
  register Sint32 thisx, thisy;
  register Sint32 nextx, nexty;
  Sint32 firstx, firsty;
  Sint32 i, idx, inc = 512 / beatflower.samples;


  firstx = thisx = ((beatflower.sine_table[0] * (beatflower_scope_offset(data[0]) + 32768)) >> 16) + (beatflower.width >> 1);
  firsty = thisy = ((beatflower.cosine_table[0] * (beatflower_scope_offset(data[0]) + 32768)) >> 16) + (beatflower.height >> 1);

  for(i = 1, idx = 0; i < beatflower.samples; i++, idx += inc)
  {
    nextx = ((beatflower.sine_table[i] * (beatflower_scope_offset(data[idx]) + 32768)) >> 16) + (beatflower.width >> 1);
    nexty = ((beatflower.cosine_table[i] * (beatflower_scope_offset(data[idx]) + 32768)) >> 16) + (beatflower.height >> 1);

    line(thisx, thisy, nextx, nexty);

    thisx = nextx;
    thisy = nexty;
  }

  line(thisx, thisy, firstx, firsty);
}

static void
line_scope(short data[512])
{
  register Sint32 x, y;
  register Sint32 i, idx, inc = 512 / beatflower.samples;

  for(i = 0, idx = 0; i < beatflower.samples; i++, idx += inc)
  {
    x = ((beatflower.sine_table[i] * (beatflower_scope_offset(data[idx]) + 32768)) >> 16) + (beatflower.width >> 1);
    y = ((beatflower.cosine_table[i] * (beatflower_scope_offset(data[idx]) + 32768)) >> 16) + (beatflower.height >> 1);

    line(beatflower.width >> 1, beatflower.height >> 1, x, y);
  }
}

static void
dot_scope(short data[512])
{
  register Sint32 x, y;
  register Sint32 i, idx, inc = 512 / beatflower.samples;

  for(i = 0, idx = 0; i < beatflower.samples; i++, idx += inc)
  {
    x = ((beatflower.sine_table[i] * (beatflower_scope_offset(data[i]) + 32768)) >> 16) + (beatflower.width >> 1);
    y = ((beatflower.cosine_table[i] * (beatflower_scope_offset(data[i]) + 32768)) >> 16) + (beatflower.height >> 1);

    beatflower.pixels[y * (beatflower.pitch >> 2) + x] = beatflower.color;
  }
}

static void
ball_scope(short data[512])
{
  register Sint32 x, y;
  register Sint32 i, idx, inc = 512 / beatflower.samples;

  for(i = 0, idx = 0; i < beatflower.samples; i++, idx += inc)
  {
    x = ((beatflower.sine_table[i] * (beatflower_scope_offset(data[idx]) + 32768)) >> 16) + (beatflower.width >> 1);
    y = ((beatflower.cosine_table[i] * (beatflower_scope_offset(data[idx]) + 32768)) >> 16) + (beatflower.height >> 1);

    beatflower.pixels[y * (beatflower.pitch >> 2) + x + 1] = beatflower.color;
    beatflower.pixels[y * (beatflower.pitch >> 2) + x - 1] = beatflower.color;
    beatflower.pixels[(y + 1) * (beatflower.pitch >> 2) + x] = beatflower.color;
    beatflower.pixels[(y - 1) * (beatflower.pitch >> 2) + x] = beatflower.color;
  }
}

