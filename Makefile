prefix = /usr

CC = gcc
INSTALL = install

LDFLAGS = -shared -rdynamic

PACKAGE = beatflower
VERSION = 1.0

xmms_visualization_plugin_dir = $(shell xmms-config --visualization-plugin-dir)

DEFS = -DPACKAGE=\"$(PACKAGE)\" -DVERSION=\"$(VERSION)\"

SDL_CFLAGS = $(shell sdl-config --cflags)
SDL_LIBS = $(shell sdl-config --libs)

XMMS_CFLAGS = $(shell xmms-config --cflags)
XMMS_LIBS = $(shell xmms-config --libs)

XMMS_PLUGIN = libbeatflower.so

SOURCES = beatflower.c
OBJECTS = $(SOURCES:%.c=%.o)
TARGETS = $(XMMS_PLUGIN)

$(TARGETS): CPPFLAGS += $(DEFS)
$(TARGETS): CFLAGS += -fPIC 

all: $(TARGETS) 

beatflower.o: CFLAGS += $(XMMS_CFLAGS) $(SDL_CFLAGS)

$(XMMS_PLUGIN): LIBS += $(XMMS_LIBS) $(SDL_LIBS)
$(XMMS_PLUGIN): beatflower.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	$(RM) $(OBJECTS)

install:
	$(INSTALL) -d $(DESTDIR)$(xmms_visualization_plugin_dir)
	$(INSTALL) -m 755 $(XMMS_PLUGIN) $(DESTDIR)$(xmms_visualization_plugin_dir)

