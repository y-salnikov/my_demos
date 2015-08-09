compiler= gcc
strip= strip
cflags= -Os -pipe -Wall -march=native 
includes=`sdl-config --cflags` -I /usr/X11R6/include
libdir=`sdl-config --libs` -L /usr/X11R6/lib
link= -lm  -lGL -lGLU -lSDL_mixer
#-lglut -lmikmod
target=magic_olic

source_files=$(wildcard *.c)
object_files=$(patsubst %.c,%.o,$(source_files))

$(target): $(object_files)
	$(compiler) $(cflags) $^ $(libdir) $(link) -o $@

%.o: %.c
	$(compiler) $(cflags) $(includes)  -c $<
	
clean: 
	rm -f $(target)
	rm -f $(object_files)
	
pack:
	$(strip) $(target)
	mv $(target) tmp_file
	upx -9 -o $(target) tmp_file
	rm	-f tmp_file
	
	
	



