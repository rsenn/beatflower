prefix = /usr

CC = gcc
INSTALL = install
STRIP = strip

DEBUG = 0

ifeq ($(DEBUG),1)
CFLAGS := -g -ggdb -O0
else
CFLAGS := -g -O2 -Wall
endif
LDFLAGS = -shared -rdynamic

PACKAGE = beatflower
VERSION = 1.0

xmms_visualization_plugin_dir := $(shell xmms-config --visualization-plugin-dir)
xmms_visualization_plugin_dirs := $(subst lib64,lib,$(xmms_visualization_plugin_dir))

ifneq ($(xmms_visualization_plugin_dir),$(xmms_visualization_plugin_dirs))
xmms_visualization_plugin_dirs += $(xmms_visualization_plugin_dir)
endif

DEFS = -DPACKAGE=\"$(PACKAGE)\" -DVERSION=\"$(VERSION)\"

SDL_CFLAGS = $(shell sdl-config --cflags)
SDL_LIBS = $(shell sdl-config --libs)

XMMS_CFLAGS = $(shell xmms-config --cflags)
XMMS_LIBS = $(shell xmms-config --libs)

XMMS_PLUGIN = libbeatflower.so

SOURCES = beatflower.c beatflower_xmms_plugin.c beatflower_xmms_settings.c beatflower_xmms_config.c beatflower_renderer_sdl.c
OBJECTS = $(SOURCES:%.c=%.o)
HEADERS = beatflower.h
TARGETS = $(XMMS_PLUGIN)

#$(TARGETS): CPPFLAGS += $(DEFS)
$(TARGETS): CFLAGS += -fPIC 

all: $(TARGETS) 

.c.o:
	$(CC) $(CPPFLAGS) $(DEFS) $(CFLAGS) -c $<

beatflower_renderer_sdl.o: CFLAGS += $(SDL_CFLAGS)
beatflower.o beatflower_xmms_settings.o beatflower_xmms_config.o beatflower_xmms_plugin.o: CFLAGS += $(XMMS_CFLAGS) 

$(XMMS_PLUGIN): LIBS += $(XMMS_LIBS) $(SDL_LIBS)
$(XMMS_PLUGIN): $(OBJECTS)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)
ifneq ($(DEBUG),1)
	$(STRIP) --strip-unneeded $@
endif
clean:
	$(RM) $(OBJECTS)

install: all
	@for dir in $(xmms_visualization_plugin_dirs); do \
	  CMD="$(INSTALL) -d $(DESTDIR)$$dir; $(INSTALL) -m 755 $(XMMS_PLUGIN) $(DESTDIR)$$dir"; \
		echo -e "$$CMD" 1>&2; eval "$$CMD"; \
  done

astyle:
	astyle \
		--style=bsd \
		--options=none \
		--indent=spaces=2 \
		--unpad-paren \
		--break-closing-brackets \
		--keep-one-line-blocks \
		--keep-one-line-statements \
		--indent-preprocessor \
		--indent-cases \
		--break-blocks=all \
		$(SOURCES)
		