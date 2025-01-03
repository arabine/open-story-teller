docker build -t cpp-dev .
docker run \
    -v $(pwd)/..:/workspace \
    cpp-dev \
    bash \
    -c "mkdir -p /workspace/story-editor/build-win32 && \
    cd /workspace/story-editor/build-win32 && \
    git config --global http.sslverify false && \
    cmake -DOPENSSL_ROOT_DIR=/libs/openssl \
    -DOPENSSL_CRYPTO_LIBRARY=/libs/openssl/lib64 \
    -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw-w64-x86_64.cmake .. && \
    make && \
    make package"