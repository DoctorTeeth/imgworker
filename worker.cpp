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
#include "shmem.hpp"

#include <iostream>
#include <string>

class NeonTest
{
public:
    NeonTest() : _server(1,1)
    {
        // a char array
        _buf = new unsigned char[BUF_SIZE];
    }

    ~NeonTest()
    {
        // destructor
        delete[] _buf;
    }

    void serve()
    {
        std::cout << "waiting to serve" << std::endl; 
        char result[1];
        _server.receive((unsigned char*) result);
        std::cout << "recv has fired" << std::endl; 
        _buf[0] = result[0];
        _server.send(_buf);
        std::cout << "send has fired" << std::endl; 
    }

    void start()
    {
        // i think that these things get started
        // automatically
        _server.start();
    }

    void read(char c)
    {
        _buf[0] = c;
    }

private:
    static const int    BUF_SIZE = 1;

private:
    // buff is just a char pointer
    unsigned char*      _buf;
    Shmem::Server  _server;
};

int main(int argc, char** argv)
{
    if (argc < 1) {
        std::cout << "Usage: " 
                  << argv[0] 
                  << std::endl;
        return 0;
    }
    
    std::cout << "Worker server started" << std::endl;

    NeonTest test;
    //test.start();

    while(true){
        test.serve();
    }

    return 0;
}
