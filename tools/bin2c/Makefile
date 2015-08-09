compiler:= gcc
cflags:= -O2 -pipe
includes:= -I /usr/X11R6/include
libdir:= -L /usr/X11R6/lib
link:= -lbz2
target:= bin2c

source_files:=$(wildcard *.c)
object_files:=$(patsubst %.c,%.o,$(source_files))


$(target): $(object_files)
	$(compiler) $(cflags) $^ $(libdir) $(link) -o $@

%.o: %.c
	$(compiler) $(cflags) $(includes)  -c $<
	
clean: 
	rm -f $(target)
	rm -f $(object_files)



