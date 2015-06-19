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

// so that we can do some image augmentation
#include <opencv2/highgui/highgui.hpp>

class NeonTest
{
public:
    NeonTest()
    {
        // a char array
        _buf = new unsigned char[BUF_SIZE];
        for (int i = 0; i < NCHANNELS; i++) {
        // for as many times as there are channels
        // TODO: what is going on here? where is out declared?
        // what does cv::Mat do?
        // where did all these other constants get declared
            _out[i] = new cv::Mat(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC1,
                                  _buf + FRAME_SIZE * i);
        }
    }

    ~NeonTest()
    {
        // destructor
        delete[] _buf;
        for (int i = 0; i < NCHANNELS; i++) {
            delete _out[i];
        }
    }

    float predict()
    {
        // length 2 array of floats
        float result[2];
        // send buf to the server
        // where buf is the unsigned char array
        _client.send(_buf);
        // then get back the response
        // which presumably is a classification
        // of some sort
        _client.receive((unsigned char*) result);
        // now return result, which was 
        // written into by the server
        return result[0];
    }

    void read(std::string filename)
    {
        // declare left, since we will look for that in file
        std::string token("left");
        // pos is wherever we find left in this file
        size_t pos = filename.find(token);
        // assert that we actually found it
        assert(pos != std::string::npos);
        // TODO are we just replacing left with depth?
        // what is going on here
        std::string depthname = filename.replace(pos,
             token.length(), "depth");
        //TODO what is going on here?
        // we're declaring some sort of opencv struct
        cv::Mat bgr = cv::imread(filename, CV_LOAD_IMAGE_COLOR);
        // checking that our image we read in has the right number of rows
        assert(bgr.rows == IMAGE_HEIGHT);
        assert(bgr.cols == IMAGE_WIDTH);
        // now we're reading some other thing - what is it?
        cv::Mat depth = cv::imread(depthname, CV_LOAD_IMAGE_GRAYSCALE);
        //now we're putting some sort of image data into buf
        mix(bgr, depth, _buf, IMAGE_WIDTH * IMAGE_HEIGHT);
    }

private:
    void mix(cv::Mat& bgr, cv::Mat& depth, unsigned char* buf, int len)
    {
        // this is called at the end of read
        //TODO: understand how this is working
        //now we mix together somehow the two things that we've read in
        cv::Mat in[] = {bgr, depth};
        int from_to[] = {0,2, 1,1, 2,0, 3,3};
        cv::Mat out[] = {*_out[0], *_out[1], *_out[2], *_out[3]};
        cv::mixChannels(in, 2, out, 4, from_to, 4);
    }

private:
    // AHH, so this is where we declare all those constanst
    static const int    NCHANNELS = 4;
    static const int    IMAGE_HEIGHT = 256;
    static const int    IMAGE_WIDTH = 256;
    // so frame size is 2^16 = 65536 
    static const int    FRAME_SIZE = IMAGE_HEIGHT * IMAGE_WIDTH;
    // and buf size is 4*65536, so the buf is just for one image here
    static const int    BUF_SIZE = NCHANNELS * FRAME_SIZE;

private:
    // buff is just a char pointer
    unsigned char*      _buf;
    // this is the client, so the server is declared in python
    Shmem::Client       _client;
    // what the hell is this? TODO
    cv::Mat*            _out[NCHANNELS];
};

int main(int argc, char** argv)
{
    if (argc < 2) {
        // it takes one argument, which is the filename
        // TODO: what does it do to the argument
        std::cout << "Usage: " << argv[0] << " filename" << std::endl;
        return 0;
    }

    // filename is the name we pass in
    std::string filename = argv[1];

    // now we declare a NeonTest
    NeonTest test;

    //then we call read on that filename
    // this puts the right stuff into buf
    // presumably in the format that python needs
    test.read(filename);

    // then we get the prediction out
    float result = test.predict();
     
    std::cout << "result " << result << std::endl;
    return 0;
}
