all: main.cpp

main.cpp: libs
	g++ main.cpp -Iinclude -Llibs -lserver -lutils -lws2_32 -o Server
	rm -rf server.o utils.o
server.o:
	g++ src/server.cpp -Iinclude -Llibs -lws2_32 -c
utils.o:
	g++ src/utils.cpp -Iinclude -Llibs -lws2_32 -c

libs: server.o utils.o
	ar rvs libs/libutils.a utils.o
	ar rvs libs/libserver.a server.o
	rm -rf server.o utils.o Server.exe

remove:
	rm -rf server.o utils.o Server.exe

removelibs:
	rm -rf libs/libutils.a libs/libserver.a Server.exe