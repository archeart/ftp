headers= auth_interface.h auth_utils.h types.h auth_struct.h
CC=gcc

../lib/libauth.a: auth_utils.o auth_func.o
	ar -r ../lib/libauth.a auth_utils.o auth_func.o

#`make test`
test: ../lib/libauth.a $(headers)
	cd ../map&&make ../lib/libmap.a
	$(CC) \
		-L../lib -lauth -lmap \
		-o pro.exe

auth_func.o: auth_func.c $(headers)
	$(CC) -c auth_func.c -o auth_func.o

auth_utils.o: auth_utils.c $(headers)
	$(CC) -c auth_utils.c -o auth_utils.o

clean:
	rm *.o
	#rm *.exe
	rm ../lib/libauth.a
	#cd ../map&&make clean
