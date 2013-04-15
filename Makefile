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
	gl_util.cc \
	image.cc \
	in_game_state.cc \
	kashi.cc \
	main.cc \
	ogg_player.cc \
	panic.cc \
	spectrum_bars.cc \
	utf8.cc \

TARGET = typomania

.cc.o:
	$(CXX) $(CXXFLAGS) -c $<

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@ $(LIBS)

clean:
	rm -f *o $(TARGET)
