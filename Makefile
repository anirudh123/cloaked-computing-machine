CC=clang
CFLAGS=-I.
DEPS = 

tsh.o: tsh.c
	cc -o tsh.o -c tsh.c

read_profile.o: read_profile.c
	cc -o read_profile.o -c read_profile.c

file_handle.o: file_handle.c
	cc -o file_handle.o -c file_handle.c

l_list.o: l_list.c
	cc -o l_list.o -c l_list.c

calc.o: calc.c
	cc -o calc.o -c calc.c

myshell: tsh.o read_profile.o file_handle.o l_list.o calc.o
		cc -o myshell tsh.o read_profile.o file_handle.o l_list.o calc.o -I.

clean: tsh.o read_profile.o file_handle.o l_list.o calc.o
		rm tsh.o read_profile.o file_handle.o l_list.o calc.o
