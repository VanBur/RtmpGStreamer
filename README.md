# RtmpGStreamer

GStreamer-based rtmp streamer 

## Usage examples: 

For file streaming: ```streamer 'path/to/file.mp4' 'rtmp://127.0.0.1/path/to/rtmp/endpoint'```

For testsrc streaming: ```streamer 'rtmp://127.0.0.1/path/to/rtmp/endpoint'```

## Commands:
	'play' - run streamer
	'pause' - pause streamer
	'exit' - terminate streamer

## Docker

Dockerfile can be used by path: "build/Dockerfile"

## Makefile

For usability you can use Makefile:
- build docker image: ```make docker-build```
- run tests in docker: ```make docker-run-test```
- build rtmp streamer in docker: ```make docker-build-streamer```

Enjoy!