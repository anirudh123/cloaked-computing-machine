CC=clang
CFLAGS=-I.
DEPS = 

tsh.o: tsh.c
	cc -o tsh.o -c tsh.c

read_profile.o: read_profile.c
	cc -o read_profile.o -c read_profile.c

myshell: tsh.o read_profile.o 
		cc -o myshell tsh.o read_profile.o -I.

clean: tsh.o read_profile.o 
		rm tsh.o read_profile.o
