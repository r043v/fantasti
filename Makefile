# generic
CFLAGS := -Wall -g -O3 `pkg-config --cflags icu-uc icu-io` `pkg-config --cflags freetype2` -pipe -fstack-protector-strong -fno-plt
# gameshell
#CFLAGS := -Wall -g -O3 `pkg-config --cflags icu-uc icu-io` `pkg-config --cflags freetype2` -mcpu=cortex-a7 -mfloat-abi=hard -mfpu=neon-vfpv4 -pipe -fstack-protector-strong -fno-plt

CC := g++
#CFLAGS := -Wall -g -O3 `pkg-config --cflags icu-uc icu-io` `pkg-config --cflags freetype2`
TARGET := fantasti

# $(wildcard *.cpp /xxx/xxx/*.cpp): get all .cpp files from the current directory and dir "/xxx/xxx/"
SRCS := ./Gdl/input/key.cpp \
./Gdl/misc/misc.cpp \
./Gdl/misc/fileio.cpp \
./Gdl/misc/log.cpp \
./Gdl/misc/freelist.cpp \
./Gdl/misc/rate.cpp \
./Gdl/output/graphic/text.cpp \
./Gdl/output/graphic/fbuffer/fbuffer.cpp \
./Gdl/output/graphic/console/console.cpp \
./Gdl/output/graphic/_anim.cpp \
./Gdl/output/graphic/map/omap.cpp \
./Gdl/output/graphic/scene/scene.cpp \
./Gdl/output/graphic/gfm/Gfm.cpp \
./Gdl/output/graphic/miscgfx.cpp \
./main/list.cpp \
./main/rfkill.cpp \
./main/explorer.cpp \
./main/pid.cpp \
./main/cpupower.cpp \
./main/fantasti.cpp
#./main/lapinou.cpp \
#./main/lapin.cpp \
#./main/dstar.cpp \
#./main/entity.cpp \
#./apt/apt.cpp \

# # $(patsubst %.cpp,%.o,$(SRCS)): substitute all ".cpp" file name strings to ".o" file name strings
OBJS := $(patsubst %.cpp,%.o,$(SRCS))

#  -lefence
all: $(TARGET)
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ -lzip -liw -lSDL -lSDL_mixer `pkg-config --libs icu-uc icu-io` `pkg-config --libs freetype2`
%.o: %.S
	$(NASM) $(NASMFLAGS) $< -o $@
%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $<
clean:
	rm -rf $(TARGET)
	find . -iname '*.o' -delete
#
.PHONY: all clean

ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

.PHONY: install
install: fantasti
	install -d $(DESTDIR)$(PREFIX)/bin/
	cp $< $(DESTDIR)$(PREFIX)/bin/fantasti
	install -d $(DESTDIR)$(PREFIX)/share/fantasti/
	cp *.ttf $(DESTDIR)$(PREFIX)/share/fantasti/

.PHONY: uninstall
uninstall:
	rm -rf $(DESTDIR)$(PREFIX)/share/fantasti
	rm -f $(DESTDIR)$(PREFIX)/bin/fantasti
