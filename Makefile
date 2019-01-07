CCPP=g++
CPPFLAGS=-c -Wall -O2 -std=c++14
CFLAGS=-c -Wall -O2

LIBS=-lGLEW -lGL -lglfw
LDFLAGS=
DSFLAGS=-DNDEBUG

SOURCES= \
		src/gl_utils.cpp \
		src/parabola.cpp \
		src/args_parser.cpp \
		src/sdf_gl.cpp \
		src/glyph_painter.cpp \
		src/sdf_atlas.cpp \
		src/font.cpp \
		src/main.cpp

VPATH=$(dir $(SOURCES))

OBJECTS=$(addsuffix .o, $(basename $(SOURCES)))

BINDIR=./bin/
BINDEST=$(addprefix $(BINDIR), $(notdir $(OBJECTS)))

DEPNAMES = $(addsuffix .d, $(basename $(SOURCES)))
DEPS     = $(addprefix $(BINDIR), $(notdir $(DEPNAMES)))

EXECUTABLE=./bin/sdf_atlas

all: bindir $(EXECUTABLE)

$(EXECUTABLE): $(BINDEST)
	$(CCPP) $(LDFLAGS) $(BINDEST) $(LIBS) -o $@

$(BINDIR)%.o:%.cpp
	$(CCPP) $(CPPFLAGS) $(DSFLAGS) -MMD $< -o $(addprefix $(BINDIR), $(notdir $@))

.PHONY: all bindir clean

bindir:
	test -d $(BINDIR) || mkdir $(BINDIR)

clean:
	rm ./bin/*

-include $(DEPS)
