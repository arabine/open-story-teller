mkdir -p /workspace/story-editor/build-linux
cd /workspace/story-editor/build-linux
git config --global http.sslverify false
cmake -DCMAKE_BUILD_TYPE=Release ..
make
cpack
