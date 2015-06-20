/*
 Copyright 2015 Nervana Systems Inc.
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/
#include <stdio.h>
#include <stdlib.h>

// implements sem_close, sem_wait, etc
#include <semaphore.h>

// the venerable assert
#include <assert.h>

// file control options
#include <fcntl.h>

// provides accesss to posix os api
// fork, pipe, read, write, close, etc
#include <unistd.h>

// c strings
#include <string.h>

// typedef int32_t and stuff like that
#include <sys/types.h>


#include <sys/stat.h>
#include <sys/mman.h>

#include <string>

#include <iostream>
/*
  overall this is very similar to a C++ version of everything in shmem.py
  this makes sense, because it's just allowing us to use C++ to write to
  a Message channel as defined in that file
 */
namespace Shmem
{
  // same classes as in shmem.py
    class Message;
    class Endpoint;
    class Server;
    class Client;
}

class Shmem::Message
{
public:
    // takes a suffix that will specify the name of the shmem region
    Message(std::string& suffix)
    : _shmem(0), _empty_sem(0), _fill_sem(0), _mutex(0), _data_size(0)
    {
      //sets the names as in shmem.py
        _shmem_name = "/neon-shmem-" + suffix;
        _empty_sem_name = "/neon-empty-sem-" + suffix;
        _fill_sem_name = "/neon-fill-sem-" + suffix;
        _mutex_name = "/neon-mutex-" + suffix;

    }

    void create(int data_size)
    {

        std::cout << "CREATE HAS BEEN CALLED" << std::endl;
        // initializes the data
        _data_size = data_size;

        //shmfd will point to an shmem file descriptor
        int shmfd = shm_open(_shmem_name.c_str(),
             O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

        // this will throw if we couldn't open
        if (shmfd < 0) {
            perror("shm_open");
            exit(1);
        }

        // truncate the shmem to data size
        int ret = ftruncate(shmfd, _data_size);
        
        // exit if the truncation failed
        if (ret == -1) {
            perror("ftruncate");
            exit(1);
        }

        // declare empty sem with initial value 1 
        // (since the region starts empty)
        _empty_sem = sem_open(_empty_sem_name.c_str(), O_CREAT, 1);
        
        // exit if we couldn't allocate the sem
        if (_empty_sem == SEM_FAILED) {
            perror("sem_open");
            exit(1);
        }

        // do the same stuff with the fill semaphore 
        // and with the mutex
        _fill_sem = sem_open(_fill_sem_name.c_str(), O_CREAT, 0);
        if (_fill_sem == SEM_FAILED) {
            perror("sem_open");
            exit(1);
        }
        _mutex = sem_open(_mutex_name.c_str(), O_CREAT, 1);
        if (_mutex == SEM_FAILED) {
            perror("sem_open");
            exit(1);
        }


        // let shmem be the mmap of the shmfd file descriptor
        _shmem = (unsigned char*) mmap(0, _data_size,
             PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
        if (_shmem == 0) {
            perror("mmap");
            exit(1);
        }

        // close the fd - we already mapped it
        // do we actually want to do this?
        ::close(shmfd);
    }

    void destroy()
    {
        // close an unlink everything
        close();
        shm_unlink(_shmem_name.c_str());
        sem_unlink(_empty_sem_name.c_str());
        sem_unlink(_fill_sem_name.c_str());
        sem_unlink(_mutex_name.c_str());
    }

    void open()
    {
        // open existing shmem file descriptor
        int shmfd = shm_open(_shmem_name.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
        if (shmfd < 0) {
            perror("shm_open");
            exit(1);
        }

        // declare a stat struct
        // and write info about shmfd into it
        struct stat sb;
        fstat(shmfd, &sb);
        // this info includes the size, which we need to know
        _data_size = sb.st_size;

        // try opening all the sems and the mutex
        _empty_sem = sem_open(_empty_sem_name.c_str(), 0);
        if (_empty_sem == SEM_FAILED) {
            perror("sem_open");
            exit(1);
        }
        _fill_sem = sem_open(_fill_sem_name.c_str(), 0);
        if (_fill_sem == SEM_FAILED) {
            perror("sem_open");
            exit(1);
        }
        _mutex = sem_open(_mutex_name.c_str(), 0);
        if (_mutex == SEM_FAILED) {
            perror("sem_open");
            exit(1);
        }


        // mmap the file descriptor
        _shmem = (unsigned char*) mmap(0, _data_size,
             PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
        if (_shmem == 0) {
            perror("mmap");
            exit(1);
        }

        // close the descriptor since we've already mmmapped it
        ::close(shmfd);
    }

    void close()
    {
        // close the sems and the mutex
        sem_close(_empty_sem);
        sem_close(_fill_sem);
        sem_close(_mutex);
    }

    void send(unsigned char* data)
    {
        // acquire empty, 
        sem_wait(_empty_sem);
        // take the lock
        sem_wait(_mutex);
        // do the writing
        memcpy(_shmem, data, _data_size);
        // give the lock back
        sem_post(_mutex);
        // give fill back
        sem_post(_fill_sem);
        // now this msg can be received
    }

    void receive(unsigned char* buf)
    {
        // take the fill sem
        sem_wait(_fill_sem);
        // take the lock
        sem_wait(_mutex);
        // copy the info out of shmem into our buf
        memcpy(buf, _shmem, _data_size);
        // give back the lock
        sem_post(_mutex);
        // post to empty sem
        sem_post(_empty_sem);
        // now this region can be written into again
    }


private:
    // all the same stuff as in shmem.py
    std::string         _shmem_name;
    std::string         _empty_sem_name;
    std::string         _fill_sem_name;
    std::string         _mutex_name;
    sem_t*              _empty_sem;
    sem_t*              _fill_sem;
    sem_t*              _mutex;
    int                 _data_size;
    unsigned char*      _shmem;
};

class Shmem::Endpoint
{
public:
    // endpoint is parent class of 
    // both Server and Client
    Endpoint()
    : _req_name("req"), _res_name("res")
    {
        _request = new Message(_req_name);
        _response = new Message(_res_name);
    }

    ~Endpoint()
    {
        // kill these on destructor call
        delete _request;
        delete _response;
    }

protected:
    Message*        _request;
    Message*        _response;
    std::string     _req_name;
    std::string     _res_name;
};

class Shmem::Server : public Endpoint
{
public:
    // server actually does all creation 
    // and destruction of resources
    Server(int req_size, int res_size)
    : Endpoint(), _req_size(req_size), _res_size(res_size)
    {
        start();
    }

    void start()
    {
        std::cout << "start has been called" << std::endl;
        _request->create(_req_size);
        _response->create(_res_size);
    }

    void stop()
    {
        _request->destroy();
        _response->destroy();
    }

    void send(unsigned char* data)
    {
        // when server sends, 
        // it sends a response
        _response->send(data);
    }

    void receive(unsigned char* buf)
    {
        // when server recvs,
        // it recvs a request
        _request->receive(buf);
    }

private:
    int         _req_size;
    int         _res_size;
};

class Shmem::Client : public Endpoint
{
public:
    // basically the inverse of the server
    // and it only opens and closes resources
    // rather than creating and destorying them
    Client()
    : Endpoint()
    {
        start();
    }


    void start()
    {
        _request->open();
        _response->open();
    }

    void stop()
    {
        _request->close();
        _response->close();
    }

    void send(unsigned char* data)
    {
        _request->send(data);
    }

    void receive(unsigned char* buf)
    {
        _response->receive(buf);
    }
};
