FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
	build-essential sqlite3 libsqlite3-dev libsodium-dev \
	&& rm -rf /var/lib/apt/lists/*

WORKDIR /server
COPY ./server /server
COPY include/customerr.h /server/src
RUN gcc -o server src/*.c -lsqlite3 -lsodium

CMD ["./server"]