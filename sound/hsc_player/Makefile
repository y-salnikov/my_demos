compiler= gcc
cflags= -O2 -pipe -std=c99
includes=`sdl-config --cflags` -I /usr/X11R6/include
libdir=`sdl-config --libs` -L /usr/X11R6/lib
link= -lSDL_mixer -lm
target=hsc_player

source_files=$(wildcard *.c)
object_files=$(patsubst %.c,%.o,$(source_files))

default:	$(target)


$(target): $(object_files)
	$(compiler) $(cflags) $^ $(libdir) $(link) -o $@

%.o: %.c
	$(compiler) $(cflags) $(includes)  -c $<
	
clean: 
	rm -f $(target)
	rm -f $(object_files)

	
	
	



