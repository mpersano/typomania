CXX = g++
LD = g++
OBJS = $(CXXFILES:.cc=.o)
# DUMP_FRAMES = 1

LIBS = -lGL -lGLU `sdl-config --libs` -lpng -lopenal -logg -lvorbis -lvorbisfile

CXXFLAGS = `sdl-config --cflags` -Wall -g -O2

CXXFILES =  \
	fft.cc \
	font.cc \
	game.cc \
	gl_vertex_array.cc \
	gl_texture.cc \
	image.cc \
	in_game_state.cc \
	song_menu_state.cc \
	kashi.cc \
	main.cc \
	ogg_player.cc \
	panic.cc \
	spectrum_bars.cc \
	utf8.cc \

TARGET = typomania

all: depend $(TARGET)

.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@ $(LIBS)

depend: .depend

.depend: $(CXXFILES)
	rm -f .depend
	$(CXX) $(CXXFLAGS) -MM $^ > .depend;

clean:
	rm -f *o $(TARGET) .depend

include .depend

.PHONY: all clean depend
