FROM ubuntu:22.04
LABEL Description="Build environment"

ENV HOME /root

SHELL ["/bin/bash", "-c"]

RUN mkdir /workspace

RUN apt-get update && apt-get -y --no-install-recommends install \
    build-essential \
    cmake \
    nsis \
    mingw-w64 \
    git \
    wget
