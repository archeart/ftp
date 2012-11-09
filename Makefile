all: ftp-server_fake ftp-client_fake

ftp-server_fake: ftp-server.c sthread.o user_mgr.o ftp.h msg.h
	cd map&&make
	cd auth&&make
	cd fs&&make
	gcc -D_REENTRANT -I/usr/include/nptl \
		ftp-server.c 	\
		sthread.o 		\
		user_mgr.o		\
		-o ftp-server 	\
		-L./lib -lauth -lmap -lfs\
		-lpthread

ftp-client_fake: ftp-client.c thread.o error.o ftp.h msg.h thread.h
	cd map&&make
	cd auth&&make
	gcc -D_REENTRANT -I/usr/include/nptl \
		ftp-client.c 	\
		thread.o		\
		error.o			\
		-o ftp-client	\
		-L./lib -lauth -lmap \
		-lpthread

thread.o: thread.c
	gcc -c thread.c 

sthread.o: sthread.c
	gcc -c sthread.c

user_mgr.o: user_mgr.c
	gcc -c user_mgr.c 

error.o: error.c
	gcc -c error.c 
	

clean:
	rm *.o
	rm ftp-client
	rm ftp-server
