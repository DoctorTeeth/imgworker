#Build client application that communicates with neon over shared memory. 
all: worker.cpp shmem.hpp
	g++ -O3 -o worker worker.cpp  -lrt -pthread $$(pkg-config opencv --cflags --libs)

clean: 
	rm worker 
