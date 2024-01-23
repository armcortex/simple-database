FROM ubuntu:22.04

RUN apt-get update && \
    apt-get install -y git gcc g++ gdb cmake make ninja-build valgrind
RUN apt-get install -y curl kmod linux-tools-common linux-tools-generic
RUN apt-get clean && rm -rf /var/lib/apt/lists/*

# For profiler
RUN rm /usr/bin/perf && ln -s /usr/lib/linux-tools/5.15.0-91-generic/perf /usr/bin/perf

WORKDIR /app

CMD ["bash"]
