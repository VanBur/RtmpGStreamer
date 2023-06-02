docker-build:
	docker build -t vanbur/rtmp_streamer:latest -f build/Dockerfile .

docker-run-test:
	docker run --rm --name=streamer_test vanbur/rtmp_streamer make run-tests

docker-build-streamer:
	docker run --rm --name=streamer_test vanbur/rtmp_streamer make build