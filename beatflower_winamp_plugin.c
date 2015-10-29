#include "beatflower_winamp.h"
#include "beatflower.h"

static void beatflower_winamp_config(struct winampVisModule *this_mod);
static int beatflower_winamp_init(struct winampVisModule *this_mod);
static int beatflower_winamp_redraw(struct winampVisModule *this_mod);
static void beatflower_winamp_quit(struct winampVisModule *this_mod);

// Plugin vars
char szAppName[] = "beatflower";

// Module header, includes version, description, and address of the module retriever function
winampVisHeader hdr = { VIS_HDRVER, "beatflower v"VERSION, getModule };

// module (soundscape)
winampVisModule mod1 = {
  "beatflower",
  NULL,   // hwndParent
  NULL,   // hDllInstance
  0,      // sRate
  0,      // nCh
  25,     // latencyMS
  25,     // delayMS
  2,      // spectrumNch
  2,      // waveformNch
  { {0}, }, // spectrumData
  { {0}, }, // waveformData
  beatflower_winamp_config,
  beatflower_winamp_init,
  beatflower_winamp_redraw,
  beatflower_winamp_quit
};

/************************************
 *   Function Def's                 *
 ************************************/
// this is the only exported symbol. returns our main header.
// if you are compiling C++, the extern "C" { is necessary, so we just #ifdef it
#ifdef __cplusplus
extern "C" {
#endif
__declspec( dllexport ) winampVisHeader *winampVisGetHeader() {return &hdr;}
#ifdef __cplusplus
}
#endif


// getmodule routine from the main header. Returns NULL if an invalid module was requested,
// otherwise returns either mod1, mod2 or mod3 depending on 'which'.
winampVisModule* getModule(int which) {
  if (which == 0) {
    return &mod1;
  } else {
    return NULL;
  }
}

static void
beatflower_winamp_log(const char *format, ...)
{
}


static void
beatflower_winamp_config_load(beatflower_config_t *cfg)
{
  cfg->fullscreen = 0;

  cfg->width = 400;
  cfg->height = 400;
  cfg->color_mode = 1;
  cfg->color1 = 0x80a030;
  cfg->color2 = 0x20f040;
  cfg->color3 =  0xa01cb2;
  cfg->draw_mode = 1;
  cfg->samples_mode = 1;
  cfg->amplification_mode = 1;
  cfg->offset_mode = 1;
  cfg->decay = 1;
  cfg->factor = 0.874;
  cfg->angle = 0.2;
  cfg->blur = TRUE;
  cfg->zoombeat = TRUE;
  cfg->rotatebeat = TRUE;


  beatflower_config_loaded = TRUE;
}


static void
beatflower_winamp_config(struct winampVisModule *this_mod) {
}

static int
beatflower_winamp_init(struct winampVisModule *this_mod) {

  SDL_LockMutex(beatflower_config_mutex);

  if(!beatflower_config_loaded)
    beatflower_winamp_config_load(&beatflower_config);

  SDL_UnlockMutex(beatflower_config_mutex);

  beatflower_log = &beatflower_winamp_log;
  beatflower_init();
//  beatflower_start();
  return 0;
}

static int
beatflower_winamp_redraw(struct winampVisModule *this_mod) {
  return 0;
}

static void
beatflower_winamp_quit(struct winampVisModule *this_mod) {
}

