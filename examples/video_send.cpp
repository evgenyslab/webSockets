#include "opencv2/opencv.hpp"
#include <uWServer_b.h>
#include <thread>
#include <iostream>
#include <random>
#include <msgpack.hpp>
#include <fstream>

using namespace cv;

int main(int, char**)
{
    VideoCapture cap(0); // open the default camera
    if(!cap.isOpened())  // check if we succeeded
        return -1;

    Mat edges;
//    namedWindow("edges",1);

    uWServer_b server(13049);
    server.run();

    msgpack::sbuffer streamBuffer;  // stream buffer
    // create a key-value pair packer linked to stream-buffer
    msgpack::packer<msgpack::sbuffer> packer(&streamBuffer);

    for(;;)
    {
        Mat frame, sframe;
        std::vector<uchar> buf;
        cap >> frame; // get a new frame from camera
        cvtColor(frame, edges, COLOR_BGR2GRAY);
        // resize image:
        resize(frame,sframe, Size((int) frame.cols/2, (int) frame.rows/2));
        imencode(".jpeg", sframe, buf);
        // need to conver uchar to char...
        std::string r(buf.begin(), buf.end());
        streamBuffer.clear();
        packer.pack_map(1);
        packer.pack("image_binary");
        packer.pack_bin(r.size());
        packer.pack_bin_body(r.data(), r.size());
        server._send(streamBuffer.data(), streamBuffer.size()); // streamsize isnt correct yet...
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}