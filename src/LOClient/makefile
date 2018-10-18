all: main.cpp

main.cpp: libs
	g++ main.cpp -Iinclude -Llibs -lclient -lutils -lws2_32 -o Client
	rm -rf client.o utils.o
client.o:
	g++ src/client.cpp -Iinclude -Llibs -lws2_32 -c
utils.o:
	g++ src/utils.cpp -Iinclude -Llibs -lws2_32 -c

libs: client.o utils.o
	ar rvs libs/libutils.a utils.o
	ar rvs libs/libclient.a client.o
	rm -rf client.o utils.o Client.exe

remove:
	rm -rf client.o utils.o Client.exe

removelibs:
	rm -rf libs/libutils.a libs/libclient.a Client.exe