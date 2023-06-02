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

## PoC for streaming RTMP to Web with sub-second latency (тут уже по-русски)

Есть такая мысль:
- стрим принимается на RTMP ендпойнт, который поднят на специальном микросервисе
- внутри RTMP поток демуксится на атомарные RTP стримы. Буффер на приемку - самый ми нимальный, так как нам важно отсутствие задержки (ну или ее минимизация)
- также под каждый такой RTMP стрим собирается SDP для WebRTC. Тут так как мы работаем в локальной сети - НАТ пробивать не надо, что облегчает интеграцию. Также H264 мы можем брать и транслировать как есть, минуя транскодинг в какой-нить VP8/VP9. Учитывая нюансы Хрома и Сафари - под некоторые браузеры нужно будет собирать свою SDP, так как есть нюансы на стороне вендоров браузеров
- на стороне веб-страницы организуем стандартную сессию между вышеупомянутой нодой по WebRTC и как только подключение произойдет - смотрим стрим.

Пулл языков и технологий:
- нужно написать что-то для бэка для приема, ремукса и работы с WebRTC. Есть неплохое решение на Го (https://github.com/pion), которое можно допилить под свои нужды. На нем не будет транскодинга, поэтому ничего сверху в виде FFmpeg/GStreamer не понадобится.
- на фронте - написать простую страницу с поддержкой WebRTC. Тут JavaScript видимо.
- для удобства - обернуть приемник RTMP в докер.

Плюсы данного решения:
- Есть уже почти готовое решение на беке
- WebRTC более универсальна при необходимости получения бОльшего покрытия клиентов на будущее (iOS/Android/etc.)

Enjoy!