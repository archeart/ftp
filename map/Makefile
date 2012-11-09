headers=struct.h utils.h map_interface.h
CC=gcc
AR=ar

../lib/libmap.a: rbtree.o utils.o rbtree_delete.o $(headers)
	$(AR) -r ../lib/libmap.a \
		rbtree.o \
		utils.o \
		rbtree_delete.o

#`make test` for test
test:
	make
	$(CC) \
		main.c \
		-L../lib -lmap \
		-o pro.exe

utils.o: utils.c $(headers)
	$(CC) -c -o utils.o utils.c

rbtree.o: rbtree.c $(headers)
	$(CC) -c -o rbtree.o rbtree.c

rbtree_delete.o: rbtree_delete.c $(headers)
	$(CC) -c -o rbtree_delete.o rbtree_delete.c

clean:
	rm *.o
	rm ../lib/libmap.a
	#rm *.exe
