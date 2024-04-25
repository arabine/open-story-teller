# Docker image to cross-build C/C++ programs from a Linux host to a Win64 target
# Also includes nsis to create installer

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


# ========================================================================
# Build OpenSSL for windows
# Libraries are installed in /libs/
# ========================================================================

ENV OPENSSL_VERSION="3.0.13"

RUN mkdir -p /libs/openssl

RUN set -x \
    && wget --no-check-certificate -O /tmp/openssl-${OPENSSL_VERSION}.tar.gz "https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz" \
    && tar -xvf /tmp/openssl-${OPENSSL_VERSION}.tar.gz -C /tmp/ \
    && rm -rf /tmp/openssl-${OPENSSL_VERSION}.tar.gz \ 
    && cd /tmp/openssl-${OPENSSL_VERSION} \
    && ./Configure --cross-compile-prefix=x86_64-w64-mingw32- mingw64 --prefix=/libs/openssl \
    && make \
    && make install \
    && cd .. \
    && rm -rf openssl-${OPENSSL_VERSION}
 
 ENV PATH /libs/openssl/bin:$PATH
 
