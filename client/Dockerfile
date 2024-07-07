FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
	build-essential locales libncurses5-dev libncursesw5-dev \
	&& rm -rf /var/lib/apt/lists/*
ARG LCL
WORKDIR /client
COPY ./client /client
COPY include/customerr.h /client/src
RUN gcc -o client src/*.c -lncursesw
RUN locale-gen ${LCL}

CMD ["bash", "-c", "COLUMNS=$(tput cols) && LINES=$(tput lines) && ./client"]