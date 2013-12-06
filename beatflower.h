#ifndef BEATFLOWER_H__
#define BEATFLOWER_H__ 1

#include <pthread.h>

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

/*************************************** Externals ********************************************/
extern pthread_t beatflower_thread;

extern pthread_mutex_t beatflower_config_mutex;
extern pthread_mutex_t beatflower_data_mutex;
extern pthread_mutex_t beatflower_status_mutex;

extern config_t beatflower_config;
extern config_t beatflower_newconfig;

extern bool beatflower_config_loaded;
extern bool beatflower_finished;
extern bool beatflower_playing;
extern bool beatflower_reset;

extern gint16 beatflower_pcm_data[2][512];
extern gint16 beatflower_freq_data[2][256];

void config_set_defaults(config_t *beatflower_config);
void config_load(config_t *cfg);
void config_save(config_t *cfg);
void find_color(short data[2][256]);

void *beatflower_thread_function(void *blah);

#endif // BEATFLOWER_H__ 1

