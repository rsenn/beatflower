#include "beatflower_winamp.h"

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
beatflower_winamp_config(struct winampVisModule *this_mod) {
}

static int
beatflower_winamp_init(struct winampVisModule *this_mod) {
  return 0;
}

static int
beatflower_winamp_redraw(struct winampVisModule *this_mod) {
  return 0;
}

static void
beatflower_winamp_quit(struct winampVisModule *this_mod) {
}

