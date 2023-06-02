build:
	g++ -Wall -o streamer main.cpp rtmp/streamer.cpp `pkg-config --cflags --libs gstreamer-1.0`

run-tests:
	g++ -Wall -o streamer_test rtmp/streamer.cpp test/streamer.cpp `pkg-config --cflags --libs gstreamer-check-1.0`
	./streamer_test
