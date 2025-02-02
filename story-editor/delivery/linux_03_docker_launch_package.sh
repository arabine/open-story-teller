
docker run --privileged -v $(pwd)/../..:/workspace cpp-dev-linux /bin/bash -c "cd /workspace/story-editor && ./build_appimage.sh"

