#Build client application that communicates with neon over shared memory. 
all: client.cpp shmem.hpp
	g++ -O3 -o client client.cpp  -lrt -pthread $$(pkg-config opencv --cflags --libs)
