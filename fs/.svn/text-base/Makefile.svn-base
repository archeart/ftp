headers= fs_struct.h fs_interface.h types.h
CC= gcc

../lib/libfs.a: fs_func.o
	ar -r ../lib/libfs.a fs_func.o

#used for test, with `make test`
test: main.c $(headers)
	make
	$(CC) main.c -L../lib -lfs -o pro.exe

fs_func.o: fs_func.c $(headers)
	$(CC) -c fs_func.c -o fs_func.o

clean:
	rm *.o
	rm ../lib/libfs.a
	#rm pro
