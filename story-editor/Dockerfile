# Multi-stage Docker file
# 1) Developer environment
# 2) nginx stage to serve frontend assets
# This file must be run in the parent directory:
# docker build -t gridwatch-front -f dashboard/Dockerfile .

# =======================================================
# 1. Build stage, we need a node environment
# =======================================================


FROM ubuntu:22.04
LABEL Description="Developer environment"

ENV HOME /root

SHELL ["/bin/bash", "-c"]

RUN apt-get update && apt-get -y --no-install-recommends install \
    build-essential \
    cmake \
    nsis \
    mingw-w64 \
    git \
    wget

# Make sure to use the POSIX version of MinGW:
# Manual equivalent command is : update-alternatives --config x86_64-w64-mingw32-g++

RUN update-alternatives --set x86_64-w64-mingw32-g++ $(update-alternatives --list x86_64-w64-mingw32-g++ | grep posix)
RUN update-alternatives --set x86_64-w64-mingw32-gcc $(update-alternatives --list x86_64-w64-mingw32-gcc | grep posix)

RUN mkdir /workspace
