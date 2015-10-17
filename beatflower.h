#ifndef BEATFLOWER_H__
#define BEATFLOWER_H__ 1

#include <stdint.h>
#include <pthread.h>

/************************************ Type definitions ****************************************/

typedef int bool;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif


/* configuration structure */
typedef struct config_s {
  bool         fullscreen;
  int          width;
  int          height;
  int32_t color1;
  int32_t color2;
  int32_t color3;
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
} beatflower_config_t;

typedef struct {
  int32_t color_table[512];
  unsigned int samples;
  void (*scope)(short data[512]);
  int32_t       width;
  int32_t       height;
  int32_t       sine_table[512];
  int32_t       cosine_table[512];
  int32_t       radius;
  uint32_t       color;
  uint32_t       pitch;
  uint32_t      *pixels;
  uint32_t     **transform_table;
  double       factor;
  double       angle;
  bool         blur_enable;
  uint32_t       color_mode;
  uint32_t       amp_mode;
  uint32_t       offset_mode;
  uint32_t       color1;
  uint32_t       color2;
  uint32_t       color3;
  uint32_t       decay;
} beatflower_state_t;

typedef void beatflower_log_function(const char *s, ...);

/*************************************** Externals ********************************************/
extern beatflower_config_t beatflower_config;
extern beatflower_log_function *beatflower_log;
extern pthread_t beatflower_thread;
extern pthread_mutex_t beatflower_status_mutex;
extern pthread_mutex_t beatflower_data_mutex;
extern pthread_mutex_t beatflower_config_mutex;
extern bool beatflower_config_loaded;
extern bool beatflower_playing;
extern bool beatflower_finished;
extern bool beatflower_reset;
extern int16_t beatflower_pcm_data[2][512];
extern int16_t beatflower_freq_data[2][256];
extern beatflower_state_t beatflower;

/********************************** Function prototypes ***************************************/

void beatflower_config_default(beatflower_config_t *cfg);
void beatflower_start(void);
int beatflower_scope_amplification(int value);
int beatflower_scope_offset(int value);
void beatflower_find_color(int16_t data[2][256]);
bool beatflower_check_finished(void);
bool beatflower_check_playing(void);

#endif // BEATFLOWER_H__ 1
