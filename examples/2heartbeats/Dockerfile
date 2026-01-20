FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    pkg-config \
    libyaml-cpp-dev \
    nlohmann-json3-dev \
    g++ \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /ionet

# Copy the entire IONet project into the container
COPY ../../ .

# Build all targets (including ionet and 2heartbeats)
RUN cmake -B build
RUN cmake --build build

# Copy the 2heartbeats binary to a convenient location
RUN cp build/examples/2heartbeats/2heartbeats /ionet/2heartbeats

CMD ["/ionet/2heartbeats"]
