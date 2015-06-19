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
    NeonTest()
    {
        // a char array
        _buf = new unsigned char[BUF_SIZE];
    }

    ~NeonTest()
    {
        // destructor
        delete[] _buf;
    }

    float predict()
    {
        char result[1];
        _client.send(_buf);
        _client.receive((unsigned char*) result);
        return result[0];
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
    Shmem::Client       _client;
};

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cout << "Usage: " 
                  << argv[0] 
                  << " <char> " 
                  << std::endl;
        return 0;
    }

    // filename is the name we pass in
    char c = argv[1][0];

    // now we declare a NeonTest
    NeonTest test;

    //then we call read on that filename
    // this puts the right stuff into buf
    // presumably in the format that python needs
    test.read(c);

    // then we get the prediction out
    float result = test.predict();
     
    std::cout << "result " << result << std::endl;
    return 0;
}
