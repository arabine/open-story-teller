# Story Editor

## How to generate a Windows executable and setup executable on Ubuntu

All the commands listed here are invoked from this `story-editor` root directory.
Make sure to have Docker installed:

```
sudo apt install docker.io
```

Build the Docker image that contains all the necessary development tools:

```
docker build -t cpp-dev .
```

Run it:

```
docker run -it -v $(pwd)/..:/workspace cpp-dev
```


Make sure to use the POSIX version of MinGW:

```
update-alternatives --config x86_64-w64-mingw32-g++


  Selection    Path                                   Priority   Status
------------------------------------------------------------
  0            /usr/bin/x86_64-w64-mingw32-g++-win32   60        auto mode
* 1            /usr/bin/x86_64-w64-mingw32-g++-posix   30        manual mode
  2            /usr/bin/x86_64-w64-mingw32-g++-win32   60        manual mode


update-alternatives --config x86_64-w64-mingw32-gcc

  Selection    Path                                   Priority   Status
------------------------------------------------------------
  0            /usr/bin/x86_64-w64-mingw32-gcc-win32   60        auto mode
* 1            /usr/bin/x86_64-w64-mingw32-gcc-posix   30        manual mode
  2            /usr/bin/x86_64-w64-mingw32-gcc-win32   60        manual mode

```


Cross build, first generate the Makefile:

```
git config --global http.sslverify false # avoid error during clone
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw-w64-x86_64.cmake ..
```

Then build the executable and then the installer:


```
make -j4
make package

```


