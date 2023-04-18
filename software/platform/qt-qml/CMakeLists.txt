cmake_minimum_required(VERSION 3.5)

project(ost-app LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set( CMAKE_VERBOSE_MAKEFILE ON)


# Add -O0 to remove optimizations when using gcc
IF(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(Qt5 COMPONENTS Core Quick Widgets REQUIRED)
#find_package(Qt5Multimedia REQUIRED)
#find_package(Qt5Widgets REQUIRED)

if (UNIX AND NOT APPLE)
    add_definitions(-DUSE_UNIX_OS -DUSE_LINUX_OS)
endif()

if(WIN32)
    add_definitions(-DUSE_WINDOWS_OS)
endif()

set(SRCS
    main.cpp
    packarchive.cpp
    StoryTellerModel.cpp
    ni_parser.h
    ni_parser.c
    qml.qrc
)

if(WIN32)
    list(APPEND
        SRCS
        icon.rc
    )
endif()

if(ANDROID)
    add_library(ost-app SHARED
      main.cpp
      packarchive.cpp
      StoryTellerModel.cpp
      ni_parser.c
      qml.qrc
    )
else()
    add_executable(ost-app
      ${SRCS}
    )
endif()

if(WIN32)
SET (LIBS
    ws2_32.lib psapi.lib setupapi.lib cfgmgr32.lib advapi32.lib Dbghelp.lib
)
else()
SET (LIBS

)
endif()

target_compile_definitions(ost-app
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(ost-app
  PRIVATE Qt5::Core Qt5::Quick Qt5::Multimedia Qt5::Widgets icl ${LIBS})

#add_link_options(-Wl,-lstdc++ -Wl,-lpthread -Wl,-ldl -Wl,-ludev)
#SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wl,-lstdc++ -Wl,-lpthread -Wl,-ldl -Wl,-ludev")
