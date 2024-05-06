#pragma once


#if defined(_MSC_VER)
    //  Microsoft 
    #define DLIB_EXPORT __declspec(dllexport)

#elif defined(__GNUC__)
    //  GCC
    #define DLIB_EXPORT __attribute__((visibility("default")))

#else
    //  do nothing and hope for the best?
    #define DLIB_EXPORT
    #pragma warning Unknown dynamic link import/export semantics.
#endif


