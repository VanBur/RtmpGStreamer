#include <iostream>
#include <thread>
#include <gst/gst.h>

#include "RtmpStreamer/RtmpStreamer.hpp"

using namespace std;

void help(){
    cout << "RTMP streamer from VanBur.\n\
\n\
Usage examples: \n\
For file streaming:\n\tstreamer 'path/to/file.mp4' 'rtmp://127.0.0.1/path/to/rtmp/endpoint'\n\
For testsrc streaming:\n\tstreamer 'rtmp://127.0.0.1/path/to/rtmp/endpoint'\n\
\n\
Commands:\n\
\t'play' - run streamer\n\
\t'pause' - pause streamer\n\
\t'exit' - terminate streamer\n\
\n\
Enjoy!\
" << endl;
}

RtmpStreamer *streamer;
const char *input, *output;

void runController(){
    bool inputIsOn = true;
    string inputCmd;
    
    while (inputIsOn) {
        cout << "Insert your command: ";
        cin >> inputCmd;
        
        if (inputCmd == "play"){
            cout << "OK." << endl;
            streamer->Play();
        } else if (inputCmd == "pause"){
            cout << "OK." << endl;
            streamer->Pause();
        } else if (inputCmd == "exit"){
            cout << "OK." << endl;
            inputIsOn = false;
            streamer->Stop();
            streamer->~RtmpStreamer();
        } else {
            cout << " ... unknown command. Try again." << endl;
        }
    }
}

void runStreamer(){
    streamer = new RtmpStreamer(input,output);
    streamer->Lounch();
}

int main(int argc, const char * argv[]) {
    switch (argc) {
        case 1:
            help();
            return 0;
            break;
        case 2:
            output = argv[1];
            break;
        case 3:
            input = argv[1];
            output = argv[2];
            break;
        default:
            break;
    }
    
    // run streamer
    std::thread streamerThread(runStreamer);
    // run controller
    std::thread controllerThread(runController);
    streamerThread.join();
    controllerThread.detach();
    
    cout << "Terminating ..." << endl;
    return 0;
}
