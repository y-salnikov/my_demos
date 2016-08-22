compiler= gcc
strip= strip
cflags= -O2 -pipe -Wall -march=native -std=c99
includes=`sdl2-config --cflags`
libdir=`sdl2-config --libs`
link=  -lSDL2_mixer
#-lglut -lmikmod
target=psg_player

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
	
	
	



